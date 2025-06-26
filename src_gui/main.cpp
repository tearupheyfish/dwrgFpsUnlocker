#include "applifemgr.h"
#include "errreport.h"
#include "fpsdialog.h"
#include "updateinformer.h"
#include "env.h"

#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <memory>

FpsSetter initialSetter()
{
    HWND targetWindow = FindWindowW(nullptr, L"第五人格");
    if (!targetWindow)
    {
        ErrorReporter::instance()->receive({"严重","未找到第五人格窗口"});
        return {};
    }

    DWORD pid;
    GetWindowThreadProcessId(targetWindow, &pid);
    std::cout << "找到窗口，进程ID: " << pid << std::endl;

    return {pid};
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Dialog w;
    w.setWindowTitle("第五帧率解锁");
    w.setFixedSize(w.width(), w.height());

    FpsSetter setter;
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    w2:

    UpdateInformer ifm;
    UpdateChecker uc(ifm);
    ifm.setRelativeData(uc);

    AppLifeManager lifemgr(a, uc, w, ifm, setter);
    /// lifemgr需要setter的实例，因此lifemgr必须在setter之后初始化
    /// setter的初始化报错了无法给予lifemgr的槽函数，因此setter必须在lifemgr之后初始化

    setter = initialSetter();
    if((bool)setter)
    {
        w.setRelativedData(setter);
        w.show();
    }


    uc.checkUpdate();

    return a.exec();
}
