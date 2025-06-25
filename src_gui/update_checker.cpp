#include "env.h"

#include "update_checker.h"
#include "errreport.h"
#include "updateinformer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QApplication>

struct Version{
    uint8_t major;
    uint8_t minor;

    Version(const QString& vstr)
    {
        QString numStr = vstr.mid(1); // Remove 'v'
        major = numStr.section('.', 0, 0).toInt();
        minor = numStr.section('.', 1).section('-', 0, 0).toInt();
    }
    Version(const std::string& vstr)
    :Version(QString::fromStdString(vstr))
    {}

    bool operator <(const Version& right)const
    {
        return major != right.major ? major < right.major : minor < right.minor;
    }
    bool operator >(const Version& right)const
    {
        return major != right.major ? major > right.major : minor > right.minor;
    }

    operator QString()const{
        return QString("v%1.%2").arg(major).arg(minor);
    }
};

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
    speedtesttimer = new QTimer(this);
    speedtesttimer->setSingleShot(true);
    speedtesttimer->setInterval(10*1000);
    connect(speedtesttimer, &QTimer::timeout, [](){
        if(informer_r->progressBar->value() < 2)
        {
            informer_r->showManualButton();
        }
    });
}

void UpdateChecker::checkUpdate() {
    // 示例：repo = "yourusername/yourrepo"
    QUrl url(
#ifdef PRERELEASE
    "https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases"
#else
"https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases/latest"
#endif
    );

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "dwrgFpsUnlocker(Windows 10; x64)"); // GitHub 要求

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::errorOccurred, [=]() {
        ErrorReporter::instance()->receive(ErrorReporter::警告, "检查更新失败");
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());

//        QJsonObject obj = doc.object();

        Version currentVersion = std::string(VERSION_STRING); // 你的当前版本（也可以从宏里读取）
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
                Version version = releaseObj["tag_name"].toString().section('-', 0, 0);
                if(version > latestVersion) {
                    latestVersion = version;

                    QJsonArray assets = releaseObj["assets"].toArray();
                    for (const QJsonValue &assetVal : assets) {
                        QJsonObject asset = assetVal.toObject();
                        if (asset["name"].toString() == "dwrgFpsUnlocker.zip") { // 替换为你的预期文件名
                            downloadurl = asset["browser_download_url"].toString();
                            break;
                        }
                    }
                }
            }
        }

        if (latestVersion > currentVersion) {
            qDebug() << "Update available:" << latestVersion;

            QMetaObject::invokeMethod(qApp, [&latestVersion](){
                informer_r->set_version(latestVersion);
                informer_r->show();

                informer_r->raise();
                QApplication::alert(informer_r);
            });

        } else {
            qDebug() << "Already up to date.";
        }

        reply->deleteLater();
    });
}

void UpdateChecker::downloadPacakge(const QString &url, const QString &filename) {
    QString saveDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/dwrgFpsUnlocker";
    QDir saveDir(saveDirPath);
    if (!saveDir.exists())
    {
        saveDir.mkpath(saveDirPath);
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
        QMetaObject::invokeMethod(qApp, [](){
            informer_r->showManualButton();
        });
    });

    connect(reply, &QNetworkReply::readyRead, [=]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, informer_r , &UpdateInformer::update_progress);

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
                {savePath, QDir::currentPath()},
                nullptr
                ))
        {
                    informer_r->close();
                    QCoreApplication::quit(); // 退出当前进程
                    return;
        }
        else
        {
            QMessageBox::information(nullptr, "出问题", "启动updater.exe失败；尝试手动更新？");
        }

        manually:
        informer_r->close();
        QDesktopServices::openUrl(saveDirPath);
        QCoreApplication::quit();
    });
}

void UpdateChecker::Update() {
    informer_r->switch_to_progress_bar();
    downloadPacakge(downloadurl, filename);
}


