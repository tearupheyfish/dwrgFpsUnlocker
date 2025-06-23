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


void UpdateChecker::checkUpdate() {
    // 示例：repo = "yourusername/yourrepo"
    QUrl url("https://api.github.com/repos/tearupheyfish/dwrgFpsUnlocker/releases/latest");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "dwrgFpsUnlocker(Windows 10; x64)"); // GitHub 要求

    QNetworkReply* reply = manager->get(request);

    connect(reply, &QNetworkReply::errorOccurred, [=]() {
        QMetaObject::invokeMethod(qApp, [&reply](){
            ErrorReporter::instance()->receive("检查更新", reply->errorString());
        });
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error()) {
            qWarning() << "Update check failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = doc.object();

        QString latestVersion = obj["tag_name"].toString().section('-', 0, 0); // 例如 v1.2
        QString currentVersion = VERSION_STRING; // 你的当前版本（也可以从宏里读取）

        if (latestVersion > currentVersion) {
            qDebug() << "Update available:" << latestVersion;

            QJsonArray assets = obj["assets"].toArray();
            for (const QJsonValue &assetVal: assets) {
                QJsonObject asset = assetVal.toObject();
                QString name = asset["name"].toString();

                if (name == "dwrgFpsUnlocker.zip") {
                    downloadurl = asset["browser_download_url"].toString();
                    break;
                }
            }

            QMetaObject::invokeMethod(qApp, [&latestVersion](){
                informer_r->set_version(latestVersion);
                informer_r->show();
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

    QFile* file = new QFile(savePath);
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save pkg.";
        return;
    }

    connect(reply, &QNetworkReply::errorOccurred, [=](QNetworkReply::NetworkError error) {
        qWarning() << "Download failed:" << reply->errorString();
        ErrorReporter::instance()->receive("下载失败", reply->errorString());
        reply->deleteLater();
        file->deleteLater();
    });

    connect(reply, &QNetworkReply::readyRead, [=]() {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, informer_r , &UpdateInformer::update_progress);

    connect(reply, &QNetworkReply::finished, [=]() {
        file->flush();
        file->close();
        reply->deleteLater();
        file->deleteLater();

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