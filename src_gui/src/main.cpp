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

//todo: 处理日志
void prtEnvInfo()
{
    qInfo()<<"====环境信息========================";
    qInfo()<< PrintProcessGroups();
}

//todo: 区分必要的弹窗和仅记录的日志
void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
#ifndef USELOG
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(msg)
    return;
#endif
    QDir logsDir("logs");
    if (!logsDir.exists()) {
        logsDir.mkpath(".");
    }

    QString logFileName = QString("logs/%1.log").arg(
            QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    static QFile logFile(logFileName);
    if (!logFile.isOpen()) { 
        logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

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
//        << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " "
        << msg
//        << " (File:" << context.file
//        << ", Line:" << context.line
        << ", Function:" << context.function << ")"
        << '\n';

    // Fatal 状态下退出程序
    if (type == QtFatalMsg) {

    }
}


FpsSetter initialSetter()
{
    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
        qWarning()<<"未找到游戏窗口";
        ErrorReporter::instance()->receive({ErrorReporter::严重,"未找到第五人格窗口"});
        return {};
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);

    return {pid};
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customLogHandler);
    prtEnvInfo();

    QApplication a(argc, argv);

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
