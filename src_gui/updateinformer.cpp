#include "env.h"

#include "updateinformer.h"
#include "ui_updateinformer.h"

#include <QDesktopServices>
#include <QLayout>
#include <QPropertyAnimation>

UpdateInformer *informer_r = nullptr;

UpdateInformer::UpdateInformer(QWidget *parent)
    : QDialog(parent), ui(new Ui::UpdateInformer)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
//    opaeff = new QGraphicsOpacityEffect(ui->manual_button);
//    opaeff->setOpacity(0);
//    ui->manual_button->setGraphicsEffect(opaeff);
    ui->manual_button->hide();

//    opaanim = new QPropertyAnimation(opaeff, "opacity");
//    opaanim->setDuration(1000);
//    opaanim->setStartValue(0);
//    opaanim->setEndValue(1);
//    opaanim->setEasingCurve(QEasingCurve::Linear);
}

UpdateInformer::~UpdateInformer()
{
    delete ui;
}

void UpdateInformer::set_version(const QString &ver) {
    ui->version->setText(ver);
}

void UpdateInformer::on_updatebutton_pressed() {
    uc->Update();
}

// 切换到进度条模式
void UpdateInformer::switch_to_progress_bar()
{
    // 如果已有进度条，则直接返回
    if (progressBar != nullptr)
        return;

    ui->version->hide();
    ui->updatebutton->hide();

    // 创建进度条控件
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);   // 范围为 0 - 100
    progressBar->setValue(0);        // 初始化为 0%
    progressBar->setGeometry(15, 8, 129, 20);
    progressBar->show();
}

void UpdateInformer::update_progress(qint64 bytesReceived, qint64 bytesTotal) {
    auto val = bytesTotal ? bytesReceived * 100 / bytesTotal : 0;
    progressBar->setValue(val);
    if(speedcheck == true)
    {
        if(bytesReceived/10.f/1024 < 50)
            showManualButton();
        speedcheck = false;
    }
    if(!ui->manual_button->isHidden() && val > 50)
    {
        hideManualButton();
    }
}

void UpdateInformer::on_manual_button_pressed() {
//    emit InformerClose();
    QDesktopServices::openUrl(QUrl("https://github.com/tearupheyfish/dwrgFpsUnlocker/releases"));
}

void UpdateInformer::showManualButton() {
//    progressBar->hide();
    ui->manual_button->show();
    ui->manual_button->raise();
//    opaanim->start(QAbstractAnimation::DeleteWhenStopped);
}

void UpdateInformer::hideManualButton() {
    ui->manual_button->hide();
}


