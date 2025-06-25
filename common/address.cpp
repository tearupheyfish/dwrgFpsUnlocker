//
// Created by Tofu on 25-6-16.
//

#include <filesystem>
#include "winapiutil.h"
#include "fpssetter.h"
#include "errreport.h"

    bool FpsSetter::getAddress()
    {
        static const char func_name[256] = "PyOS_ReadlineFunctionPointer";

        moduleBase = GetModuleBaseAddress(processID, targetModuleName);
        if (!moduleBase) {
            ErrorReporter::instance()->receive("(oT-T)尸","无法找到模块基址");
            bad = true;
            return false;
        }
#ifdef _DEBUG
        std::cout << "dll基址: " << std::hex << moduleBase <<'\n';
#endif

        funcaddr = getProcAddressExBuffered(processHandle, moduleBase, "PyOS_ReadlineFunctionPointer");
        if(!funcaddr)
        {
            ErrorReporter::instance()->receive(ErrorReporter::严重,"无法查询符号");
            bad = true;
            return false;
        }
#ifdef _DEBUG
        std::cout<< "符号地址：" << std::hex<< funcaddr<< '\n';
#endif

        if (!ReadProcessMemory(processHandle, (LPCVOID)(DYRCX_P_OFFSET), &dyrcx, sizeof(dyrcx),
                               nullptr))
        {
            ErrorReporter::instance()->receive(ErrorReporter::严重,"无法获取数组指针");
            bad = true;
            return false;
        }

        if (!ReadProcessMemory(processHandle, (LPCVOID)(dyrcx + DYRCX_O_OFFSET), &dyrcx, sizeof(dyrcx),
                               nullptr))
        {
            ErrorReporter::instance()->receive(ErrorReporter::严重,"无法获取数组指针");
            bad = true;
            return false;
        }
#ifdef _DEBUG
        std::cout << "动态内存地址: " << std::hex << dyrcx << '\n';
#endif

        {
            float checkfr = 0;
            if (!ReadProcessMemory(processHandle, (LPCVOID) (dyrcx + 0x80), &checkfr, sizeof(checkfr), nullptr))
            {
            }
            else{
                if(checkfr < 15 || checkfr > 75)
                {
                    ErrorReporter::instance()->receive("可疑", ("帧率值"+std::to_string(checkfr)+"，尝试修复").c_str());
                }
            }
        }

        if (!ReadProcessMemory(processHandle, (LPCVOID)(dyrcx+PFR_OFFSET), &preframerateaddr, sizeof(preframerateaddr), nullptr))
        {
            ErrorReporter::instance()->receive(ErrorReporter::警告,"无法获取帧率所在内存段的指针");
            fpsbad = true;
        }
        else
            std::cout << "帧率地址: " << std::hex << preframerateaddr+FR_OFFSET << '\n';

        return true;
    }

