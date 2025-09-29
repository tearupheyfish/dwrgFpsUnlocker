//
// Created by Tofu on 2025/5/15.
//

#ifndef ERRREPORT_H
#define ERRREPORT_H

#include <QObject>
#include <QQueue>
#include <QMutex>

class ErrorReporter: public QObject
{
    Q_OBJECT
public:
    static constexpr char 严重[] = "_(´ཀ`」 ∠)_";
    static constexpr char 警告[] = "危";
    struct ErrorInfo {
        QString level;
        QString msg;
    };
    static ErrorReporter* instance();
    void receive(const ErrorInfo& einfo);
    void receive(QString level, QString msg);
signals:
    void report(const ErrorInfo& einfo);
private:
explicit
    ErrorReporter(QObject *parent = nullptr);
    QMutex mutex;
    QQueue<ErrorInfo> errqueue;
};

#endif