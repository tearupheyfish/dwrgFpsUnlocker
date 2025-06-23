//
// Created by Tofu on 2025/5/15.
//

#ifndef AUTOXTIMERPROXY_H
#define AUTOXTIMERPROXY_H

#include "../common/fpssetter.h"
#include "errreport.h"

#include <QObject>
#include <QTimer>


class autoxTimerProxy: public QObject
{
    Q_OBJECT
public:
    FpsSetter& belongsto;
    QTimer timer;
    autoxTimerProxy(FpsSetter& master) :belongsto(master)
    {
        timer.setSingleShot(true);
        timer.setInterval(15000);
        connect(&timer, &QTimer::timeout, this, &autoxTimerProxy::xhandlerauto);
    }
    ~autoxTimerProxy()
    {
        timer.stop();
        timer.disconnect();
    }
private slots:
    void xhandlerauto()
    {
        belongsto.closeHandle();
#ifdef QT_DEBUG
        ErrorReporter::instance()->receive("提醒", "句柄关闭定时任务执行。");
#endif
    }
    // void start()
    // {
    //     timer.start();
    // }
    // void stop()
    // {
    //     timer.stop();
    // }
};

#endif //AUTOXTIMERPROXY_H
