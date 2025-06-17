//
// Created by Tofu on 2025/5/15.
//

#ifndef AUTOXTIMERPROXY_H
#define AUTOXTIMERPROXY_H

#include "fpsSetter.h"

class autoxTimerProxy
{
public:
    FpsSetter& fpssetter;
    struct Timer
    {
        Timer(FpsSetter& setter)
            :fpss(setter)
        {}
        FpsSetter& fpss;
        void start()
        {
            // fpss.closeHandle();
        }
        void stop(){};
    }timer;
    autoxTimerProxy(FpsSetter& setter)
        : fpssetter(setter), timer(setter)
    {
    }
};

#endif //AUTOXTIMERPROXY_H
