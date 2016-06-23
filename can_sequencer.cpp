#include "can_sequencer.h"
#include <QDebug>
#include <QFile>
#include <QStandardItemModel>

#define TX_CAN_ID_C300 0x700
#define RX_CAN_ID_C300 0x701

can_sequencer::can_sequencer(QObject *parent) : QThread(parent)
{
    CanIsoTp.Initialise("can0",TX_CAN_ID_C300,RX_CAN_ID_C300);
    CanIsoTp.start(QThread::NormalPriority);
    logfilename = "log.csv";
    file= new QFile(logfilename);
    if ( file->open(QIODevice::ReadWrite) )
    {
        stream = new QTextStream(file);
        *stream << "Log Start=" << QTime::currentTime().toString() << endl;
    }
    is_repeatable = false;
}

void can_sequencer::tableView(QTableView * TableView)
{
    XTableView = TableView;
}
void can_sequencer::set_model(QStandardItemModel * model)
{
    Xmodel = model;
}

can_sequencer::~can_sequencer()
{

}

char * can_sequencer::convertStr2Hex(QString str)
{
    int size, i, index;
    bool ok;
    char * hexArray;
    index = 0;
    size = str.simplified().length();
    hexArray = (char *) malloc(size);
    memset(hexArray, 0, size);
    QString x ="";
    for(i=0;i<size;i+=2)
    {
        x.clear();
        x.append(str.at(i));
        x.append(str.at(i+1));
        hexArray[index]= (char) x.toUInt(&ok,16);
        index++;
    }
    return hexArray;

}

