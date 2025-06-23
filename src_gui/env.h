#ifndef ENV_H
#define ENV_H

#include "fpssetter.h"
#include "update_checker.h"

#include <QTimer>
#include <QFile>
#include <memory>

inline std::unique_ptr<FpsSetter> setter;
inline std::unique_ptr<UpdateChecker> uc;

inline std::shared_ptr<QFile> hipp(
        new QFile("hipp"),
        [](QFile*f)
        {
                if(f->isOpen())
                        f->close();
                delete f;
        });;//-ocampus

class Dialog;
extern Dialog* dl_r;
class UpdateInformer;
extern UpdateInformer *informer_r;

#endif // ENV_H
