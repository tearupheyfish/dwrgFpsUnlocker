//
// Created by Tofu on 2025/5/15.
//

#include "fpssetter.h"
#include "console_color_mgr.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <thread>



int main()
{
    system("chcp 65001");

    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
       std::cerr<<"未找到第五人格窗口...\n";
       Sleep(1500);
       return -1;
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);
    std::cout << "找到窗口，进程ID: " << pid << std::endl;

    FpsSetter setter(pid);
    ConsoleStyleManager csm;

    auto cycle_print_fps = [&]()
    {
        int time(5);

        csm.setStyle(FOREGROUND_GREEN);
        while (time--)
        {
            std::cout<<setter.getFps();
            Sleep(1000);
            std::cout<<'\r';
        }
        csm.resetStyle();
    };

    int fps = 60;

    if (std::filesystem::exists(std::filesystem::path("./hipp")))
    {
        std::fstream file("./hipp", std::ios::in | std::ios::binary);
        if (!file)
        {
            std::cerr << "打不开文件 hipp..\n";
            Sleep(1500);
            exit(-5);
        }
        file.read((char*)&fps, sizeof(fps));
        setter.setFps(fps);
        std::cout<<"自动设置上次的帧率值: ";
        csm.setStyle(FOREGROUND_GREEN);;
        std::cout<<std::dec<<fps<<'\n';
        csm.resetStyle();
        cycle_print_fps();
        file.close();
    }

    std::atomic_bool receive_input(false), bad_input(false);
//    std::cout<<"输入期望帧率的正整数值并回车：";
    std::thread inputThread([&]() {
        std::cout << "输入期望帧率的正整数值并回车：";
        if (!(std::cin>>fps))
        {
            std::cout<<"没看懂..请检查输入？";
            bad_input = true;
            return;
        }
        receive_input = true;
    });

    int timeout = 5*2; // 超时时间（秒）
    for (int i = 0; i < timeout; ++i)
    {
        if (receive_input || bad_input)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds (500));
    }

    if (!receive_input)
    {
        if(!bad_input)
            std::cout << "\n(－_－)"<<std::flush;
        if (inputThread.joinable())
            inputThread.detach(); // 分离子线程，确保程序退出
        Sleep(1500);
        std::cout<<"，退堂！"<<std::flush;
        Sleep(500);
        return 0;               // 超时退出
    }

    setter.setFps(fps);
    cycle_print_fps();

    std::ofstream file("./hipp", std::ios::out | std::ios::binary);
    file.write((char*)&fps, sizeof(fps));
    bool b = true;
    file.write((char*)&b, sizeof(b));

    std::cout<<'\n';
    Sleep(2000);

    inputThread.join();

    return 0;
}