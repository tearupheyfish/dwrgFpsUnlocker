#include "macroes.h"
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
#include <QDebug>


UpdateChecker::UpdateChecker(UpdateDialog &ifm, QObject *parent)
:QObject(parent), informer(ifm)
{
    manager = new QNetworkAccessManager(this);

    downloadtimecost = new QElapsedTimer();
}

UpdateChecker::~UpdateChecker()
{
    delete manager;
    delete downloadtimecost;
}

std::pair<Version, QString> getLastestVersion(const QJsonArray& releases)
{
    Version latestVersion;
    QString downloadurl;

    for(const QJsonValue &releaseVal: releases) {
        QJsonObject releaseObj = releaseVal.toObject();
#ifdef PRE_RELEASE
        if(releaseObj["prerelease"].toBool())
#endif
        {
            auto tag_name = releaseObj["tag_name"].toString();
            Version version(tag_name);
            if(version > latestVersion) {
                QJsonArray assets = releaseObj["assets"].toArray();
                for (const QJsonValue &assetVal : assets) {
                    QJsonObject asset = assetVal.toObject();
                    if (asset["name"].toString() == downloadfilename) {
                        //要同时有新版本和可下载的文件才算有可用更新
                        downloadurl = asset["browser_download_url"].toString();
                        latestVersion = version;
                        break;
                    }
                }
            }
        }
    }

    return {latestVersion, downloadurl};
}

void UpdateChecker::checkUpdate() {
    QUrl query_url(
#ifdef PRE_RELEASE
    "https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases"
#else
"https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases/latest"
#endif
    );

    QNetworkRequest request(query_url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "dwrgFpsUnlocker(Windows; x64)"); // GitHub 要求

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::errorOccurred, [=]() {
        // ErrorReporter::instance()->receive(ErrorReporter::警告, "检查更新失败");
            qWarning()<<"更新失败：无法访问发布页";
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if(reply->error() != QNetworkReply::NoError)
        {
            reply->deleteLater();
            emit noUpdateAvailable();
            return;
        }
        //查询完成且没有出错的话↓

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

        QJsonArray releases
#ifdef PRE_RELEASE
         = doc.array();
#else
        = {doc.object()};
#endif

        Version latestVersion;
        std::tie(latestVersion, downloadurl) = getLastestVersion(releases);

        if (latestVersion > QApplication::applicationVersion()) {
                informer.set_version(latestVersion.toQString());
                informer.show();

                informer.raise();
                QApplication::alert(&informer);
        } else {
            emit noUpdateAvailable();
        }

        reply->deleteLater();
    });
}

void UpdateChecker::doDownload() {
    QDir saveDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)+'/'+QApplication::applicationName();//默认为可执行文件名称

    if (!saveDir.exists()
        && !saveDir.mkpath(saveDir.path()))
    {
        // ErrorReporter::instance()->receive(ErrorReporter::警告, "无法创建下载目录");
        informer.set_version("无法创建下载目录");
        qWarning()<<"无法创建下载目录: "<<saveDir.path();
        informer.showManualButton();
        return;
    }

    QString savePath = saveDir.filePath(downloadfilename);
    QNetworkReply* reply = manager->get(QNetworkRequest(downloadurl));
    qInfo()<<"开始下载: "<<downloadurl;
    informer.switch_to_progress_bar();
    downloadtimecost->start();

    QFile* file = new QFile(savePath, this);
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "分配下载文件失败，路径:"<<savePath;
        return;
    }

    connect(reply, &QNetworkReply::errorOccurred, [&](QNetworkReply::NetworkError error) {
            qWarning()<<"下载出错："<<error;
            informer.showManualButton();
    });

    connect(reply, &QNetworkReply::readyRead, [=]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, &informer , &UpdateDialog::update_progress);

    connect(reply, &QNetworkReply::finished, [&]() {
        file->flush();
        file->close();
        file->deleteLater();
        reply->deleteLater();

        delete downloadtimecost;
        downloadtimecost = nullptr;
        if(reply->error() == QNetworkReply::NoError)
            doUpdate(saveDir);
    });

}

void UpdateChecker::doUpdate(const QDir& saveDir)
{
#ifndef GUI_BUILD_SINGLE
    if (!QFile::exists(QDir::currentPath()+"./updater.exe"))
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
#else
    if(QProcess::startDetached(
            saveDir.filePath(updaterfilename),
#endif
            {saveDir.filePath(downloadfilename), QDir::currentPath(), QString::number(QCoreApplication::applicationPid())},
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
    QDesktopServices::openUrl(QUrl::fromLocalFile(saveDir.path()));
    QCoreApplication::quit();
}



