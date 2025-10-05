#pragma once

#include "ylt/reflection/member_names.hpp"

#include <fstream>
#include <algorithm>

/** 存储布局 */
#pragma pack(push, 1)
    struct hipp
    {
        int fps;
        bool checked;
    };
#pragma pack(pop)

template<typename Layout>
class Storage
{
public:
    //确保存储可用
    operator bool() const;
    //检查是否存有记录
    bool exist() const;
    //存
    template<auto Member, typename T>
    bool save(const T& value);
    //取
    template<auto Member>
    auto load() const;
    //清空记录
    void clear();

    Storage(std::string name = filenameify(std::string(ylt::reflection::get_struct_name<Layout>())))
        :storage_name(std::move(name))
    {}
    ~Storage();

private:
    static std::string filenameify(std::string str)
    {
        auto ret = str;
        std::replace(ret.begin(), ret.end(), ':', '-');
        return ret;
    }

    inline static const auto keys = ylt::reflection::get_member_names<Layout>();
    const std::string storage_name;

#ifndef GUI_BUILD_SINGLE
mutable
    std::fstream _file;
#endif
};

#ifdef GUI_BUILD_SINGLE

#include <qt6keychain/keychain.h>
#include <QEventLoop>
/** keychain的同步封装 */
template <typename JobType>
    struct SyncJob : public JobType
{
    SyncJob(const QString &service, QObject *parent = nullptr)
    : JobType(service, parent){}
    /*能写出这样的代码我真是炉火纯青hi啊hi啊hiahia @25.10.3*/
    void start(const std::function<void(QKeychain::Job*)>& afterfinish)
    {
        QEventLoop loop;
        //不会重复注册，qt信号槽自带去重
        QObject::connect(this, &QKeychain::Job::finished, [&loop, &afterfinish, this]()
            {
                if (afterfinish)
                    afterfinish(this);
                loop.quit();
            });
        JobType::start();
        loop.exec();
    }
};

template<class Layout>
    Storage<Layout>::operator bool()const{return true;}

template<class Layout>
    bool Storage<Layout>::exist()const
    {
        bool keyExists = true;
        for (auto key : keys) // 遍历 keys 检查存在性
        {
            SyncJob<QKeychain::ReadPasswordJob> readJobSync(QString::fromUtf8(storage_name));

            readJobSync.setKey(QString::fromUtf8(key));

            readJobSync.start([&keyExists](QKeychain::Job* job)
            {
                if (job->error() != QKeychain::NoError) {
                    keyExists = false;
                }
            });

            if (! keyExists)
                return false;
        }

        return keyExists;
    }

template<class Layout>
    void Storage<Layout>::clear()
    {
        for (auto key : keys)
        {
            auto eraseJobSync = new SyncJob<QKeychain::DeletePasswordJob>(QString::fromUtf8(storage_name));
            eraseJobSync->setKey(QString::fromUtf8(key));
            eraseJobSync->start({});
        }
    }

template<class Layout>
    template<auto Member>
    auto Storage<Layout>::load() const
    {
        SyncJob<QKeychain::ReadPasswordJob> readJobSync (QString::fromUtf8(storage_name));

        using Traits = ylt::reflection::internal::member_tratis<decltype(Member)>;
        using FieldT = Traits::value_type;

        auto key = QString::fromUtf8(keys[ylt::reflection::index_of<Member>()]);
        readJobSync.setKey(key);

        FieldT value = {};
        readJobSync.start([&value](QKeychain::Job* job)
        {
            if (job->error() == QKeychain::NoError)
            {
                QByteArray dat = ((QKeychain::ReadPasswordJob*)job)->binaryData();
                memcpy(&value, dat.data(), sizeof(FieldT));
            }
        });

        return value;  // 返回读取的值
    }

template<class Layout>
    template <auto Member, typename T>
    bool Storage<Layout>::save(const T& value)
    {
        SyncJob<QKeychain::WritePasswordJob> writeJobSync (QString::fromUtf8(storage_name));

        using Traits = ylt::reflection::internal::member_tratis<decltype(Member)>;
        static_assert(std::is_same_v<std::decay_t<T>, typename Traits::value_type>,
                      "value type must match field type");

        auto key = QString::fromUtf8(keys[ylt::reflection::index_of<Member>()]);
        writeJobSync.setKey(key);

        QByteArray data(reinterpret_cast<const char*>(&value), sizeof(T));
        writeJobSync.setBinaryData(data);

        bool status;
        writeJobSync.start([&status](QKeychain::Job* job)
        {
            status = job->error() == QKeychain::NoError;
        });

        return status;
    }

template <typename Layout>
Storage<Layout>::~Storage() = default;

#else
#include <filesystem>

template<class Layout>
    bool Storage<Layout>::exist() const
    {
        return std::filesystem::exists(std::filesystem::path(".")/storage_name);
    }

template<class Layout>
    Storage<Layout>::operator bool() const
    {
        if (!exist())
        {
            _file.open(storage_name, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
        }
        else if (!_file.is_open())
        {
            _file.open(storage_name, std::ios::binary | std::ios::out | std::ios::in);
        }
        return _file.is_open() && _file.good();
    }

template<class Layout>
    template <auto Member>
    // ylt::reflection::internal::member_tratis<decltype(Member)>::value_type
    auto Storage<Layout>::load() const {
        using Traits = ylt::reflection::internal::member_tratis<decltype(Member)>;
        using Owner =  Traits::owner_type;
        using FieldT = Traits::value_type;

        constexpr size_t idx = ylt::reflection::index_of<Member>();
        size_t off = ylt::reflection::member_offsets<Owner>[idx];

        FieldT value{};
        _file.seekg(off);

        _file.read(reinterpret_cast<char*>(&value), sizeof(FieldT));
        return value;
    }

template<class Layout>
    template <auto Member, typename T>
    bool Storage<Layout>::save(const T& value) {
        using Traits = ylt::reflection::internal::member_tratis<decltype(Member)>;
        using Owner = Traits::owner_type;
        using FieldT = Traits::value_type;

        static_assert(std::is_same_v<std::decay_t<T>, FieldT>,
                      "value type must match field type");

        // 通过成员指针求索引
        constexpr size_t idx = ylt::reflection::index_of<Member>();
        size_t off = ylt::reflection::member_offsets<Owner>[idx];

        _file.seekp(off);
        auto size = sizeof(FieldT);
        _file.write(reinterpret_cast<const char*>(&value), sizeof(FieldT));
        return _file.good();
    }

template<class Layout>
    void Storage<Layout>::clear()
    {
        _file.close();
        std::filesystem::remove("./_file");
    }

template<class Layout>
    Storage<Layout>::~Storage()
    {
        _file.close();
    }
#endif