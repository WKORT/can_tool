#include "cantool.h"
#include "ui_cantool.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QProgressBar>

cantool::cantool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::cantool)
{
    ui->setupUi(this);

    const int numRows = 1;
    const int numColumns = 1;


    model = new QStandardItemModel(numRows, numColumns);

    QStringList ColumHeader;
    ColumHeader << "Time" << "RequestName" << "Len" << "Request" << "Status" << "Description";
    model->setHorizontalHeaderLabels(ColumHeader);
    model->setRowCount(2);
    ui->tableView->setModel(model);
    ui->tableView->resizeRowsToContents();

    ui->checkBox_boucle->setText("Loop in");
    ui->checkBox_boucle->setCheckState(Qt::Unchecked);

    cvsFileName = "dbc.cvs";
    table_load_cvs();

    ui->tableView->resizeColumnToContents(1);
    ui->tableView->resizeColumnToContents(2);
    ui->tableView->resizeColumnToContents(4);
    ui->tableView->resizeColumnToContents(5);

    ui->tableView->setColumnWidth(3,250);

    ui->tableView->setAutoFillBackground(true);

    xsequencer = new can_sequencer();

//    QObject::connect(xsequencer, SIGNAL(sequence_end()),
//                     this,         SLOT(on_sequence_end()));

//    QObject::connect(xsequencer, SIGNAL(sequencer_exit()),
//                     this,         SLOT(on_sequencer_exit()));
}

cantool::~cantool()
{
    delete ui;
}


void cantool::table_load_cvs(void)
{
//    ui->tableWidget->clearContents();
//    model->clear();

//    ui->tableWidget->setUpdatesEnabled(true);
//    ui->tableWidget->setRowCount(0);

//    ui->tableWidget->verticalHeader()->setDefaultSectionSize(ui->tableWidget->verticalHeader()->minimumSectionSize());
//    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
//    ui->tableWidget->setAlternatingRowColors(true);

    QPalette palette;
    if(system("ifconfig can0 | grep UP")==0)
    {
       palette.setColor(QPalette::Background, QColor(170,200,170));
       ui->statusBar->showMessage("CONNECTED");
    }
    else
    {
        palette.setColor(QPalette::Background,Qt::red);
        ui->statusBar->showMessage("DISCONNECTED");
    }
    ui->statusBar->setPalette(palette);
    ui->statusBar->setAutoFillBackground(true);

    QList<QStandardItem*> *row_items;
    row_items = new QList<QStandardItem*>();

    QFile f(cvsFileName);
    if(f.open(QIODevice::ReadWrite))
    {
        //file opened successfully
        QString data;
        QStringList line;
        int rowIndex = 0;
        while(!f.atEnd())
        {
            data = f.readLine();
            line = data.split(',');

            row_items->clear();
            QStandardItem * time_stamp      = new QStandardItem("");
            row_items->append(time_stamp);

            if(QString::compare(">>",line[0], Qt::CaseInsensitive)==0)
            {
                /* is frame to send ">>" */
//                qDebug() << line[1] << line[2] ;

//                QStandardItem * item_time_stamp = new QStandardItem(QTime::currentTime().toString());

                QStandardItem * req_name        = new QStandardItem(line[1]);
                QStandardItem * req_len         = new QStandardItem("");
                QStandardItem * req_frame       = new QStandardItem(line[2]);
                QStandardItem * response_length = new QStandardItem("");
                QStandardItem * res_frame       = new QStandardItem("");

                row_items->append(req_name);
                row_items->append(req_len);
                row_items->append(req_frame);
                row_items->append(response_length);
                row_items->append(res_frame);
            }
            else if(QString::compare("d",line[0], Qt::CaseInsensitive)==0)
            {
                QStandardItem * directive = new QStandardItem(line[1]);
                QStandardItem * value = new QStandardItem(line[2]);

                row_items->append(directive);
                row_items->append(value);
            }
            else if(QString::compare("c",line[0], Qt::CaseInsensitive)==0)/* comment */
            {
                QStandardItem * comment = new QStandardItem(line[1]);

                row_items->append(comment);
            }
            else{continue;}

            model->insertRow(rowIndex,*row_items);
            ui->tableView->setRowHidden(rowIndex+1,true);
            rowIndex+=2;
            model->setRowCount(rowIndex+2);
        }
        f.close();
        model->setRowCount(rowIndex);
    }
}

void cantool::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->pushButton_reloadcvs->setEnabled(false);
    xsequencer->tableView(ui->tableView);
    xsequencer->set_model(model);
    xsequencer->setRepeatable(ui->checkBox_boucle->isChecked());
    xsequencer->start(QThread::NormalPriority);
}

void cantool::on_pushButton_reloadcvs_clicked()
{
    table_load_cvs();
}

void cantool::on_actionOpen_File_triggered()
{
   cvsFileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Files (*.cvs)"));
   table_load_cvs();
}

void cantool::on_checkBox_boucle_stateChanged(int arg1)
{
    xsequencer->setRepeatable(ui->checkBox_boucle->isChecked());
}

void cantool::on_sequence_end()
{
    if(ui->checkBox_boucle->isChecked())
        table_load_cvs();
}

void cantool::on_sequencer_exit()
{
    ui->pushButton->setEnabled(true);
    ui->pushButton_reloadcvs->setEnabled(true);
}

void cantool::on_actionOpen_triggered()
{
    cvsFileName = QFileDialog::getOpenFileName(this, tr("Open File"),"",tr("Files (*.cvs)"));
    table_load_cvs();
}
