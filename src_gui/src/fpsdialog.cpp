#include "ui_fpsdialog.h"
#include "fpsdialog.h"
#include "env.h"
#include "fpssetter.h"
#include "version.h"

#include <QMessageBox>
#include <QTime>
#include <QRandomGenerator>

FpsDialog::FpsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::dwrgFpsSetter)
    ,keepupdate(false)
{
    ui->setupUi(this);

    ui->fpscombox->setEditable(true);

    frupdatereminder = new QTimer(ui->curframerate);
    frupdatereminder->setInterval(1500);
    connect(frupdatereminder, &QTimer::timeout, this, &FpsDialog::updateFR);

    tmpreadtimer = new QTimer(ui->curframerate);
    tmpreadtimer->setSingleShot(true);
    tmpreadtimer->setInterval(6500);
    connect(tmpreadtimer, &QTimer::timeout, this, &FpsDialog::tempreadreach);

    frpalette = ui->curframerate->palette();

    checkchangePalette();

    ui->version->setText(Version(QString(VERSION_STRING)).toSimple());

    checkload();

    connect(ErrorReporter::instance(), &ErrorReporter::report, this, &FpsDialog::showError);
}

FpsDialog::~FpsDialog()
{
    if(ui->autoappradio->isChecked() || std::filesystem::exists("./hipp"))
        saveprofile();
    delete ui;
}

void FpsDialog::on_applybutton_pressed()
{
    setter->setFps(ui->fpscombox->currentText().toInt());
    set2tempread();
    Sleep(QRandomGenerator::global()->bounded(200, 700));
}

void FpsDialog::on_curframerate_clicked()
{
    keepupdate = !keepupdate;
    whileKeepUpdateChange();
}

void FpsDialog::updateFR()
{
    ui->curframerate->setText(QString::number(setter->getFps(), 'f', 1));
}

void bootfixup()
{
//    system(("tryfix "+std::to_string(DYRCX_P_OFFSET)+ ' ' + "dwrgFpsUnlocker.exe").c_str());
}

void FpsDialog::showError(const ErrorReporter::ErrorInfo& einf)
{
    QMessageBox::critical(this,einf.level,einf.msg);
//    if (einf.level == "修复")
//    {
//        atexit(bootfixup);
//            goto quit;
//    }
    if (einf.level == ErrorReporter::严重)
    {
        qCritical()<<einf.msg;
        emit ErrOccured();
    }
}

void FpsDialog::checkchangePalette()
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

void FpsDialog::whileKeepUpdateChange()
{
    if (keepupdate)
    {
        setter->pauseAutoX();
        frupdatereminder->start();
    }
    else
    {
        frupdatereminder->stop();
        setter->continueAutoX();
    }
    checkchangePalette();
}

void FpsDialog::set2tempread()
{
    keepupdate = true;
    whileKeepUpdateChange();
    tmpreadtimer->start();
}

void FpsDialog::tempreadreach()
{
    keepupdate = false;
    whileKeepUpdateChange();
}

void FpsDialog::saveprofile()const
{
    hipp = std::make_unique<std::fstream>("./hipp", std::ios::out | std::ios::binary);
    {
        if(hipp->is_open())
        {
                goto rewrite;
        }
        goto close;
    }
rewrite:
    hipp->seekp(0);

    {
        int fps = ui->fpscombox->currentText().toInt();
        hipp->write((char *) &fps, sizeof(fps));
        bool checked = ui->autoappradio->isChecked();
        hipp->write((char *) &checked, sizeof(checked));
    }
close:
    hipp->close();
}

// 字节序翻转工具函数
template <typename T>
T swapEndian(T value) {
    T swapped = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        swapped |= ((value >> (i * 8)) & 0xFF) << ((sizeof(T) - 1 - i) * 8);
    }
    return swapped;
}

bool FpsDialog::checkload() {
    if(std::filesystem::exists(std::filesystem::path("./hipp")))
    {
        hipp = std::make_unique<std::fstream>("./hipp", std::ios::in | std::ios::binary);
        if(hipp->is_open())
        {
            int fps;
            hipp->read((char*)&fps, sizeof(fps));

            bool uselast;
            hipp->read((char*)&uselast, sizeof(uselast));

            //处理QFile的端序残留问题
            if(fps > 0x0000FFFF || fps < 0)
            {
                fps = swapEndian(fps);
                uselast = swapEndian(uselast);
            }
            if(uselast)
            {
                ui->fpscombox->setCurrentText(QString::number(fps));
                ui->autoappradio->setChecked(uselast);
            }
        }
        else
        {
            ErrorReporter::instance()->receive({"错误", "无法访问文件 ./hipp "});
            return false;
        }
        hipp->close();
    }
    return true;
}

