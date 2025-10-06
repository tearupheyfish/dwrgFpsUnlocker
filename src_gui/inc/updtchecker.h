
#ifndef UPDATE_CHECKER_H
#define UPDATE_CHECKER_H

#include "updtdialog.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QStandardPaths>
#include <QFile>

#include <iostream>

#ifdef GUI_BUILD_SINGLE
constexpr auto downloadfilename = "dwrgFpsUnlocker.exe";
constexpr auto updaterfilename = "dwrgFpsUnlocker.exe";
#else
constexpr auto downloadfilename = "dwrgFpsUnlocker.zip";
constexpr auto updaterfilename = "updater.exe";
#endif

class UpdateChecker : public QObject {
Q_OBJECT
    UpdateDialog& informer;
public:
    UpdateChecker(UpdateDialog &ifm, QObject *parent = nullptr);
    ~UpdateChecker();

    void checkUpdate();

    void doDownload();
signals:
    void noUpdateAvailable();
private:
    //网络管理器（实际上只会用一次）
    QNetworkAccessManager* manager;
    //
    class QElapsedTimer* downloadtimecost;

    void doUpdate(const QDir&);

    QUrl downloadurl;

    friend class UpdateDialog;
};

#endif //UPDATE_CHECKER_H
