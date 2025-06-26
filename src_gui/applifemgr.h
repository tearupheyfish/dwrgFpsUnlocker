//
// Created by Tofu on 2025/6/26.
//

#ifndef DWRGFPSUNLOCKER_APPLIFEMGR_H
#define DWRGFPSUNLOCKER_APPLIFEMGR_H

#include <QObject>
#include <QApplication>

class UpdateChecker;
class Dialog;
class UpdateInformer;
class FpsSetter;

//todo 滥用信号槽？

class AppLifeManager:public QObject{
    Q_OBJECT
    QApplication& app;
    UpdateChecker& udck;
    FpsSetter& setter;
    Dialog& wMain;
    UpdateInformer& wInfm;

    bool mainclosed, informerclosed;
public:
    AppLifeManager(QApplication &a, UpdateChecker& udc, Dialog& wm, UpdateInformer& winf, FpsSetter& setter);
private slots:
    void mainquitonly();
    void informerquitonly();
    void appquit();
private:
    void checkShouldQuit()
    {
        if(mainclosed && informerclosed)
        {
            appquit();
        }
    }
};

#endif //DWRGFPSUNLOCKER_APPLIFEMGR_H
