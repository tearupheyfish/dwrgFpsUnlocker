#ifndef UPDATEINFORMER_H
#define UPDATEINFORMER_H

#include <QDialog>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace Ui {
class UpdateInformer;
}

class UpdateInformer : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateInformer(QWidget *parent = nullptr);
    ~UpdateInformer();
    void set_version(const QString& ver);

    void switch_to_progress_bar();                 // 切换为进度条

private:
    Ui::UpdateInformer *ui;
    QProgressBar* progressBar = nullptr;

//    QGraphicsOpacityEffect* opaeff;
//    QPropertyAnimation *opaanim;

private slots:
    void on_updatebutton_pressed(); // 更新进度条进度
    void on_manual_button_pressed();
    void update_progress(qint64 bytesReceived, qint64 bytesTotal);

    friend class UpdateChecker;

    void showManualButton();

};

#endif // UPDATEINFORMER_H
