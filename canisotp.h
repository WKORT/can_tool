#ifndef CANISOTP_H
#define CANISOTP_H

#include <QList>
#include "std_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <QDebug>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <QThread>


#define SI_FRAME 0
#define FI_FRAME 1
#define CO_FRAME 2
#define FC_FRAME 3

class canIsoTp : public QThread
{
    Q_OBJECT
public:
    explicit canIsoTp(QObject *parent = 0);
    ~canIsoTp();
    void Initialise(const char *can_interface, TU32 ftx_can_id, TU32 frx_can_id);
    void reset(void);
    void run(void);
    void send_frame(TU8 * buffer);
    void send_canIsoTp_frame(TU8 *buffer, TU32 len);
    void send_canIsoTp_singleFrame(TU8 * buffer, TU32 len);
    void send_canIsoTp_firstFrame(TU8 * buffer, TU32 len);
    void send_canIsoTp_consecutiveFrame(TU8 * buffer, TU32 len);
    TS32 sendReceive_canIsoTp_frame(TU8 * bufferTx, TU8 * bufferRx, TU32 len);
    void process_frame(TU8 * buffer, TU32 len);
    void print_buffer(TU8 * buffer, TU32 len);
    void send_canIsoTp_FlowControl(void);
    void construct_canIsoTp_frame(TU8 * buffer, TU32 len);
    void resetReceiving_canIsoTp_multiFrame(void);
    void push_new_canIsoTp_frame(TU8 * buffer, TU32 len);
    TS32 recieve_canIsoTp_frame(TU8 *buffer, TU32 *len, TU32 mstimeout);

private:
    TU32 rx_can_id;
    TU32 tx_can_id;
    int  Can_Socket;
    TU32 multiFrameLength;
    TU32 constructingFrameLength;
    TU8  receivedMultiFrame[4096];
    TU8  multiFramePointerIndex;
    bool multiFrameReception;

    typedef struct isoTpFrameType{
        TU32 N_SA;
        TU32 N_TA;
        TU8 * MessageData;
        TU32 Length;
    }isoTpFrame;

    QList<isoTpFrame*> receptionFrameStack;

    volatile bool multiFrameSending;
    volatile bool flowControleReceived;
    volatile bool transmitMultiFrameComplete;
    volatile TU32 sendingFrameLength;
    volatile TU8  sendingFramePointerIndex;
    volatile TU8  sendingConsecutiveFrameSequence;

signals:

public slots:
};

#endif // CANISOTP_H
