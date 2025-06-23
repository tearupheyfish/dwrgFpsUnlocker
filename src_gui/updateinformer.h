#ifndef UPDATEINFORMER_H
#define UPDATEINFORMER_H

#include <QDialog>
#include <QProgressBar>

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

private slots:
    void on_updatebutton_pressed(); // 更新进度条进度
    void update_progress(qint64 bytesReceived, qint64 bytesTotal);

    friend class UpdateChecker;
};

#endif // UPDATEINFORMER_H
