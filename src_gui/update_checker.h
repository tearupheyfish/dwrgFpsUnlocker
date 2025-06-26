
#ifndef UPDATE_CHECKER_H
#define UPDATE_CHECKER_H

#include "macroes.h"
#include "updateinformer.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QStandardPaths>
#include <QFile>

#include <iostream>

class UpdateChecker : public QObject {
Q_OBJECT

    QString downloadurl;
    static const QString filename;
    UpdateInformer& informer;
public:
    UpdateChecker(UpdateInformer &ifm, QObject *parent = nullptr);

    void checkUpdate();

    void Update();
signals:
    void noUpdateAvailable();
private:
    QNetworkAccessManager* manager;
    QTimer* speedtesttimer;

    void downloadPacakge(const QString& url, const QString& filename);

};

inline const QString UpdateChecker::filename = "dwrgFpsUnlocker.zip";


#endif //UPDATE_CHECKER_H
