//
// Created by Tofu on 2025/6/26.
//

#include "applifemgr.h"
#include "updtchecker.h"
#include "fpsdialog.h"
//#include "updateinformer.h"
#include "fpssetter.h"

AppLifeManager::AppLifeManager(QApplication &a, UpdateChecker& udc, FpsDialog& wm, UpdateDialog& winf) :
app(a), udck(udc), wMain(wm), wInfm(winf),
mainclosed(false), informerclosed(false)
{
    connect(&udck, &UpdateChecker::noUpdateAvailable, this, &AppLifeManager::informerquitonly);


    connect(&wMain, &FpsDialog::MainWinClose, this, &AppLifeManager::appquit);
    connect(&wMain, &FpsDialog::ErrOccured, this, &AppLifeManager::mainquitonly);

    connect(&wInfm, &UpdateDialog::InformerClose, this, &AppLifeManager::informerquitonly);
}

//对一个尚未show的窗口close不会触发closeEvent。。。

void AppLifeManager::mainquitonly() {
    wMain.close();
    mainclosed = true;
    checkShouldQuit();
}

void AppLifeManager::informerquitonly() {
    wInfm.close();
    informerclosed = true;
    checkShouldQuit();
}

void AppLifeManager::appquit() {
    app.quit();
}
