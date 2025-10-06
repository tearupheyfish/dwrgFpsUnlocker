//
// Created by Tofu on 2025/7/16.
//

#ifndef DWRGFPSUNLOCKER_VERSION_H
#define DWRGFPSUNLOCKER_VERSION_H

#include <QRegularExpression>
#include <QString>


struct Version {
    std::vector<uint8_t> verchain;
    std::optional<QString> verstring;

    // 构造函数，解析传入的版本号字符串
    Version(const QString& vstr = "0.0") {
        //左侧是 [开头(^)|.|v] 且右侧是 [结尾($)|.|-] 的 整数(\d)+
        QRegularExpression re(R"((?:^|\.|v)(\d+)(?=\.|-|$))");
        auto it = re.globalMatch(vstr);

        while (it.hasNext())
        {
            QRegularExpressionMatch m = it.next();
            auto part = m.captured(1); //每次匹配的第一个捕获组就是(\d+)
            verchain.push_back(static_cast<uint8_t>(part.toUShort()));//可惜没有toUChar
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
explicit
    operator const QString&() {
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
    QString toQString(int lenth = -1)
    {
        return operator const QString&().sliced(0, 1+2*((lenth<=0?verchain.size():lenth)-1)+1);
    }
};

#endif //DWRGFPSUNLOCKER_VERSION_H
