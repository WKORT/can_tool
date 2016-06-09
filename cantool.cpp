#include "cantool.h"
#include "ui_cantool.h"
#include <QListWidgetItem>
#include <QTableWidget>
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

    QStringList ColumHeader;
    ColumHeader << "Time" << "RequestName" << "Len" << "Request" << "Status" << "Description";

    ui->tableWidget->setColumnCount(ColumHeader.size());
    ui->tableWidget->setHorizontalHeaderLabels(ColumHeader);

    ui->checkBox_boucle->setText("Loop in");
    ui->checkBox_boucle->setCheckState(Qt::Unchecked);

    cvsFileName = "dbc.cvs";
    table_load_cvs();

    xsequencer = new can_sequencer();

    QObject::connect(xsequencer, SIGNAL(sequence_end()),
                     this,         SLOT(on_sequence_end()));

    QObject::connect(xsequencer, SIGNAL(sequencer_exit()),
                     this,         SLOT(on_sequencer_exit()));
}

cantool::~cantool()
{
    delete ui;
}


void cantool::table_load_cvs(void)
{
    ui->tableWidget->clearContents();

    ui->tableWidget->setUpdatesEnabled(true);
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(ui->tableWidget->verticalHeader()->minimumSectionSize());
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setAlternatingRowColors(true);

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

            if(QString::compare(">>",line[0], Qt::CaseInsensitive)==0)
            {
                ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
                /* is frame to send ">>" */
//                qDebug() << line[1] << line[2] ;

                QTableWidgetItem *tabWidItemName;
                tabWidItemName = new QTableWidgetItem();
                tabWidItemName->setText(line[1]);
                tabWidItemName->setCheckState(Qt::Checked);

                QTableWidgetItem *tabWidItemFrame;
                tabWidItemFrame = new QTableWidgetItem();
                tabWidItemFrame->setFont(QFont("mono"));
                tabWidItemFrame->setText(line[2]);

                QTableWidgetItem *tabWidItemResponseLength;
                tabWidItemResponseLength = new QTableWidgetItem();
                tabWidItemResponseLength->setFont(QFont("mono"));
                tabWidItemResponseLength->setText("");

                QTableWidgetItem *tabWidItemResponse;
                tabWidItemResponse = new QTableWidgetItem();
                tabWidItemResponse->setFont(QFont("mono"));
                tabWidItemResponse->setText("");

                ui->tableWidget->setItem(rowIndex,1,tabWidItemName);
                ui->tableWidget->setItem(rowIndex,3,tabWidItemFrame);
                ui->tableWidget->setItem(rowIndex,2,tabWidItemResponseLength);
                ui->tableWidget->setItem(rowIndex,4,tabWidItemResponse);
                ui->tableWidget->resizeColumnsToContents();

//                QTableWidgetItem *VerticalHeaderItem;
//                VerticalHeaderItem = new QTableWidgetItem();
//                VerticalHeaderItem->setFont(QFont("Arial"));
//                VerticalHeaderItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsTristate|Qt::ItemIsUserCheckable);
//                VerticalHeaderItem->setCheckState(Qt::Checked);
//                ui->tableWidget->setVerticalHeaderItem(rowIndex, VerticalHeaderItem);
//                ui->tableWidget->verticalHeader()->setStyle(QApplication::style());

                rowIndex+=2;
                ui->tableWidget->setRowCount(rowIndex);
                ui->tableWidget->hideRow(rowIndex-1);
            }
            else if(QString::compare("d",line[0], Qt::CaseInsensitive)==0)
            {
                ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
                QTableWidgetItem *tabItemDirective;
                tabItemDirective = new QTableWidgetItem();
                tabItemDirective->setText(line[1]);
                tabItemDirective->setCheckState(Qt::Checked);
                tabItemDirective->setFont(QFont("Arial"));
                tabItemDirective->setTextColor(QColor("blue"));

                QTableWidgetItem *tabWidItemValue;
                tabWidItemValue = new QTableWidgetItem();
                tabWidItemValue->setText(line[2]);
                tabWidItemValue->setFont(QFont("Arial"));
                tabWidItemValue->setTextColor(QColor("blue"));

                ui->tableWidget->setItem(rowIndex,1,tabItemDirective);
                ui->tableWidget->setItem(rowIndex,3,tabWidItemValue);
                rowIndex+=2;
                ui->tableWidget->setRowCount(rowIndex);
                ui->tableWidget->hideRow(rowIndex-1);
            }
            else if(QString::compare("c",line[0], Qt::CaseInsensitive)==0)/* comment */
            {
                ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
                QTableWidgetItem *tabItemComment;
                tabItemComment = new QTableWidgetItem();
                tabItemComment->setText(line[1]);
                tabItemComment->setFont(QFont("Courier New"));
                tabItemComment->setTextColor(QColor(105,115,8));
                tabItemComment->setBackgroundColor(QColor(230,240,132));

                ui->tableWidget->setItem(rowIndex,1,tabItemComment);
                rowIndex+=2;
                ui->tableWidget->setRowCount(rowIndex);
                ui->tableWidget->hideRow(rowIndex-1);
            }
            else
            {
            }
        }
        f.close();
    }
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setColumnWidth(0,57); // Time column
    ui->tableWidget->setColumnWidth(3,180); // Frame column
    ui->tableWidget->setColumnWidth(4,180); // Status column
}

void cantool::on_pushButton_clicked()
{
    ui->pushButton->setEnabled(false);
    ui->pushButton_reloadcvs->setEnabled(false);
    xsequencer->setTableWidget(ui->tableWidget);
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
