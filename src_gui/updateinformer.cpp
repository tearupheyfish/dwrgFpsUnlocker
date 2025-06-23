#include "env.h"

#include "updateinformer.h"
#include "ui_updateinformer.h"

#include <QLayout>

UpdateInformer *informer_r = nullptr;

UpdateInformer::UpdateInformer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateInformer)
{
    ui->setupUi(this);
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
    progressBar->setGeometry(10, 10, 150, 20);
    progressBar->show();
}

void UpdateInformer::update_progress(qint64 bytesReceived, qint64 bytesTotal) {
    progressBar->setValue(bytesReceived * 100 / bytesTotal);
}

