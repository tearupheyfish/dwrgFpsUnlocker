//
// Created by Tofu on 2025/7/16.
//

#ifndef DWRGFPSUNLOCKER_VERSION_H
#define DWRGFPSUNLOCKER_VERSION_H

#include <QRegularExpression>

//todo:支持不定长版本号比较
struct Version {
    std::vector<uint8_t> verchain;
    std::optional<QString> verstring;

    // 构造函数，解析传入的版本号字符串
explicit
    Version(const QString& vstr) {
        QString numStr = vstr.mid(1); // 去除首字符 'v'
        // 使用正则表达式分割字符串，匹配 '.' 和 '-'
        QStringList parts = numStr.split(QRegularExpression("[.-]"));
        for (const QString& part : parts) {
            bool ok;
            int num = part.toInt(&ok); // 转换为整数
            if (ok) {
                verchain.push_back(static_cast<uint8_t>(num));
            }
        }
    }

    // 比较运算符 '<'，支持长度不等的版本号
    bool operator<(const Version& right) const {
        return verchain < right.verchain;

    }

    // 比较运算符 '>'，支持长度不等的版本号
    bool operator>(const Version& right) const {
        return verchain > right.verchain;

    }

    // 版本号转换为字符串格式
    operator QString() {
        if(!verstring.has_value()) {
            QStringList parts;
            for (uint8_t num: verchain) {
                parts.append(QString::number(num));
            }
            verstring = "v" + parts.join('.');
        }
        return verstring.value();
    }

[[nodiscard]]
    QString toSimple() const
    {
        return "v" + QString::number(verchain[0]) +'.'+ QString::number(verchain[1]);
    }
};

inline Version curver(VERSION_STRING);

#endif //DWRGFPSUNLOCKER_VERSION_H
