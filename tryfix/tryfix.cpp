//
// Created by Tofu on 25-6-24.
//

#include "winapiutil.h"

#include <windows.h>

#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

/** 接受地址、exe名称
 *  修改内存中与地址一致的静态变量*/
int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hex_address> <process_name>" << std::endl;
        return 1;
    }

    uintptr_t address;
    try {
        address = std::stoull(argv[1]);
    } catch (const std::exception &) {
        std::cerr << "Invalid address format" << std::endl;
        return -1;
    }

    const char *exename = argv[2];

    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
        std::cerr<<("未找到第五人格窗口");
        return -2;
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);
    std::cout << "找到窗口，进程ID: " << pid << std::endl;

    auto pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    std::cout<<"进程句柄："<<std::hex<<pHandle<<std::endl;

    uintptr_t moduleBase = GetModuleBaseAddress(pid, "neox_engine.dll");
    std::cout << "基址: " << std::hex << moduleBase << std::endl;;

    uintptr_t funcaddr = getProcAddressEx(pHandle, moduleBase, "PyOS_ReadlineFunctionPointer");
    std::cout <<"符号地址："<<std::hex<<funcaddr<<std::endl;

    std::cout<<"--------------------------------\n";
    std::cout<<"原符号地址"<<std::hex<<address<<std::endl;

    ModifyMappedData(exename, address, funcaddr);

    return 0;
}