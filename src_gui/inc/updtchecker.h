
#ifndef UPDATE_CHECKER_H
#define UPDATE_CHECKER_H

#include "updtdialog.h"

#include <QNetworkReply>
#include <QFile>
#include <QTimer>

#include <iostream>

class UpdateChecker : public QObject {
Q_OBJECT
    UpdateDialog& informer;
    //网络管理器（实际上只会用一次）
    QNetworkAccessManager* manager;
    QUrl downloadurl;
    QString hash2cert;
    //本质是记录时刻然后作差，所以不用担心放着影响性能
    QElapsedTimer downloadtimecost;
public:
    UpdateChecker(UpdateDialog &ifm, QObject *parent = nullptr);
    ~UpdateChecker() override;

    void doDownload();
signals:
    void noUpdateAvailable();
public slots:
    void checkUpdate();

private:
    static void doUpdate(const QDir&);
};

#endif //UPDATE_CHECKER_H