void can_sequencer::run(void)
{
    int   i,j;
    int   frameLength;
    char  * frame;
    TU8   buffer[4095];
    char  response[4095];
    char  description[100];
    int   length;
    ulong sleepTime;
    bool  receiving_again;
    QTableWidgetItem *tabWidItem;
    QTableWidgetItem *tabItemName;
    bool one_shot = true;

    qDebug() << Xmodel->rowCount();
//    file->resize(0);
    while((is_repeatable) || (one_shot))
    {
        for(i=0;i< Xmodel->rowCount();i+=2)
        {
            if(QString::compare(Xmodel->item(i,1)->text(),"sleep")==0)
            {
                qDebug() << "directive" << Xmodel->item(i,1)->text();
//                sleepTime = (ulong) Xmodel->item(i,2)->text().toULong();
//                Xmodel->item(i,3)->setText(QString("%1").arg(sleepTime));
//                while(sleepTime > 0)
//                {
//                    QThread::sleep(1);
//                    sleepTime--;

//                    Xmodel->item(i,3)->setText(QString("%1").arg(sleepTime));
////                    XTableView->viewport()->update();
//                }
            }
            else if(QString::compare(Xmodel->item(i,1)->text(),"reboot")==0)
            {
                 qDebug() << "directive" << Xmodel->item(i,1)->text();
            }
            else
            {

                if(Xmodel->item(i,3)==0)
                {
                    qDebug() << "comment" << Xmodel->item(i,3);
                }
                else
                {

                    qDebug() << "frame" << Xmodel->item(i,3)->text();

                    QString req_name        = Xmodel->item(i,1)->text();
                    //QString req_len         = Xmodel->item(i,2)->text();
                    QString req_frame       = Xmodel->item(i,3)->text();


                    frameLength = req_frame.simplified().length()/2;
                    frame = convertStr2Hex(req_frame.simplified());

                    Xmodel->setData(Xmodel->index(i,2),frameLength,Qt::DisplayRole);

                    *stream <<"<<<,["<< frameLength <<"]," << tabWidItem->text().simplified() << endl;
                    CanIsoTp.send_canIsoTp_frame((TU8*)frame, (TU32)frameLength);
                    receiving_again = false;
                    while(receiving_again == false)
                    {
                        length = 0;
                        memset(buffer,   0, 4095);
                        memset(response, 0, 4095);
                        if(CanIsoTp.recieve_canIsoTp_frame(buffer, (TU32 *)&length, 1000) > 0)
                        {
                            for(j=0;j<length;j++)
                            {
                                sprintf(response+(j*2), "%02X", buffer[j]);
                            }
                            fprintf(stdout,"%s \n",response);
                            *stream <<">>>,["<< length <<"]," << response << endl;

                            //                        XTableView->showRow(i+1);

//                            QTableWidgetItem *tabWidItemValue;
//                            tabWidItemValue = new QTableWidgetItem();
//                            tabWidItemValue->setText(QString(response));
//                            tabWidItemValue->setFont(QFont("mono"));
//                            XTableView->setItem(i+1,3,tabWidItemValue);

                            if(buffer[0]!=0x7F)
                            {
//                                XTableView->item(i+1,3)->setBackgroundColor(QColor(0x99,0xFF,0x99));
//                                Xmodel->setData(Xmodel->index(i+1,3),"",Qt::DisplayRole);

//                                Xmodel->setData(Xmodel->index(i+1,4),"OK",Qt::DisplayRole);

//                                process_result(buffer, description);
//                                Xmodel->setData(Xmodel->index(i+1,5),"description",Qt::DisplayRole);

                                receiving_again=true;
                            }
                            else
                            {   if(buffer[2]!=0x78){
                                    receiving_again=true;
                                }
//                                if(buffer[2]==0x31){
//                                    XTableView->setItem(i+1,4, new QTableWidgetItem("OutOfRange"));
//                                    XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                }else if(buffer[2]==0x22){
//                                    XTableView->setItem(i+1,4, new QTableWidgetItem("CondNotCorrect"));
//                                    XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                }else if(buffer[2]==0x13){
//                                    XTableView->setItem(i+1,4, new QTableWidgetItem("BadFormat"));
//                                    XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                }else if(buffer[2]==0x78){
//                                    XTableView->setItem(i+1,4, new QTableWidgetItem("Waiting..."));
//                                    XTableView->item(i+1,3)->setBackgroundColor(QColor("orange"));
//                                }
                            }
//                            XTableView->setItem(i+1,2, new QTableWidgetItem(QString("%1").arg(length)));
//                            XTableView->setItem(i+1,0, new QTableWidgetItem(QTime::currentTime().toString()));
//                            XTableView->scrollToItem(XTableView->item(i+1,0), QAbstractItemView::EnsureVisible);

                            /* Refrech table */
//                            XTableView->viewport()->update();
                        }
                        else
                        {
                        }
                    }
                    fprintf(stdout,"\n");
                    free(frame);
                }

            }




////            tabItemName = XTableView->item(i,1);
//            if(tabWidItem != NULL)
//            {
//                if(tabItemName->checkState() == Qt::Checked)
//                {
//                    XTableView->setItem(i,0, new QTableWidgetItem(QTime::currentTime().toString()));
//                    //                XTableView->item(i,2)->setBackgroundColor(QColor(0xdfffaf));
//                    if(QString::compare(tabItemName->text(),"sleep")==0)
//                    {
//                        sleepTime = (ulong) tabWidItem->text().toULong();
//                        XTableView->setItem(i,4, new QTableWidgetItem(QString("%1").arg(sleepTime)));
//                        while(sleepTime > 0)
//                        {
//                           QThread::sleep(1);
//                           sleepTime--;

//                           XTableView->item(i,4)->setText(QString("%1").arg(sleepTime));
//                           XTableView->viewport()->update();
//                        }
//                    }
//                    else
//                    {
//                        frameLength = tabWidItem->text().simplified().length()/2;
//                        frame = convertStr2Hex(tabWidItem->text().simplified());
//                        XTableView->item(i,2)->setText(QString("%1").arg(frameLength));

//                        *stream <<"<<<,["<< frameLength <<"]," << tabWidItem->text().simplified() << endl;
//                        CanIsoTp.send_canIsoTp_frame((TU8*)frame, (TU32)frameLength);
//                        receiving_again = false;
//                        while(receiving_again == false)
//                        {
//                            length = 0;
//                            memset(buffer,   0, 4095);
//                            memset(response, 0, 4095);
//                            if(CanIsoTp.recieve_canIsoTp_frame(buffer, (TU32 *)&length, 1000) > 0)
//                            {
//                                for(j=0;j<length;j++)
//                                {
//                                    sprintf(response+(j*2), "%02X", buffer[j]);
//                                }
//                                fprintf(stdout,"%s \n",response);
//                                *stream <<">>>,["<< length <<"]," << response << endl;

//                                XTableView->showRow(i+1);

//                                QTableWidgetItem *tabWidItemValue;
//                                tabWidItemValue = new QTableWidgetItem();
//                                tabWidItemValue->setText(QString(response));
//                                tabWidItemValue->setFont(QFont("mono"));
//                                XTableView->setItem(i+1,3,tabWidItemValue);

//                                if(buffer[0]!=0x7F)
//                                {
//                                    XTableView->item(i+1,3)->setBackgroundColor(QColor(0x99,0xFF,0x99));
//                                    XTableView->setItem(i+1,4, new QTableWidgetItem("OK"));
//                                    process_result(buffer, description);
//                                    XTableView->setItem(i+1,5, new QTableWidgetItem(description));
//                                    receiving_again=true;
//                                }
//                                else
//                                {   if(buffer[2]!=0x78){
//                                        receiving_again=true;
//                                    }
//                                    if(buffer[2]==0x31){
//                                        XTableView->setItem(i+1,4, new QTableWidgetItem("OutOfRange"));
//                                        XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                    }else if(buffer[2]==0x22){
//                                        XTableView->setItem(i+1,4, new QTableWidgetItem("CondNotCorrect"));
//                                        XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                    }else if(buffer[2]==0x13){
//                                        XTableView->setItem(i+1,4, new QTableWidgetItem("BadFormat"));
//                                        XTableView->item(i+1,3)->setBackgroundColor(QColor(0xFF,0x99,0x99));
//                                    }else if(buffer[2]==0x78){
//                                        XTableView->setItem(i+1,4, new QTableWidgetItem("Waiting..."));
//                                        XTableView->item(i+1,3)->setBackgroundColor(QColor("orange"));
//                                    }
//                                }
//                                XTableView->setItem(i+1,2, new QTableWidgetItem(QString("%1").arg(length)));
//                                XTableView->setItem(i+1,0, new QTableWidgetItem(QTime::currentTime().toString()));
//                                XTableView->scrollToItem(XTableView->item(i+1,0), QAbstractItemView::EnsureVisible);
//                                /* Refrech table */
//                                XTableView->viewport()->update();
//                            }
//                            else
//                            {
//                            }
//                        }
//                        fprintf(stdout,"\n");
//                        free(frame);
//                    }
//                }
//            }
        }
        CanIsoTp.reset();
        one_shot = false;
        sequence_end();
        QThread::sleep(1); // wait relaoding file
    }
    stream->flush();
    sequencer_exit();
}

