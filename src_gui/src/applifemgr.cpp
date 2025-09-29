//
// Created by Tofu on 2025/6/26.
//

#include "applifemgr.h"
#include "updtchecker.h"
#include "fpsdialog.h"
//#include "updateinformer.h"
#include "fpssetter.h"

AppLifeManager::AppLifeManager(QApplication &a, UpdateChecker &udc, FpsDialog &wm, UpdateDialog &winf, FpsSetter& setter) :
        app(a), udck(udc), wMain(wm), wInfm(winf), setter(setter)
        ,mainclosed(false), informerclosed(false)
{
//    app.setQuitOnLastWindowClosed(true);

    connect(&udck, &UpdateChecker::noUpdateAvailable, this, &AppLifeManager::informerquitonly);


    connect(&wMain, &FpsDialog::MainWinClose, this, &AppLifeManager::appquit);
    connect(&wMain, &FpsDialog::ErrOccured, this, &AppLifeManager::mainquitonly);

    connect(&wInfm, &UpdateDialog::InformerClose, this, &AppLifeManager::informerquitonly);
}

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
