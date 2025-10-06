#include "macroes.h"

#include "applifemgr.h"
#include "errreport.h"
#include "fpsdialog.h"
#include "updtdialog.h"
#include "env.h"
#include "winapiutil.h"

#include <QDir>
#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <memory>

void prtEnvInfo()
{
    qInfo()<<"====环境信息========================";
    qInfo()<< PrintProcessGroups();
    qInfo()<< "版本: "<<VERSION_STRING;
}

//@ref DevDoc/logging.cov.md
void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
#ifndef USE_LOG
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(msg)
    return;
#endif
    static QMutex mutex;
    QMutexLocker lock(&mutex);//自动阻塞-加锁-释放类

    static QFile logFile;
    static bool inited = false;

    if (!inited) {
        QString logDir = "logs";
        QDir().mkpath(logDir);

        QString logName = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss.log");
        logFile.setFileName(QString("%1/%2").arg(logDir, logName));

        logFile.open(QIODevice::Append | QIODevice::Text);
        inited = true;
    }

#ifndef _DEBUG
    if (type == QtDebugMsg)
    {
        return;
    }
#endif

    // 格式化日志信息
    QString logType;
    QTextStream out(&logFile);
    switch (type) {
        case QtDebugMsg:
            logType = "[DEBUG]";
            break;
        case QtInfoMsg:
            logType = "[INFO]";
            break;
        case QtWarningMsg:
            logType = "[WARNING]";
            break;
        case QtCriticalMsg:
            logType = "[CRITICAL]";
            break;
        case QtFatalMsg:
            logType = "[FATAL]";
            break;
    }

    out << logType << ": "
        << msg
#ifdef _DEBUG
        << ", Function:" << context.function << ")"
#endif
        << '\n';
}


FpsSetter initialSetter()
{
    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
        qCritical()<<"未找到游戏窗口";
        ErrorReporter::instance()->receive({ErrorReporter::严重,"未找到第五人格窗口"});
        return {};
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);

    return {pid};
}

int main(int argc, char *argv[])
{
#ifdef GUI_BUILD_SINGLE
    if (argc > 1)
    {
        std::filesystem::path newExePath = argv[1];
        std::filesystem::path curDir = argv[2];
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
        auto to = curDir/newExePath.filename();
        try
        {
            std::filesystem::copy(newExePath, to, std::filesystem::copy_options::overwrite_existing);
        }catch (std::exception&e)
        {
            qCritical()<<e.what();
            return -1;
        }
        return 0;
    }
#endif
    qInstallMessageHandler(customLogHandler);
    prtEnvInfo();

    QApplication a(argc, argv);
    a.setApplicationVersion(VERSION_STRING);

    FpsSetter setter;
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    FpsDialog w; //需要再fpssetter后创建，才不会让timer访问已经销毁的setter
    w.setWindowTitle("第五帧率解锁");
    w.setFixedSize(w.width(), w.height());


    UpdateDialog ifm;
    UpdateChecker uc(ifm);
    ifm.setRelativeData(uc);

    AppLifeManager lifemgr(a, uc, w, ifm, setter);
    /// lifemgr需要setter的实例，因此lifemgr必须在setter之后初始化
    /// setter的初始化报错了无法给予lifemgr的槽函数，因此setter必须在lifemgr之后初始化

    setter = initialSetter();
    if(setter)
    {
        w.setRelativedData(setter);
        w.show();
    }

    uc.checkUpdate();

    qInfo()<<"====运行========================";
    return a.exec();
}
