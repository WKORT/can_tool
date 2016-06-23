#ifndef CAN_SEQUENCER_H
#define CAN_SEQUENCER_H

#include "canisotp.h"
#include <QThread>
#include <QTableWidget>
#include <QTime>
#include <QFile>
#include <QStandardItemModel>

/*
*  Parameters Data identifiers (PARAM DID)
*/
#define udsID_BUB_STA            0x9000    //R      //Battery state
#define udsID_TIME               0x9010    //R/W    //Real Time Clock
#define udsID_GSM_FW_IDENT       0x9020    //R      //GSM fw release
#define udsID_GSM_HW_IDENT       0x9021    //R      //GSM hw release
#define udsID_IMSI_1             0x9022    //R      //IMSI number SIM1
#define udsID_IMSI_2             0x9023    //R      //IMSI number SIM2
#define udsID_IMEI               0x9024    //R      //IMEI number
#define udsID_PCPU_BOOT_IDENT    0xA000    //R      //PCPU bootloader identification
#define udsID_PCPU_APP_IDENT     0xA001    //R      //PCPU application identification
#define udsID_CCPU_BOOT_IDENT    0xA002    //R      //CCPU bootloader identification
#define udsID_CCPU_APP_IDENT     0xA003    //R      //CCPU application identification
#define udsID_CCPU_CALIB_IDENT   0xA004    //R      //CCPU calibration identification
#define udsID_ETH_MAC_ADDR       0x9050    //R/W    //Ethernet MAC address
#define udsID_WIFI_MAC_ADDR      0x9051    //R      //Wifi MAC address

#define udsID_DIN1 		 	 0x100 //Digital Input
#define udsID_DIN2 	     	 0x101 //Digital Input
#define udsID_DIN3 	 	 	 0x102 //Digital Input
#define udsID_DIN3_PCPU 	 0x103 //Digital Input
#define udsID_DOUT1 		 0x400 //High side PWM output
#define udsID_DOUT2 		 0x401 //High side PWM output
#define udsID_DOUT3 		 0x402 //High side PWM output
#define udsID_DOUT4 		 0x500 //Low side digital output
#define udsID_MICP 		 	 0x600 //Audio
#define udsID_MICN 		 	 0x601 //Audio
#define udsID_SPEAKER 	 	 0x602 //Audio
#define udsID_SIMSEL 		 0x603 //SIM Selection
#define udsID_NAD_ANT_STA  	 0x604 //NAD Antenna Status
#define udsID_GNSS_ANT_ON  	 0x605 //GNSS Antenna activation
#define udsID_GNSS_ANT_STA 	 0x606 //GNSS Antenna Status
#define udsID_STATUSLED 	 0x700 //PCB LED output
#define udsID_SWAKEUP 	 	 0x701 //Secondary Wake UP
#define udsID_15WAKEUP 	 	 0x702 //Wakeup input
#define udsID_ALARMWAKEUP 	 0x703 //Wakeup input
#define udsID_SWAKEUP_PCPU 	 	 0x704 //Secondary Wake UP
#define udsID_15WAKEUP_PCPU 	 0x705 //Wakeup input
#define udsID_ALARMWAKEUP_PCPU 	 0x706 //Wakeup input
#define udsID_PCPUTEMP 		 	 0x800 //Internal temperature
#define udsID_CCPUTEMP 	 		 0x801 //Internal temperature
#define udsID_HCPUTEMP 	 	 	 0x802 //Internal temperature
#define udsID_2V5 			0x803 //Internal voltage
#define udsID_4V 			0x804 //Internal voltage
#define udsID_5V 			0x805 //Internal voltage
#define udsID_8V 			0x806 //Internal voltage
#define udsID_12V 			0x807 //Internal voltage
#define udsID_VBAT 			0x808 //Internal voltage
#define udsID_VBAT_PROT_SW 	0x809 //Internal voltage
#define udsID_I_BUB_ADC 	0x80A //Internal voltage
#define udsID_V_BUB 		0x80B //Internal voltage
#define udsID_V_CHR 		0x80C //Internal voltage
#define udsID_IMX_3V3 		0x80D //Internal voltage
#define udsID_COMP_3V3 		0x80E //Internal voltage
#define udsID_IMX_1V2 		0x80F //Internal voltage
#define udsID_IMX_1V35 		0x810 //Internal voltage
#define udsID_IMX_VDDHIGH 	0x811// Internal voltage

#define udsID_NOT_SUPPORTED1 	0x777// Internal voltage
#define udsID_NOT_SUPPORTED2 	0x555// Internal voltage
#define udsID_NOT_SUPPORTED3	0x444// Internal voltage

#define udsID_FUNCT_DISTRO_IDENT 0xA005    //R      //Functional Distribution firmware identification format

/* Routine Control ID */
#define GSM_POWER_MODE          0xF000
#define GSM_FACTORY_MODE        0xF001
#define GSM_LOOPBACK_CONTROL    0xF002
#define GSM_TONE_CTRLE          0xF003
#define GSM_CT_WAVE_OUTPUT_TEST 0xF004
#define GSM_CT_WAVE_INPUT_TEST  0xF005
#define GSM_CHECK_RING_LINE     0xF006
#define ETHERNET_SELFT_TEST     0xF020
#define CAN_SELF_TEST           0xF030
#define GYRO_SENSOR_SELF_TEST   0xF040
#define GNSS_SELF_TEST          0xF041
#define ONEWIRE_SELF_TEST       0xF060

#define WIFI_POWER_MODE         0xF070
#define WIFI_CONTINUOUS_TX_MODE 0xF071
#define WIFI_CONTINUOUS_RX_MODE 0xF072

class can_sequencer : public QThread
{
    Q_OBJECT
public:
    explicit can_sequencer(QObject *parent = 0);
    void tableView(QTableView *TableView);
    void set_model(QStandardItemModel * model);
    char * process_result(TU8 *buffer, char *response);
    bool setRepeatable(bool repeat);
    void run(void);
    ~can_sequencer();

signals:
    void sequence_end(void);
    void sequencer_exit(void);

private:
    char * convertStr2Hex(QString str);

private:
    bool is_repeatable;
    canIsoTp CanIsoTp;
    QTableView * XTableView;
    QStandardItemModel * Xmodel;

    QString logfilename;
    QFile *file;
    QTextStream *stream;

public slots:
};

#endif // CAN_SEQUENCER_H
