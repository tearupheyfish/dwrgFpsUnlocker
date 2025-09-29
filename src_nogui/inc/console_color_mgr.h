//
// Created by Tofu on 25-6-19.
//

#ifndef DWRGFPSUNLOCKER_CONSOLE_COLOR_MGR_H
#define DWRGFPSUNLOCKER_CONSOLE_COLOR_MGR_H

#include <windows.h>

class ConsoleStyleManager {
private:
    HANDLE hConsole; // 控制台句柄
    WORD defaultAttributes; // 控制台的默认输出风格

public:
    ConsoleStyleManager() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        // 保存当前控制台的输出风格
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        defaultAttributes = csbi.wAttributes;
    }

    // 设置控制台输出颜色
    void setStyle(WORD attributes) {
        SetConsoleTextAttribute(hConsole, attributes);
    }

    // 恢复控制台默认颜色
    void resetStyle() {
        SetConsoleTextAttribute(hConsole, defaultAttributes);
    }

    // 析构函数，自动恢复控制台默认颜色
    ~ConsoleStyleManager() {
        resetStyle();
    }
};


#endif //DWRGFPSUNLOCKER_CONSOLE_COLOR_MGR_H
