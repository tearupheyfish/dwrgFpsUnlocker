//
// Created by Tofu on 2025/5/15.
//
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>

#include "fpsSetter.h"

int main()
{
    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
       std::cerr<<"未找到第五人格窗口";
        return -1;
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);
    std::cout << "找到窗口，进程ID: " << pid << std::endl;

    FpsSetter setter(pid);

    int fps = 60;

    if (std::filesystem::exists(std::filesystem::path("./hipp")))
    {
        std::fstream file("./hipp", std::ios::in | std::ios::binary);
        if (!file)
        {
            std::cerr << "打不开文件 hipp";
            exit(-5);
        }
        file.read((char*)&fps, sizeof(fps));
        setter.setFps(fps);
        std::cout<<"自动设置上次的帧率值: "<<std::dec<<fps<<'\n';
        file.close();
    }

    std::cout<<"输入期望帧率的正整数值并回车：";

    if (!(std::cin>>fps))
        exit(-1);

    setter.setFps(fps);

    int time(5);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
    while (time--)
    {
        Sleep(1000);
        std::cout << '\r' << setter.getFps();
    }
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    std::ofstream file("./hipp", std::ios::out | std::ios::binary);
    file.write((char*)&fps, sizeof(fps));
    bool b = true;
    file.write((char*)&b, sizeof(b));

    std::cout<<'\n';
    Sleep(2000);

    return 0;
    //todo: 定时终止
}