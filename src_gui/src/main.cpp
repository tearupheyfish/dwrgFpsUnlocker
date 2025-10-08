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

int main_update(int, char **);
int main_setter(int, char **);

#ifndef GUI_BUILD_SINGLE
#define main_update sizeof
#endif
int main(int argc, char *argv[])
{
    switch (argc)
    {
        case 1 : return main_setter(argc, argv);
        default: return main_update(argc, argv);
    }
}
#undef main_update

int main_setter(int argc, char *argv[])
{
    qInstallMessageHandler(customLogHandler);

    qInfo()<<"====环境信息========================";
    qInfo()<< "版本: "<<VERSION_STRING;
    qInfo()<< PrintProcessGroups();

    QApplication app(argc, argv);
    QApplication::setApplicationVersion(VERSION_STRING);
    QApplication::setQuitOnLastWindowClosed(false);//手动管理


    std::unique_ptr<FpsDialog> w {FpsDialog::create()};
    w->show();

    UpdateDialog ifm;
    UpdateChecker uc(ifm);
    ifm.setRelativeData(uc);
    ifm.hide();

    AppLifeManager lifemgr(app, uc, *w, ifm);
    // setter的初始化报错时lifemgr需要已经注册信号槽，因此setter必须在lifemgr之后初始化

    uc.checkUpdate();

    qInfo()<<"====运行========================";
    return app.exec();
}

[[maybe_unused]]
int main_update(int argc, char *argv[])
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
