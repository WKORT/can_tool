#include "canisotp.h"

struct sockaddr_can addr;

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
canIsoTp::canIsoTp(QObject *parent) : QThread(parent)
{
    setTerminationEnabled(true);

    multiFrameLength = 0;
    constructingFrameLength = 0;
    memset(receivedMultiFrame, 0x00, 4096);
    multiFramePointerIndex = 0;
    multiFrameReception = false;

    multiFrameSending = false;
    flowControleReceived = false;
    transmitMultiFrameComplete = false;
    sendingFrameLength = 0;
    sendingFramePointerIndex = 0;
    sendingConsecutiveFrameSequence = 0;
}


/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::Initialise(const char * can_interface, TU32 ftx_can_id, TU32 frx_can_id)
{
    struct ifreq ifr;

    struct can_frame frame;

    memset(&ifr, 0x0, sizeof(ifr));
    memset(&addr, 0x0, sizeof(addr));
    memset(&frame, 0x0, sizeof(frame));

    this->tx_can_id = ftx_can_id;

    if ((Can_Socket = socket (PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror ("socket");
    }

    strcpy (ifr.ifr_name, can_interface);
    ioctl (Can_Socket, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    addr.can_addr.tp.tx_id = ftx_can_id;
    addr.can_addr.tp.rx_id = frx_can_id;

    //    addr.can_addr.tp.tx_id |= CAN_EFF_FLAG;
    //    addr.can_addr.tp.rx_id |= CAN_EFF_FLAG;

    struct can_filter rfilter[2];

    rfilter[0].can_id   = ftx_can_id;
    rfilter[0].can_mask = ( CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[1].can_id   = frx_can_id;
    rfilter[1].can_mask = ( CAN_RTR_FLAG | CAN_SFF_MASK);

    setsockopt(Can_Socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

    if (bind (Can_Socket, (struct sockaddr *) &addr, sizeof (addr)) < 0)
    {
        perror ("bind");
    }
}


/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::reset(void)
{
    multiFrameLength = 0;
    constructingFrameLength = 0;
    memset(receivedMultiFrame, 0x00, 4096);
    multiFramePointerIndex = 0;
    multiFrameReception = false;

    multiFrameSending = false;
    flowControleReceived = false;
    transmitMultiFrameComplete = false;
    sendingFrameLength = 0;
    sendingFramePointerIndex = 0;
    sendingConsecutiveFrameSequence = 0;
}


/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::run(void)
{
    fd_set fd_rx;
    TS32   max_sd;
    TS32   result;
    TU8    buffer[4095];
    struct timeval timeout;

    while(1)
    {
        FD_ZERO(&fd_rx);
        FD_SET(Can_Socket,&fd_rx);
        max_sd = Can_Socket;

        memset(buffer , 0 , 4095);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        result = 0;
        result = select(max_sd+1,&fd_rx,NULL,NULL,&timeout);
        if(result == 1)
        {
            TS32 s = Can_Socket;
            /* get socket on which there is data */
            if(FD_ISSET(Can_Socket,&fd_rx))
                s = Can_Socket;

            struct sockaddr_can addr;

            socklen_t len = sizeof(addr);
            struct can_frame frame;

            len = recvfrom(s, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, &len);
            if(len>0)
            {
                process_frame(frame.data, (TS32)frame.can_dlc);
            }
            else{}
        }
    }
}


/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
TS32 canIsoTp::sendReceive_canIsoTp_frame(TU8 * bufferTx, TU8 * bufferRx, TU32 len)
{
    TS32 ret;
    return ret;
}




/*********************************************
 *  isotp send / receive API -- engine
**********************************************/
/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_canIsoTp_frame(TU8 * buffer, TU32 len)
{
    if(len > 7)
    {
        /* Mutli Frame */
        multiFrameSending = true;
        /* Get All Frame Length */
        sendingFrameLength = len;
        send_canIsoTp_firstFrame(buffer, len);
        sendingFramePointerIndex += 6;
        /* Wait for FlowControle */
//        qDebug() << "Waiting for FlowControle";
        while(!flowControleReceived);
//        qDebug() << "flowControleReceived";
        TU8 BS=0;
        while(transmitMultiFrameComplete == false)
        {
            sendingConsecutiveFrameSequence++;
            if((sendingFramePointerIndex+7) <= sendingFrameLength)
            {
                BS = 7;
            }
            else if ((sendingFramePointerIndex+7) > sendingFrameLength)
            {
                BS = (sendingFrameLength-sendingFramePointerIndex);
            }
            else {/*error*/}
//            qDebug() << "send consecutiveFrame" << BS;
            send_canIsoTp_consecutiveFrame(buffer+sendingFramePointerIndex, BS);
            sendingFramePointerIndex += BS;

            if(sendingFramePointerIndex == sendingFrameLength)
            {
                /* transmit complete */
//                qDebug() << "transmit complete" << BS;

                sendingFramePointerIndex = 0;
                sendingFrameLength = 0;
                transmitMultiFrameComplete = true;
            }
        }
        flowControleReceived = false;
        multiFrameSending = false;
        transmitMultiFrameComplete = false;
        sendingConsecutiveFrameSequence = 0;
    }
    else
    {
        /* Single Frame */
        send_canIsoTp_singleFrame(buffer, len);
    }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_canIsoTp_singleFrame(TU8 * buffer, TU32 len)
{
    struct can_frame singleFrame;

    if(len <= 7)
    {
        /* N_PCItype + SF_DL */
        singleFrame.data[0] = (len & 0x0F);

        for(int i=1;i<=len;i++)
            singleFrame.data[i] = buffer[i-1];
        for(int i=(len+1);i<8;i++)
            singleFrame.data[i] = 0xFF;

        send_frame(singleFrame.data);
    }
    else
    { /*error*/ }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_canIsoTp_firstFrame(TU8 * buffer, TU32 len)
{
    struct can_frame firstFrame;

    if(len <= 0xFFF)
    {
        /* N_PCItype + FF_DL */
        firstFrame.data[0] = 0x10 + ((len & 0x0F00) >> 8);
        firstFrame.data[1] = (len & 0xFF);

        for(int i=2;i<=7;i++)
            firstFrame.data[i] = buffer[i-2];

        send_frame(firstFrame.data);
    }
    else
    { /*error*/ }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_canIsoTp_consecutiveFrame(TU8 * buffer, TU32 len)
{
    struct can_frame consecutiveFrame;

    if(1)
    {
        /* N_PCItype + SN */
        consecutiveFrame.data[0] = 0x20 + (sendingConsecutiveFrameSequence & 0x0F);

        for(int i=1;i<=len;i++)
            consecutiveFrame.data[i] = buffer[i-1];
        for(int i=(len+1);i<8;i++)
            consecutiveFrame.data[i] = 0xFF;

        send_frame(consecutiveFrame.data);
    }
    else
    { /*error*/ }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_canIsoTp_FlowControl(void)
{
    struct can_frame flowcontrole_frame;
    flowcontrole_frame.data[0] = 0x30;
    flowcontrole_frame.data[1] = 0x00;
    flowcontrole_frame.data[2] = 0x00;
    flowcontrole_frame.data[3] = 0xFF;
    flowcontrole_frame.data[4] = 0xFF;
    flowcontrole_frame.data[5] = 0xFF;
    flowcontrole_frame.data[6] = 0xFF;
    flowcontrole_frame.data[7] = 0xFF;
    send_frame(flowcontrole_frame.data);
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::send_frame(TU8 * buffer)
{
    struct can_frame frame;
    memcpy(frame.data, buffer, 8);
    frame.can_dlc = 8;
    frame.can_id  = tx_can_id;
    //    frame.can_id |= CAN_EFF_FLAG;
    sendto(Can_Socket, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::construct_canIsoTp_frame(TU8 * buffer, TU32 len)
{
    TU32 BS = 0;
    if((constructingFrameLength + len) > multiFrameLength)
    {
        BS = multiFrameLength - constructingFrameLength;
        constructingFrameLength = multiFrameLength;
    }
    else
    {
        constructingFrameLength += len;
        BS = len;
    }


    if (constructingFrameLength <= multiFrameLength)
    {
//        qDebug() << "frame constructing in progress" << constructingFrameLength << BS;
        memcpy(receivedMultiFrame+multiFramePointerIndex, buffer, BS);
        multiFramePointerIndex += BS;
    }
    else{
//        qDebug() << "error";
    }

    if(constructingFrameLength == multiFrameLength)
    {
//        qDebug() << "Push frame in reception stack";
        multiFrameReception = false;
        /* Push frame in reception stack */
        push_new_canIsoTp_frame(receivedMultiFrame, multiFrameLength);
        resetReceiving_canIsoTp_multiFrame();
    }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::resetReceiving_canIsoTp_multiFrame(void)
{
    constructingFrameLength = 0;
    multiFramePointerIndex  = 0;
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::push_new_canIsoTp_frame(TU8 * buffer, TU32 len)
{
    //    if(receptionFrameStack.isEmpty())
    {
        isoTpFrame * XisoTpFrame = new isoTpFrame();
        XisoTpFrame->MessageData = (TU8*)malloc(len);
        memcpy(XisoTpFrame->MessageData, buffer, len);
        XisoTpFrame->Length = len;

        receptionFrameStack.append(XisoTpFrame);
    }
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
TS32 canIsoTp::recieve_canIsoTp_frame(TU8 * buffer, TU32 *len, TU32 mstimeout)
{
    TS32 ret;
    TS32 timeout;
    timeout = (TS32) mstimeout;
    while(receptionFrameStack.isEmpty() && (timeout>0))
    {
        QThread::msleep(50);
        timeout -= 50;
    }

    if(!receptionFrameStack.isEmpty())
    {
        ret = receptionFrameStack.size();
        isoTpFrame * XisoTpFrame;
        XisoTpFrame = receptionFrameStack.first();
        memcpy(buffer, XisoTpFrame->MessageData, XisoTpFrame->Length);
        *len = (TS32)XisoTpFrame->Length;

        free(XisoTpFrame->MessageData);
        receptionFrameStack.removeFirst();
    }
    else
    {
        ret = -1;
    }
    return ret;
}












/*********************************************
 * small isotp state machine -- engine
**********************************************/



/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::process_frame(TU8 * buffer, TU32 len)
{
    TU8 FrameType = 0;
    FrameType = (buffer[0] >> 4) & 0x0F;
    //    qDebug() << "FrameType:" << FrameType << endl;
    if(FrameType == SI_FRAME)
    {
//        qDebug() << "single frame" << endl;
        /* Push frame in reception stack */
        push_new_canIsoTp_frame(buffer+1, buffer[0]);
    }
    else if(FrameType == FI_FRAME)
    {
//        qDebug() << "First frame" << endl;
        //qDebug() << "Begin construct frame" << endl;
        //qDebug() << "MultiFrame length:" << buffer[1] <<  endl;
        multiFrameReception = true;
        multiFrameLength = (TU32) ((buffer[0]&0x0F)<<8) + buffer[1];
        construct_canIsoTp_frame(buffer+2, 6);
        send_canIsoTp_FlowControl();
//        qDebug() << "multiFrameLength" << multiFrameLength;
    }
    else if(FrameType == CO_FRAME)
    {
        if(multiFrameReception)
        {
//            qDebug() << "Consecutive frame" << endl;
            construct_canIsoTp_frame(buffer+1, 7);
        }else{/*error*/}
    }
    else if(FrameType == FC_FRAME) /* FlowControl */
    {
        if(multiFrameSending)
        {
            transmitMultiFrameComplete = false;
            flowControleReceived = true;
        }
//        else{qDebug() << "error";}
    }
}




/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
void canIsoTp::print_buffer(TU8 * buffer, TU32 len)
{
    for(int i=0;i<len;i++)
        fprintf(stderr,"%02X:",buffer[i]);
    fprintf(stderr,"\n");
}

/****************************************************************************
* DESCRIPTION:
* NOTE:
***************************************************************************/
canIsoTp::~canIsoTp()
{

}

