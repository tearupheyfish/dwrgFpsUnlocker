//
// Created by Tofu on 2025/6/26.
//

#include "applifemgr.h"
#include "update_checker.h"
#include "fpsdialog.h"
//#include "updateinformer.h"
#include "fpssetter.h"

AppLifeManager::AppLifeManager(QApplication &a, UpdateChecker &udc, Dialog &wm, UpdateInformer &winf, FpsSetter& setter) :
        app(a), udck(udc), wMain(wm), wInfm(winf), setter(setter)
        ,mainclosed(false), informerclosed(false)
{
//    app.setQuitOnLastWindowClosed(true);

    connect(&udck, &UpdateChecker::noUpdateAvailable, this, &AppLifeManager::informerquitonly);


    connect(&wMain, &Dialog::MainWinClose, this, &AppLifeManager::appquit);
    connect(&wMain, &Dialog::ErrOccured, this, &AppLifeManager::mainquitonly);

    connect(&wInfm, &UpdateInformer::InformerClose, this, &AppLifeManager::informerquitonly);
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
