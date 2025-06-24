#include "ui_fpsdialog.h"
#include "fpsdialog.h"
#include "env.h"
#include <QMessageBox>
#include <QTime>
#include <QRandomGenerator>

Dialog *dl_r = nullptr;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::dwrgFpsSetter)
    ,keepupdate(false)
{
    ui->setupUi(this);

    ui->fpscombox->setEditable(true);

    frupdatereminder = new QTimer(ui->curframerate);
    frupdatereminder->setInterval(1500);
    connect(frupdatereminder, &QTimer::timeout, this, &Dialog::updateFR);

    tmpreadtimer = new QTimer(ui->curframerate);
    tmpreadtimer->setSingleShot(true);
    tmpreadtimer->setInterval(6500);
    connect(tmpreadtimer, &QTimer::timeout, this, &Dialog::tempreadreach);

    frpalette = ui->curframerate->palette();

    checkchangePalette();

    connect(ErrorReporter::instance(), &ErrorReporter::report, this, &Dialog::showError);

    ui->version->setText(VERSION_STRING);
    dl_r = this;
}

Dialog::~Dialog()
{
    if(ui->autoappradio->isChecked() || hipp->exists())
        saveprofile();
    delete ui;
}

void Dialog::on_applybutton_pressed()
{
    setter->setFps(ui->fpscombox->currentText().toInt());
    set2tempread();
    Sleep(QRandomGenerator::global()->bounded(200, 700));
}

void Dialog::on_curframerate_clicked()
{
    keepupdate = !keepupdate;
    whilekeepupdatechange();
}

void Dialog::updateFR()
{
    ui->curframerate->setText(QString::number(setter->getFps(), 'f', 1));
}

void Dialog::showError(const ErrorReporter::ErrorInfo& einf)
{
    QMessageBox::critical(this,
        einf.level,
        einf.msg);
    if (einf.level == ErrorReporter::严重)
        QApplication::exit(-1);
}

void Dialog::checkchangePalette()
{
    if(keepupdate)
    {
        frpalette.setColor(QPalette::ButtonText, QColor(0x067d17));
        updateFR();
    }
    else
    {   frpalette.setColor(QPalette::ButtonText, Qt::gray);
        ui->curframerate->setText("--.-");
    }
    ui->curframerate->setPalette(frpalette);
}

void Dialog::whilekeepupdatechange()
{
    if (keepupdate)
    {
        setter->pauseautox();
        frupdatereminder->start();
    }
    else
    {
        frupdatereminder->stop();
        setter->continueautox();
    }
    checkchangePalette();
}

void Dialog::set2tempread()
{
    keepupdate = true;
    whilekeepupdatechange();
    tmpreadtimer->start();
}

void Dialog::tempreadreach()
{
    keepupdate = false;
    whilekeepupdatechange();
}

void Dialog::saveprofile()const
{
    if(hipp->exists())
    {
        if(hipp->isOpen())
        {
            if(hipp->isWritable())
            {
                goto rewrite;
            }
            hipp->close();
        }
    }
openwrite:
    hipp->open(QIODevice::WriteOnly);
    if(!hipp->isOpen())
        ErrorReporter::instance()->receive("出错","无法写入文件");
rewrite:
    hipp->seek(0);

    QDataStream wds(hipp.get());
    wds<<ui->fpscombox->currentText().toInt();
    wds<<ui->autoappradio->isChecked();
}

Ui::dwrgFpsSetter* Dialog::getui()
{
    return ui;
}

