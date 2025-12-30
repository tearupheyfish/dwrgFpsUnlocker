#include "macroes.h"

#include "errreport.h"
#include "log.h"

#include "applifemgr.h"
#include "fpsdialog.h"
#include "updtchecker.h"
#include "winapiutil.h"
#include "gamelistener.h"
#include "tray.h"
#include "duplicate.h"

#include <QApplication>
#include <QSessionManager>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QProcess>

#include <memory>
#include <codecvt>
#include <qhotkey.h>

int main_update(int, char **);
int main_setter(int, char **);

#ifndef BUILD_SINGLE
#define main_update sizeof
#endif
int main(int argc, char *argv[])
{
    switch (argc)
    {
        case 1 : case 2: return main_setter(argc, argv);
        default: return main_update(argc, argv);
    }
}
#undef main_update

int main_setter(int argc, char *argv[])
{
    qInfo()<<"====环境信息========================";
    qInfo()<< "版本: "<<VERSION_STRING;
    qInfo()<< PrintProcessGroups();

    QApplication app(argc, argv);
    QApplication::setApplicationVersion(VERSION_STRING);
    QApplication::setQuitOnLastWindowClosed(false);//手动管理

    qInstallMessageHandler(customLogHandler);
    // app.installEventFilter(new EventSpy(&app));

    AppLifeManager lifemgr(app);
    auto defaultlaunch = [&lifemgr]()
    {
        lifemgr.delever<FpsDialog>();
    };
    auto silentlaunch = [&lifemgr]()
    {
        ErrorReporter::silent();
        auto w_ = new FpsDialog(NULL);
        if (*w_)
        {
            w_->show();w_->raise();w_->activateWindow();
            lifemgr.trustee(w_);
        }
        else
            delete w_;
        ErrorReporter::verbose();
    };
    auto followlaunch = [&lifemgr](DWORD pid)
    {
        qInfo()<<"检测到游戏启动，pid:"<<pid;
        QTimer::singleShot(3000, [&lifemgr,pid]()
        {
            lifemgr.delever<FpsDialog, DWORD>(pid);
        });
    };

    //首次启动的窗口不报错
    Copy copy("dwrgFpsUncloker");
    QObject::connect(&copy, &Copy::NewLaunch, defaultlaunch);
    QObject::connect(&copy, &Copy::SilentLaunch,  silentlaunch);
    //↑这两个都会无响应，如果加了silent后verbose的话。
    if (copy )
    {
        QMetaObject::invokeMethod(&app, "quit", Qt::QueuedConnection);
        return app.exec();
    }

    auto ifm{std::make_unique<UpdateDialog>()};
    auto uc(std::make_unique<UpdateChecker>(*ifm));
    ifm->setRelativeData(uc.get());

    lifemgr.trustee(std::move(uc));
    lifemgr.trustee(std::move(ifm));
    /// 此后的ifm 和 uc 都默认失效了

    /// 监听跟随启动
    ProgStartListener listener;
    // //lifemgr生命周期结束后于事件循环，所以引用安全
    QObject::connect(&listener, &ProgStartListener::processStarted, followlaunch);
    listener.start(QStringLiteral("dwrg.exe"), 3);

    /// 托盘图标
    AppTray tray;
    tray.show();
    //应该是direct类型
    QObject::connect(&tray, &QSystemTrayIcon::activated, [&](const QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::DoubleClick)
        {
            defaultlaunch();
        }
    });

    /// 热键
    QHotkey resetfps(QKeySequence(Qt::Key_F5), true, &app);
    QObject::connect(&resetfps, &QHotkey::activated, [&lifemgr]()
    {
        auto [fw, fwpid] = getFocusedWindowInfo();
        if (fwpid > 0)
        lifemgr.foreachwin(
            [fwpid](FpsDialog& dia){if (gidequal(dia, fwpid))dia.switch_default();});
    });

    //不应该直接发dosave，因为不知道uselast
    auto checksave = []()
    {
        using Hipp = Storage<hipp, "hipp">;
        //question: load基于一种假设就是exist&available
        //          可是dosave不要求exist
        //          直接dosave的话我们并不知道是否需要save
        if (Hipp::available() && Hipp::load<&hipp::checked>())//question: 存储available也不等于缓存有数据？
        {
            Hipp::dosave();
        }
    };

    HiddenWin hidden_win;
    QObject::connect(&app, &QApplication::commitDataRequest, checksave);
    QObject::connect(&app, &QApplication::aboutToQuit, checksave);

    //预添加更新
    QMetaObject::invokeMethod(&lifemgr.get<UpdateChecker>(), "checkUpdate", Qt::QueuedConnection);

    // setter的初始化报错时lifemgr需要已经注册信号槽，因此setter必须在lifemgr之后初始化
    qInfo()<<"====运行========================";
    qDebug()<<"主线程id:"<<QThread::currentThreadId();
    return app.exec();
}

[[maybe_unused]]
int main_update(int argc, char *argv[])
{
    std::filesystem::path exedownloaded = argv[1];
    std::filesystem::path dirtoextra = argv[2];
    if (argc > 3)
    {
        int64_t waitpid = std::stoull(argv[3]);
        HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, waitpid);
        if (!hProcess)
        {
            std::cerr<<"出错：未能访问指定进程";
            return 1;
        }
        //Windows中对象本身就是可等待的
        WaitForSingleObject(hProcess, INFINITE);

        CloseHandle(hProcess);
    }

    auto to = dirtoextra/exedownloaded.filename();
    try
    {
        std::filesystem::copy(exedownloaded, to, std::filesystem::copy_options::overwrite_existing);
    }catch (std::exception&e)
    {
        return -1;
    }

    auto filetoexecute_q = QString::fromStdString(to.string());
    QProcess::startDetached(filetoexecute_q);

    return 0;
}
