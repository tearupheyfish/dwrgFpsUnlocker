#ifndef FPSDIALOG_H
#define FPSDIALOG_H

#include "errreport.h"

#include <QDialog>

#include "env.h"

#include "ui_fpsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class dwrgFpsSetter;
}
QT_END_NAMESPACE

class FpsDialog : public QDialog
{
    Q_OBJECT
    Ui::dwrgFpsSetter *ui;
    FpsSetter* setter;

    QTimer *frupdatereminder; // 帧率tag刷新定时器
    bool keepupdate;
    QPalette frpalette;

    QTimer *tmpreadtimer; // 临时帧率显示定时器

    FpsDialog(QWidget *parent = nullptr);
public:
    static FpsDialog* create();
    ~FpsDialog();

    void set2tempread();
    void dobuttonpress()
    {
        on_applybutton_pressed();
    }
signals:
    void MainWinClose();
    void ErrOccured();
private slots:
    void on_applybutton_pressed();
    // void on_applybutton_released() = delete;
    void on_curframerate_clicked();

    void showError(const ErrorReporter::ErrorInfo& einf);

private:
    bool checkload();
    void saveprofile()const;

    void tempreadreach();
    void checkchangePalette();
    void whileKeepUpdateChange();

    void updateFR();
    // QGraphicsDropShadowEffect* buttonShadow;
    void applyFPS(int fps);

    void closeEvent(QCloseEvent* event) override
    {
        emit MainWinClose();
    }
};
#endif // FPSDIALOG_H
