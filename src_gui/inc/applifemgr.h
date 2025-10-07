//
// Created by Tofu on 2025/6/26.
//

#ifndef DWRGFPSUNLOCKER_APPLIFEMGR_H
#define DWRGFPSUNLOCKER_APPLIFEMGR_H

#include <QObject>
#include <QApplication>

class UpdateChecker;
class FpsDialog;
class UpdateDialog;
class FpsSetter;

//todo 滥用信号槽？

class AppLifeManager:public QObject{
    Q_OBJECT
    QApplication& app;
    UpdateChecker& udck;
    FpsDialog& wMain;
    UpdateDialog& wInfm;

    bool mainclosed, informerclosed;
public:
    AppLifeManager(QApplication &a, UpdateChecker& udc, FpsDialog& wm, UpdateDialog& winf);
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
