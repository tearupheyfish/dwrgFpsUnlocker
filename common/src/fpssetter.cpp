#include "fpssetter.h"

#include "autoxtimerproxy.h"
#include "errreport.h"

#include <QTimer>
#include <QDebug>

#include <chrono>

#include "winapiutil.h"

FpsSetter::FpsSetter(DWORD pid):
    bad(false),processID(pid),processHandle(NULL),keepaccess(false)
   ,autoxprocesstimer(new autoxtimerproxy(this))
{
    if (!pid)
        return;
    qInfo()<<"====初始化FpsSeter========================";

    if(!openHandle())
        return;

    if (!getAddress())
        return;

    autoRelease();//默认自动close

/*  if (!*allocrice)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        *allocrice = si.dwAllocationGranularity;
    }
    std::cout<<"系统页面粒度"<<*allocrice<<'\n';*/
}

FpsSetter FpsSetter::create(DWORD pid)
{
    qDebug()<<"创建FpsSetter类，用的pid"<<pid<<"，当前线程"<<Qt::hex<<GetCurrentThreadId();
    if (!pid)
    {
        // HWND hwd;
        auto hwd = queryTopCognWindow(L"第五人格");
        if (!hwd) hwd = queryTopCognWindow(L"IdentityV");

        if (!hwd)
        {
            qCritical()<<"未找到游戏窗口";
            ErrorReporter::receive(ErrorReporter::警告,"未找到第五人格窗口");
            return {};
        }

        GetWindowThreadProcessId(hwd, &pid);
    }

    return {pid};
}

//不可拷贝的部分：processHandle、autoxtimer（除非可以reset指针）
FpsSetter::FpsSetter(FpsSetter &&right) noexcept
:autoxprocesstimer{new autoxtimerproxy(this)}
{
    // 这些都留着，是给析构函数做的
    // delete right.autoxprocesstimer;right.autoxprocesstimer = nullptr;
    // right.closeHandle();
    processHandle = right.processHandle;right.processHandle = NULL;

    std::tie(bad, keepaccess, processID, moduleBase, funcaddr, dyrcx, preframerateaddr)
            = std::make_tuple(
            std::move(right.bad),std::move(right.keepaccess),
            std::move(right.processID), std::move(right.moduleBase),
            std::move(right.funcaddr), std::move(right.dyrcx), std::move(right.preframerateaddr)
    );
}

FpsSetter &FpsSetter::operator=(FpsSetter &&right) noexcept {
    if(this != &right)
    {
        delete autoxprocesstimer;
        closeHandle();

        // delete right.autoxprocesstimer;right.autoxprocesstimer = nullptr;
        //接管句柄，防止被右侧销毁掉
        processHandle = right.processHandle;right.processHandle = NULL;

        autoxprocesstimer = new autoxtimerproxy(this);
        std::tie(bad, keepaccess, processID, moduleBase, funcaddr, dyrcx, preframerateaddr)
                = std::make_tuple(
                std::move(right.bad), std::move(right.keepaccess),
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
    if (!openHandle()) //如果不持有则尝试申请. 判断bad是上层的责任
        return false;
    // checkGameLiving();

    //写入失败则标记bad
    double frametime = 1.0/fps;
#ifdef USE_LOG
    qInfo()<<"写入帧数到"<<(dyrcx+FRT_OFFSET)<<"..";
#endif
    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+FRT_OFFSET), &frametime, sizeof(frametime), nullptr)) {
        ErrorReporter::receive(ErrorReporter::警告,"无法设置帧率");
        qCritical()<<"没能写入帧时长)";
        bebad();
        return false;
    }

    float framerate = fps;
    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+0x80), &framerate, sizeof(framerate), nullptr)) {
        // ErrorReporter::instance()->receive(ErrorReporter::警告,"不完整的帧率设置");
        qWarning()<<"没能写入帧数(float)";
        bebad();
        return false;
    }

    if (!WriteProcessMemory(processHandle, (LPVOID)(dyrcx+0x8C), &fps, sizeof(fps), nullptr)) {
        // ErrorReporter::instance()->receive(ErrorReporter::严重,"不完整的帧率设置");
        qWarning()<<"没能写入帧数(int)";
        bebad();
        return false;
    }

    return true;
}

float FpsSetter::getFps()
{
    if (!openHandle())
        return 0;

    qInfo()<<"从"<<Qt::hex<<(preframerateaddr+FR_OFFSET)<<Qt::dec<<"读出实时帧率..";

    float framerate{0};
    if (!ReadProcessMemory(processHandle, (LPVOID)(preframerateaddr+FR_OFFSET), &framerate, sizeof(framerate), nullptr))
    {
        qWarning()<<"读取实时帧率失败："<<GetLastError();
        ErrorReporter::receive(ErrorReporter::警告, "无法读取帧率值：");
        bebad();
        return 0;
    }

    return framerate;
}

void FpsSetter::keepAccessible()
{
    keepaccess = true;
    openHandle();//保持句柄持有
    autoxprocesstimer->stop();
}

void FpsSetter::autoRelease()
{
    if (bad)
        return;
    keepaccess = false;
    autoxprocesstimer->start();
}


bool FpsSetter::checkGameLiving()
{
    if (DWORD exitCode; GetExitCodeProcess(processHandle, &exitCode), exitCode != STILL_ACTIVE)
    {
        qCritical()<<"游戏似乎已退出（"<<exitCode<<"）";
        ErrorReporter::receive(ErrorReporter::严重,"游戏似乎已经退出");
        bebad();
        return false;
    }
    return true;
}

bool FpsSetter::openHandle()
{
    if(processHandle)
        return true;

    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!processHandle)
    {
        qCritical()<<"无法访问进程"<<processID;
        ErrorReporter::receive(ErrorReporter::严重,"无法访问进程");
        bebad();
        return false;
    }
    return true;
}

inline void FpsSetter::closeHandle() {
    if(processHandle){
        CloseHandle(processHandle);
        processHandle = NULL;
    }
}

inline void FpsSetter::bebad()
{
    closeHandle();
    bad = true;
    keepaccess = false;
    autoxprocesstimer->stop();
}


