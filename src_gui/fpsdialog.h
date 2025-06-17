#ifndef FPSDIALOG_H
#define FPSDIALOG_H

#include "errreport.h"

#include <QDialog>
#include <QGraphicsDropShadowEffect>
#include <QTimer>

#include "env.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class dwrgFpsSetter;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void saveprofile()const;
    Ui::dwrgFpsSetter *getui();

    void set2tempread();
    void dobuttonpress()
    {
        on_applybutton_pressed();
    }

private slots:
    void on_applybutton_pressed();
    // void on_applybutton_released() = delete;
    void on_curframerate_clicked();

private:
    Ui::dwrgFpsSetter *ui;

    QTimer *frupdatereminder;
    bool keepupdate;
    QPalette frpalette;

    QTimer *tmpreadtimer;

    void tempreadreach();

    void checkchangePalette();
    void whilekeepupdatechange();

    void updateFR();
    // QGraphicsDropShadowEffect* buttonShadow;
    void showError(const ErrorReporter::ErrorInfo& einf);

    void closeEvent(QCloseEvent* event) override
    {
        setter.release();
    }
};
#endif // FPSDIALOG_H