bool can_sequencer::setRepeatable(bool repeat)
{
    is_repeatable = repeat;
}

char * can_sequencer::process_result(TU8 * buffer, char * response)
{
    TU8  service   = 0;
    TU16 serviceId = 0;
    char result[100];

    memset(result, 0 ,100);
    service = buffer[0]-0x40;
    printf(" -- ");

    if(service == 0x22){ /* RDBI */
        serviceId = ((TU16)buffer[1] << 8) + (TU16)buffer[2];
        if(serviceId == 0x9024){
            sprintf(result,"IMEI: %s",&buffer[3]);
        }
        else if(serviceId == 0x9023){
            sprintf(result,"IMSI: %s",&buffer[3]);
        }
        else if(serviceId == 0x9020){
            sprintf(result,"SW_ID: %s",&buffer[3]);
        }
        else if(serviceId == 0x9021){
            sprintf(result,"HW_ID: %s",&buffer[3]);
        }
        else if((serviceId == udsID_PCPU_BOOT_IDENT)
                || 	 (serviceId == udsID_PCPU_APP_IDENT)
                || 	 (serviceId == udsID_CCPU_BOOT_IDENT)
                || 	 (serviceId == udsID_CCPU_APP_IDENT)
                || 	 (serviceId == udsID_CCPU_CALIB_IDENT)
                ){
            sprintf(result,"_IDENT: %s",&buffer[8]);
        }
        else if(serviceId == udsID_BUB_STA){
            sprintf(result,"BUB_STA: %d",buffer[3]);
        }
        else if(serviceId == udsID_TIME){
            sprintf(result,"TIME: %02d:%02d:%02d %02d/%02d/%04d ",buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],(((TU16)buffer[8] << 8) + (TU16)buffer[9]));
        }
        else if(serviceId == udsID_ETH_MAC_ADDR){
            sprintf(result," <= ETH_MAC_ADDR");
        }
        else if(serviceId == udsID_WIFI_MAC_ADDR){
            sprintf(result," <= WIFI_MAC_ADDR");
        }
    }
    else if(service == 0x31){ /* RC */
        if(buffer[1] == 0x03){
            serviceId = ((TU16)buffer[2] << 8) + (TU16)buffer[3];
            if(serviceId == 0xF005){
                if (buffer[4] != 0x03) sprintf(result,"level = %d",(TS16)(((TU16)buffer[5] << 8) + (TU16)buffer[6])); else sprintf(result,"NOK");
            }
            else if((serviceId == 0xF020)||(serviceId == 0xF040)||(serviceId == 0xF041)||(serviceId == 0xF000)||(serviceId == 0xF001)||(serviceId == 0xF002)||(serviceId == 0xF060l)||(serviceId == 0xF070)){
                if(buffer[4]==0x01) sprintf(result,"[ O K ]"); else if(buffer[4]==0x02) sprintf(result,"OPERATION IN PROGRESS"); else sprintf(result,"[ N O K ]");
            }
            else if (serviceId == 0xF030){
                sprintf(result,"ring line  ");
                if(buffer[4]==0x01) sprintf(result,"[ O K ]"); else if(buffer[4]==0x02) sprintf(result,"IN PROGRESS"); else sprintf(result,"[ N O K ]");
            }
            else if (serviceId == 0xF006){
                sprintf(result,"ring line  ");
                if(buffer[4]==0x01) sprintf(result,"[ O K ]"); else if(buffer[4]==0x02) sprintf(result,"IN PROGRESS"); else sprintf(result,"[ N O K ]");
            }
        }
        else if(buffer[1] == 0x02){
            serviceId = ((TU16)buffer[2] << 8) + (TU16)buffer[3];
            if(serviceId == 0xF004){
                sprintf(result,"[ T X ]");
            }
            else if(serviceId == 0xF004){
                sprintf(result,"[ S T O P ]-[ T X ]");
            }
        }
    }
    memcpy(response, result ,100);

    return result;
}
