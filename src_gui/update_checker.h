
#ifndef UPDATE_CHECKER_H
#define UPDATE_CHECKER_H

#include "macroes.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QStandardPaths>
#include <QFile>

#include <iostream>

class UpdateChecker : public QObject {
Q_OBJECT

    QString downloadurl;
    static const QString filename;
public:
    UpdateChecker(QObject* parent = nullptr) : QObject(parent) {
        manager = new QNetworkAccessManager(this);
    }

    void checkUpdate();

    void Update();

private:
    QNetworkAccessManager* manager;

    void downloadPacakge(const QString& url, const QString& filename);

};

inline const QString UpdateChecker::filename = "dwrgFpsUnlocker.zip";


#endif //UPDATE_CHECKER_H
