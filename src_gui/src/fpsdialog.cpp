#include "fpsdialog.h"

#include "version.h"

#include "errreport.h"
#include "storage.h"
#include "fpssetter.h"

#include <QRandomGenerator>

FpsDialog::FpsDialog(DWORD pid, QWidget *parent)
    :QDialog(parent), ui(new Ui::dwrgFpsSetter)
    ,setter(FpsSetter::create(pid)), keepupdate(false), painlesssuicide(nullptr)
{
    ui->setupUi(this);

    frpalette = ui->curframerate->palette();
    checkchangePalette();

    frupdateremider = new QTimer(this);
    frupdateremider->setInterval(1500);
    connect(frupdateremider, &QTimer::timeout, this, &FpsDialog::updateFR);

    tmpreadtimer = new QTimer(this);
    tmpreadtimer->setSingleShot(true);
    tmpreadtimer->setInterval(6600);
    connect(tmpreadtimer, &QTimer::timeout, this, &FpsDialog::dissmissFR);

    ui->version->setText(Version(QApplication::applicationVersion()).toQString(2));

    loadpreset();

    if (!setter)
    {
        bebad();
    }
    else
    {
        setWindowTitle(windowTitle()+'('+QString::number(setter.getGamePID())+')');
        if (ui->autoappradio->isChecked())
            on_applybutton_pressed();
    }
}

FpsDialog::~FpsDialog()
{
    savepreset();
    delete ui;
}

//假设调用的时候不是bad
void FpsDialog::on_applybutton_pressed()
{
    const static QRegularExpression intfpschecker("^[0-9]+$");
    auto fpsstr = ui->fpscombox->currentText();
    if (fpsstr.isEmpty())
        fpsstr = "60";
    else if (!intfpschecker.match(fpsstr).hasMatch())
    {
        ErrorReporter::receive("错误", "输入的帧数需要是正整数");
        return;
    }

    applyFPS(fpsstr.toInt()/*不合法的数值将会返回0*/);
}

void FpsDialog::applyFPS(int fps)
{
    if (!setter)
    {
        bebad();
        return;
    }
    //在这写过测试代码，写了两行loop.exec()但是只quit了一次 排查2小时 又记一笔 --25.10.13
    setter.setFps(fps);

    set2tempread();//即便已经在显示帧率了，也得刷新计时器

    //todo: 尝试使用动画化手感方案？
    ui->applybutton->setEnabled(false);
    QTimer::singleShot(QRandomGenerator::global()->bounded(200, 700), this,
        [&]()
        {
            ui->applybutton->setEnabled(true);
        });
}

void FpsDialog::bebad()
{
    bad = true;

    if (tmpreadtimer->isActive())
        tmpreadtimer->stop();

    keepupdate = false;
    whileKeepUpdateChange();

    //tag和button失效5s
    ui->applybutton->setEnabled(false);
    ui->curframerate->setEnabled(false);

    setWindowTitle(windowTitle() + " (..`)" );

    painlesssuicide = new QTimer(this);
    painlesssuicide->setInterval(5000);
    connect(painlesssuicide, &QTimer::timeout, [this](){emit SetterClosed(this);});
    painlesssuicide->start();

    care2saveI = QObject::connect(ui->fpscombox, &QComboBox::highlighted, this, &FpsDialog::on_fpscombox_touched);
    care2saveE = QObject::connect(ui->fpscombox, &QComboBox::editTextChanged, this, &FpsDialog::on_fpscombox_touched);
}

// 假设调用的时候不是bad
void FpsDialog::on_curframerate_clicked()
{
    if (keepupdate&&tmpreadtimer->isActive())
    {
        tmpreadtimer->stop();
    }else
    {
        keepupdate = !keepupdate;
        whileKeepUpdateChange();
    }
}

void FpsDialog::switch_default()
{
    usedefault = !usedefault;
    if (usedefault)
        applyFPS(60);
    else
        on_applybutton_pressed();
}

void FpsDialog::updateFR()
{
    if (!setter)
    {
        bebad();
        return;
    }
    ui->curframerate->setText(QString::number(setter.getFps(), 'f', 1));
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
        setter.keepAccessible();
        frupdateremider->start();
    }
    else
    {
        setter.autoRelease();
        frupdateremider->stop();
    }
    checkchangePalette();
}

void FpsDialog::set2tempread()
{
    keepupdate = true;
    whileKeepUpdateChange();
    tmpreadtimer->start();
}

void FpsDialog::dissmissFR()
{
    keepupdate = false;
    whileKeepUpdateChange();
}

void FpsDialog::savepreset()const
{
    if(! ui->autoappradio->isChecked())return ;

    Storage<hipp, "hipp"> hipp;
    auto fpsstr = ui->fpscombox->currentText();
    if(hipp)
    {
        int fps = fpsstr.toInt();
        hipp.save<&hipp::fps>(fps);
        bool checked = ui->autoappradio->isChecked();
        hipp.save<&hipp::checked>(checked);
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

bool FpsDialog::loadpreset() {
    //如果存在hipp文件 (有过记录)
    Storage<hipp, "hipp"> hipp;
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
            ui->autoappradio->setChecked(uselast);
        }
        else
        {
            ErrorReporter::receive(ErrorReporter::警告, "无法读写存储");
            return false;
        }
    }
    return true;
}

