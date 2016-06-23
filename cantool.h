#ifndef CANTOOL_H
#define CANTOOL_H

#include "can_sequencer.h"
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QStandardItemModel>

namespace Ui {
class cantool;
}

class cantool : public QMainWindow
{
    Q_OBJECT

public:
    explicit cantool(QWidget *parent = 0);
    ~cantool();
private:
    void table_load_cvs(void);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_reloadcvs_clicked();
    void on_actionOpen_File_triggered();
    void on_checkBox_boucle_stateChanged(int arg1);
    void on_sequence_end(void);
    void on_sequencer_exit(void);
    void on_actionOpen_triggered();

private:
    Ui::cantool *ui;
    can_sequencer * xsequencer;
    QString cvsFileName;
    QLabel *statusLabel;
    QProgressBar *statusProgressBar;
    QStandardItemModel* model;
};

#endif // CANTOOL_H
