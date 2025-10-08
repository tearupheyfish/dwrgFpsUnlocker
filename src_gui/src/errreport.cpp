//
// Created by Tofu on 2025/5/15.
//
#include "errreport.h"

ErrorReporter* ErrorReporter::instance() {
    static ErrorReporter manager;  // 单例实例
    return &manager;
}

ErrorReporter::ErrorReporter(QObject *parent)
    : QObject(parent) {}

void ErrorReporter::receive(const ErrorInfo& error) {
    QMutexLocker locker(&mutex);    // 加锁保证线程安全

    // 发射信号通知消费错误
    emit report(error);
}

void ErrorReporter::receive(QString level, QString msg)
{
    receive({std::move(level), std::move(msg)});
}