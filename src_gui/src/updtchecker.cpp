#include "env.h"

#include "errreport.h"
#include "updtchecker.h"
#include "updtdialog.h"
#include "version.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QApplication>


UpdateChecker::UpdateChecker(UpdateDialog &ifm, QObject *parent)
:QObject(parent), informer(ifm)
{
    manager = new QNetworkAccessManager(this);

    speedtesttimer = new QTimer(this);
    speedtesttimer->setSingleShot(true);
    speedtesttimer->setInterval(10*1000);//判断间隔：10s
    connect(speedtesttimer, &QTimer::timeout,[this](){
        if(this->informer.progressBar->value() < 2)//若进度小于2则判断为过慢
        {
            this->informer.showManualButton();//则提议手动下载
        }
    });
}

void UpdateChecker::checkUpdate() {
    QUrl url(
#ifdef PRERELEASE
    "https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases"
#else
"https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases/latest"
#endif
    );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "dwrgFpsUnlocker(Windows; x64)"); // GitHub 要求

    QNetworkReply* reply = manager->get(request);

    //question: 不同用户群的代码调整借助宏判断吗？
//    connect(reply, &QNetworkReply::errorOccurred, [=]() {
//        ErrorReporter::instance()->receive(ErrorReporter::警告, "检查更新失败");
//    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if(reply->error() != QNetworkReply::NoError)
        {
            reply->deleteLater();
            emit noUpdateAvailable();
            return;
        }
        //查询完成且没有出错的话↓

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

//        QJsonObject obj = doc.object();

        //question: 为何需要显式转换？
        Version currentVersion(VERSION_STRING);
        Version latestVersion = currentVersion;
        QJsonArray releases
#ifdef PRERELEASE
         = doc.array();
#else
        = {doc.object()};
#endif
        for(const QJsonValue &releaseVal: releases) {
            QJsonObject releaseObj = releaseVal.toObject();
#ifdef PRERELEASE
            if(releaseObj["prerelease"].toBool())
#endif
            {
                Version version(releaseObj["tag_name"].toString());
                if(version > latestVersion) {
                    latestVersion = version;

                    QJsonArray assets = releaseObj["assets"].toArray();
                    for (const QJsonValue &assetVal : assets) {
                        QJsonObject asset = assetVal.toObject();
                        if (asset["name"].toString() ==
                    #ifdef BUILD_SINGLE
                                        "dwrgFpsUnlocker.exe"
                    #else
                                        "dwrgFpsUnlocker.zip"
                    #endif
                            ) {
                            downloadurl = asset["browser_download_url"].toString();
                            break;
                        }
                    }
                }
            }
        }

        if (latestVersion > currentVersion) {
                informer.set_version(latestVersion);
                informer.show();

                informer.raise();
                QApplication::alert(&informer);
        } else {
            emit noUpdateAvailable();
        }

        reply->deleteLater();
    });
}

void UpdateChecker::downloadPacakge(const QString &url, const QString &filename) {
    QString saveDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/dwrgFpsUnlocker";
    QDir saveDir(saveDirPath);

    if (!saveDir.exists()
        && !saveDir.mkpath(saveDirPath))
    {
        informer.showManualButton();
        return;
    }

    QString savePath = saveDir.filePath(filename);
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(url)));
    speedtesttimer->start();

    QFile* file = new QFile(savePath);
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save pkg.";
        return;
    }

    connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError error) {
            informer.showManualButton();
    });

    connect(reply, &QNetworkReply::readyRead, [=]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, &informer , &UpdateDialog::update_progress);

    connect(reply, &QNetworkReply::finished, [=]() {
        file->flush();
        file->close();
        file->deleteLater();
        reply->deleteLater();

        if(reply->error() != QNetworkReply::NoError)
            return;

        if (!QFile::exists(QDir::currentPath() + "/updater.exe"))
        {
            QMessageBox::information(nullptr, "出问题", "找不到updater.exe；尝试手动更新？");
            goto manually;
        }

        if(QFile::exists(saveDir.filePath("updater.exe")))
        {
            QFile::remove(saveDir.filePath("updater.exe"));
        }
        if(!QFile::copy(
                QDir::currentPath().append("/updater.exe"),
                saveDir.filePath("updater.exe")
                ))
        {
            QMessageBox::information(nullptr, "出问题", "移动updater.exe失败；尝试手动更新？");
            goto manually;
        }

        if(QProcess::startDetached(
                saveDir.filePath("updater.exe"),
                {savePath, QDir::currentPath(), QString::number(QCoreApplication::applicationPid())},
                nullptr
                ))
        {
                    QCoreApplication::quit();
                    return;
        }
        else
        {
            QMessageBox::information(nullptr, "出问题", "启动updater.exe失败；尝试手动更新？");
        }

        manually:
        QDesktopServices::openUrl(saveDirPath);
        QCoreApplication::quit();
    });
}

void UpdateChecker::Update() {
    informer.switch_to_progress_bar();
    downloadPacakge(downloadurl, filename);
}


