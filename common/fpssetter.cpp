#include "fpssetter.h"

#include "autoxTimerProxy.h"
#include "errreport.h"

#include <QTimer>
#include <QObject>
#include <chrono>

FpsSetter::FpsSetter(DWORD pid):
    bad(false),fpsbad(false)
   ,processID(pid),processHandle(nullptr)
//   ,dyrcx(NULL), funcaddr(NULL), preframerateaddr(NULL)
{
    autoxprocesstimer = new autoxTimerProxy(*this);
    openHandle();

//    auto t = std::chrono::high_resolution_clock::now();
    getAddress();
//    auto dulation = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t).count();
//    std::cout<<std::dec<<dulation<<" microseconds\n";

    continueautox();//默认自动close
/*    //句柄实际上就是模块的装入地址
    HMODULE moduleHandle = reinterpret_cast<HMODULE>(moduleBase);
    FARPROC funcAddress = GetProcAddress(moduleHandle, "PyOS_ReadlineFunctionPointer");
    if (funcAddress == NULL) {
        std::cerr << "无法找到符号: " << func_name << std::endl;
        ErrorReporter::instance()->receive(ErrorReporter::严重, "无法找到符号");
    }*/

/*    if (!*allocrice)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        *allocrice = si.dwAllocationGranularity;
    }
    std::cout<<"系统页面粒度"<<*allocrice<<'\n';*/
}

FpsSetter::~FpsSetter()
{
    closeHandle();
    delete autoxprocesstimer;
}


bool FpsSetter::setFps(int fps)
{
    if (!(openHandle() && checkHandle()))
        return false;

    double frametime = 1.0/fps;

    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+FRT_OFFSET), &frametime, sizeof(frametime), nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法设置帧率");
        bad = true;
        return false;
    }

    return true;
}

bool FpsSetter::checkHandle()
{
    if (DWORD exitCode; GetExitCodeProcess(processHandle, &exitCode), exitCode != STILL_ACTIVE)
    {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"进程似乎已经退出");
        bad = true;
        return false;
    }
    return true;
}

float FpsSetter::getFps()
{
    if (fpsbad ||
        !(
        openHandle()||checkHandle()
        )) return 0;

    float framerate;
    if (!ReadProcessMemory(processHandle, (LPVOID)(preframerateaddr+FR_OFFSET), &framerate, sizeof(framerate), nullptr))
    {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法读取帧率值");
        fpsbad = true;
        return 0;
    }

    return framerate;
}

bool FpsSetter::openHandle()
{
    if(processHandle) return true;
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle)
    {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法访问进程");
        bad = true;
        return false;
    }
    return true;
}

void FpsSetter::pauseautox()
{
    autoxprocesstimer->timer.stop();
}

void FpsSetter::continueautox()
{
    autoxprocesstimer->timer.start();
}

