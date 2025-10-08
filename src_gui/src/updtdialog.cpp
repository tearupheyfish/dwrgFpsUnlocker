#include "env.h"

#include "updtdialog.h"
#include "ui_updtdialog.h"

#include <QDesktopServices>
#include <QLayout>
#include <QTimer>

UpdateDialog *informer_r = nullptr;

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::UpdateInformer)
{
    ui->setupUi(this);

    setFixedSize(width(), height());

    ui->manual_button->hide();
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
}

void UpdateDialog::set_version(const QString &ver) {
    ui->version->setText(ver);
}

void UpdateDialog::on_updatebutton_pressed() {
    uc->doDownload();
}

// 切换到进度条模式
void UpdateDialog::switch_to_progress_bar()
{
    if (progressBar != nullptr)
        return;

    //隐藏版本号tag和更新按钮
    ui->version->hide();
    ui->updatebutton->hide();

    // 创建进度条控件
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);   // 范围为 0 - 100
    progressBar->setValue(0);        // 初始化为 0%
    progressBar->setGeometry(15, 8, 129, 20);
    progressBar->show();
}

void UpdateDialog::update_progress(qint64 bytesReceived, qint64 bytesTotal) {
    auto val = bytesTotal ? bytesReceived * 100 / bytesTotal : 0;
    progressBar->setValue(val);

    /* 事件 ① 耗时 > 7s ② 显示按钮 ③ 进度>20
     * 条件 ① 速率 < 50kB/s
     */
    static enum STAGE{SOON, LONGTIME, SHOWN, HOPEFUL}stage = SOON;
    auto usedtime = uc->downloadtimecost->elapsed();
    switch (stage)
    {
    case SOON:
        if (usedtime > 7000)
                stage = LONGTIME;
    case LONGTIME:
        //到时间才进行速率检测
        if (stage == LONGTIME && bytesReceived/usedtime < 50)
        {
            showManualButton();
            stage = SHOWN;
        }
    case SHOWN:
        if (val > 20)
        {
            hideManualButton();
            stage = HOPEFUL;
        }
    case HOPEFUL:
        ;
    }
}

void UpdateDialog::on_manual_button_pressed() {
//    emit InformerClose();
    QDesktopServices::openUrl(QUrl("https://github.com/tearupheyfish/dwrgFpsUnlocker/releases"));
}

void UpdateDialog::showManualButton() {
//    progressBar->hide();
    ui->manual_button->show();
    ui->manual_button->raise();
//    opaanim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UpdateDialog::hideManualButton() {
    ui->manual_button->hide();
}


