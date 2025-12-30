//
// Created by Tofu on 2025/6/26.
//

#include "applifemgr.h"
#include "updtchecker.h"
#include "fpsdialog.h"
#include "fpssetter.h"

//偏特化还得放调用前
template<>
void AppLifeManager::trustee(std::unique_ptr<UpdateDialog>&& ifm)
{
    //右值是对调用方的约束，在函数体内是新的左值，所以移动赋值的话还是需要移动语义
    this->inform = std::move(ifm);
    QObject::connect(this->inform.get(), &UpdateDialog::InformerClose, this, &AppLifeManager::closeupdate);
}

template<>
void AppLifeManager::trustee(std::unique_ptr<UpdateChecker>&& chk)
{
    this->udck = std::move(chk);
    connect(udck.get(), &UpdateChecker::noUpdateAvailable, this, &AppLifeManager::closeupdate);
}

template<>
void AppLifeManager::trustee(FpsDialog* dialog)
{
    windows.push_back(dialog);
    qDebug()<<"记录窗口:"<<Qt::hex<<dialog;
    QObject::connect(dialog, &FpsDialog::SetterClosed, this, &AppLifeManager::setterclosed, Qt::QueuedConnection);
    //破案了，窗口的closeEvent函数发送了SetterClose，DirectConnect->setterclosed就把窗口本身删了 1h --25.12.30
    //question: 奇怪的是跟随启动的窗口和主动启动的好窗口不会这样
}

template<>
void AppLifeManager::trustee(std::unique_ptr<FpsDialog>&& dialog)
{
    trustee(dialog.release());
}

//类成员函数模板不支持偏特化 👊👿

template <>
FpsDialog& AppLifeManager::delever<FpsDialog,DWORD>(DWORD pid)
{
    auto w_ = new FpsDialog(pid);
    w_->show();w_->raise();w_->activateWindow();
    trustee(w_);
    return *w_;
}
template<>
FpsDialog& AppLifeManager::delever<FpsDialog>()
{
    return delever<FpsDialog,DWORD>(NULL);
}


AppLifeManager::AppLifeManager(QApplication &a) :app(a){}

AppLifeManager::~AppLifeManager()
{
    for (const auto* w : windows)
        delete w;
}

//对一个尚未show的窗口close不会触发closeEvent。。。

void AppLifeManager::setterclosed(FpsDialog* that) {
    auto it = std::ranges::find(windows, that);
    if (it == windows.end())
    {
        ErrorReporter::receive(ErrorReporter::严重, "销毁一个未记录的窗口");
        qCritical()<<"销毁一个未记录的窗口";
        return;
    }
    qDebug()<<"销毁窗口:"<<Qt::hex<<that;
    windows.erase(it);
    delete that;
}

void AppLifeManager::closeupdate() {
    // inform->close();
    inform.reset();
    udck.reset();
}

void AppLifeManager::appquit() {
    app.quit();
}