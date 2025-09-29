#ifndef UPDATEINFORMER_H
#define UPDATEINFORMER_H

#include <QDialog>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {
class UpdateInformer;
}

class UpdateDialog : public QDialog
{
    Q_OBJECT

    Ui::UpdateInformer *ui;
    QProgressBar* progressBar = nullptr;
    class UpdateChecker *uc;
    bool speedcheck = false;
public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();
    void setRelativeData(UpdateChecker& checker)
    {
        uc = &checker;
    }

    void set_version(const QString& ver);

    void switch_to_progress_bar();                 // 切换为进度条

signals:
    void InformerClose();


//    QGraphicsOpacityEffect* opaeff;
//    QPropertyAnimation *opaanim;

private slots:
    void on_updatebutton_pressed(); // 更新进度条进度
    void on_manual_button_pressed();

    void update_progress(qint64 bytesReceived, qint64 bytesTotal);

    friend class UpdateChecker;

    void showWindow(){show();};
    void showManualButton();
    void hideManualButton();

    void closeEvent(QCloseEvent *) override
    {
        emit InformerClose();
    }

};

#endif // UPDATEINFORMER_H
