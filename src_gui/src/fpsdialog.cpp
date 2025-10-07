#include "ui_fpsdialog.h"
#include "fpsdialog.h"
#include "storage.h"
#include "fpssetter.h"
#include "version.h"

#include <QHotKey>

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

    tmpreadtimer = new QTimer(ui->curframerate);//可能在setter销毁之后触发
    tmpreadtimer->setSingleShot(true);
    tmpreadtimer->setInterval(6500);
    connect(tmpreadtimer, &QTimer::timeout, this, &FpsDialog::tempreadreach);

    frpalette = ui->curframerate->palette();

    checkchangePalette();

    ui->version->setText(Version(QApplication::applicationVersion()).toQString(2));

    checkload();

    QHotkey* f5 = new QHotkey(QKeySequence(Qt::Key_F5), this);

    connect(f5, &QHotkey::activated, this, [&]()
    {
        static bool usedefault = false;
        usedefault = !usedefault;
        if (usedefault)
            applyFPS(60);
        else
            on_applybutton_pressed();
    });
    connect(ErrorReporter::instance(), &ErrorReporter::report, this, &FpsDialog::showError, Qt::QueuedConnection);
}

FpsDialog* FpsDialog::create()
{
    auto w = new FpsDialog;
    w->setter =FpsSetter::create();
    return w;
}

FpsDialog::~FpsDialog()
{
    saveprofile();
    delete setter;
    delete ui;
}

void FpsDialog::on_applybutton_pressed()
{
    const static QRegularExpression intfpschecker("^[0-9]+$");
    auto fpsstr = ui->fpscombox->currentText();
    if (fpsstr.isEmpty())
        fpsstr = "60";
    else if (!intfpschecker.match(fpsstr).hasMatch())
    {
        ErrorReporter::instance()->receive(ErrorReporter::ErrorInfo{"错误", "输入的帧数需要是正整数"});
        return;
    }

    applyFPS(fpsstr.toInt()/*不合法的数值将会返回0*/);
}

void FpsDialog::applyFPS(int fps)
{
    setter->setFps(fps);
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
    if (einf.level == ErrorReporter::严重)
    {
        qCritical()<<einf.msg;
        // emit ErrOccured();
        this->close();
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

/** 有记忆 -> 读取fps和check
 *    ◇ ->
 */

void FpsDialog::saveprofile()const
{
    if(! ui->autoappradio->isChecked())return ;

    Storage<hipp> hipp;
    auto fpsstr = ui->fpscombox->currentText();
    if (fpsstr.isEmpty())
    {
        hipp.clear();
        return ;
    }
    if(hipp)
    {
        int fps = fpsstr.toInt();
        bool ret = hipp.save<&hipp::fps>(fps);
        assert(ret);
        bool checked = ui->autoappradio->isChecked();
        ret = hipp.save<&hipp::checked>(checked);
        assert(ret);
    }
}

// 翻转字节序
template <typename T>
T swapEndian(T value) {
    T swapped = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
        swapped |= ((value >> (i * 8)) & 0xFF) << ((sizeof(T) - 1 - i) * 8);
    }
    return swapped;
}

bool FpsDialog::checkload() {
    //如果存在hipp文件 (有过记录)
    Storage<hipp> hipp;
    if(hipp.exist())
    {
        //如果文件可访问
        if(hipp)
        {
            //读取fps和uselast
            int fps = hipp.load<&hipp::fps>();
            bool uselast = hipp.load<&hipp::checked>();

            //处理QFile的端序残留问题
            //               0x0000FFFF while int32
            if(fps > (0b1<<(sizeof(int)*8/2))-1 || fps < 0)
            {
                fps = swapEndian(fps);
                uselast = swapEndian(uselast);
            }
            //如果uselast
            ui->fpscombox->setCurrentText(QString::number(fps));
            if(uselast)
            {
                //todo: uselast 紧接 autoapply 还是分开？
                ui->autoappradio->setChecked(uselast);
            }
        }
        else
        {
            ErrorReporter::instance()->receive({"出错", "无法读写存储"});
            return false;
        }
    }
    return true;
}

