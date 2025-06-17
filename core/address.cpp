//
// Created by Tofu on 25-6-16.
//

#include "fpsSetter.h"
#include "errreport.h"

bool FpsSetter::getAddress()
{
    moduleBase = GetModuleBaseAddress(processID, targetModuleName);
    if (!moduleBase) {
        std::cerr << "无法找到模块基址: " << targetModuleName << '\n';
        ErrorReporter::instance()->receive("只是一昧出错"," (˘•ω•˘)");
        bad = true;
        return false;
    }
    std::cout << "dll基址: " << std::hex << moduleBase <<'\n';

    if (!ReadProcessMemory(processHandle, (LPCVOID)(moduleBase + DYRCX_P_OFFSET), &dyrcx, sizeof(dyrcx),
                           nullptr))
    {
        std::cerr<<"无法获取数组指针\n";
        ErrorReporter::instance()->receive("只是一昧出错"," (˘•ω•˘)");
        bad = true;
        return false;
    }

    // dyrcx = moduleBase + 0x5B85D58;AI你怎么来的
    if (!ReadProcessMemory(processHandle, (LPCVOID)(dyrcx + DYRCX_O_OFFSET), &dyrcx, sizeof(dyrcx),
                           nullptr))
    {
        std::cerr<<"无法获取数组指针\n";
        ErrorReporter::instance()->receive("只是一昧出错"," (˘•ω•˘)");
        bad = true;
        return false;
    }
    std::cout << "动态内存地址: " << std::hex << dyrcx << '\n';

    if (!ReadProcessMemory(processHandle, (LPCVOID)(dyrcx+PFR_OFFSET), &preframerateaddr, sizeof(preframerateaddr), nullptr))
    {
        ErrorReporter::instance()->receive("警告","无法获取帧率所在内存段的指针");
        fpsbad = true;
    }
    else
        std::cout << "帧率地址: " << std::hex << preframerateaddr+FR_OFFSET << '\n';
}