#include "fpssetter.h"

#include "autoxtimerproxy.h"
#include "errreport.h"

#include <QTimer>
#include <QDebug>

#include <chrono>

FpsSetter::FpsSetter(DWORD pid):
    bad(false),fpsbad(false)
   ,processID(pid),processHandle(nullptr)
//   ,dyrcx(NULL), funcaddr(NULL), preframerateaddr(NULL)
{
    qInfo()<<"====初始化FpsSeter========================";

    autoxprocesstimer = new autoxtimerproxy(*this);
    if(!openHandle())
        return;

    getAddress();

    continueAutoX();//默认自动close

/*  if (!*allocrice)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        *allocrice = si.dwAllocationGranularity;
    }
    std::cout<<"系统页面粒度"<<*allocrice<<'\n';*/
}

//不可拷贝的部分：processHandle（）、autoxtimer（除非可以reset指针）
FpsSetter::FpsSetter(FpsSetter &&right) noexcept
:processHandle(right.processHandle)
{
    delete right.autoxprocesstimer;
    right.autoxprocesstimer = nullptr;
    autoxprocesstimer = new autoxtimerproxy(*this);

    right.processHandle = nullptr;

    std::tie(bad, fpsbad, processID, moduleBase, funcaddr, dyrcx)
            = std::make_tuple(
            std::move(right.bad), std::move(right.fpsbad),
            std::move(right.processID), std::move(right.moduleBase),
            std::move(right.funcaddr), std::move(dyrcx)
    );
}

FpsSetter &FpsSetter::operator=(FpsSetter &&right) noexcept {
    if(this != &right)
    {
        delete autoxprocesstimer;
        delete right.autoxprocesstimer;
        right.autoxprocesstimer = nullptr;
        autoxprocesstimer = new autoxtimerproxy(*this);

        closeHandle();
        processHandle = right.processHandle;
        right.processHandle = nullptr;

        std::tie(bad, fpsbad, processID, moduleBase, funcaddr, dyrcx, preframerateaddr)
                = std::make_tuple(
                std::move(right.bad), std::move(right.fpsbad),
                std::move(right.processID), std::move(right.moduleBase),
                std::move(right.funcaddr), std::move(right.dyrcx), std::move(right.preframerateaddr)
                /* 2025.6.26： dyrcx和pfraddr没有move右侧的(right.~)，因此额外花费2h，记一笔*/
        );
    }
    return *this;
}

FpsSetter::~FpsSetter()
{
    closeHandle();
    delete autoxprocesstimer;
}

bool FpsSetter::setFps(int fps)
{
    if (!(openHandle() && checkGameLiving()))
        return false;

    double frametime = 1.0/fps;
#ifdef USELOG
    qInfo()<<"写入帧数到"<<(dyrcx+FRT_OFFSET)<<"..";
#endif
    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+FRT_OFFSET), &frametime, sizeof(frametime), nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法设置帧率");
        //todo QFatal
        bad = true;
        return false;
    }

    float framerate = fps;
    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+0x80), &framerate, sizeof(framerate), nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法设置帧率");
        //todo QFatal
        bad = true;
        return false;
    }

    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+0x8C), &fps, sizeof(fps), nullptr)) {
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法设置帧率");
        //todo QFatal
        bad = true;
        return false;
    }

    return true;
}

float FpsSetter::getFps()
{
    //无法获取实时帧率不是致命的
    if (fpsbad)
        return 0;

    if (!(
                openHandle()||checkGameLiving()
        )) return 0;

    qInfo()<<"从"<<std::ios::hex<<(preframerateaddr+FR_OFFSET)<<std::ios::dec<<"读出实时帧率..";

    float framerate;
    if (!ReadProcessMemory(processHandle, (LPVOID)(preframerateaddr+FR_OFFSET), &framerate, sizeof(framerate), nullptr))
    {
        qWarning()<<"读取实时帧率失败："<<GetLastError();
        ErrorReporter::instance()->receive(
                ErrorReporter::严重,
                "无法读取帧率值：");
        fpsbad = true;
        return 0;
    }

    return framerate;
}

bool FpsSetter::checkGameLiving()
{
    if (DWORD exitCode; GetExitCodeProcess(processHandle, &exitCode), exitCode != STILL_ACTIVE)
    {
        qCritical()<<"游戏似乎已退出（"<<exitCode<<"）";
        ErrorReporter::instance()->receive(ErrorReporter::严重,"游戏似乎已经退出");
        bad = true;
        return false;
    }
    return true;
}

bool FpsSetter::openHandle()
{
    // question: processHandle的检查是交由调用者还是自己，亦或是防御性
    if(processHandle)
        return true;

    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle)
    {
        qCritical()<<"无法访问进程"<<processID;
        ErrorReporter::instance()->receive(ErrorReporter::严重,"无法访问进程");
        bad = true;
        return false;
    }
    return true;
}

void FpsSetter::closeHandle() {
    if(processHandle){
        CloseHandle(processHandle);
        processHandle = nullptr;
    }
}


void FpsSetter::pauseAutoX()
{
    autoxprocesstimer->timer.stop();
}

void FpsSetter::continueAutoX()
{
    autoxprocesstimer->timer.start();
}



