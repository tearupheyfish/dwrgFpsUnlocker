#ifndef ENV_H
#define ENV_H

#include "fpsSetter.h"
#include <QTimer>
#include <QFile>
#include <memory>

inline std::unique_ptr<FpsSetter> setter;
inline std::shared_ptr<QFile> hipp(
        new QFile("hipp"),
        [](QFile*f)
        {
                if(f->isOpen())
                        f->close();
                delete f;
        });;//-ocampus

class Dialog;
extern Dialog* dl;

#endif // ENV_H
