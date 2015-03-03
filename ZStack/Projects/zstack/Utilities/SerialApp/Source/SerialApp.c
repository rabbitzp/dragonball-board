/**************************************************************************************************
  Filename:       SerialApp.c
  Revised:        $Date: 2009-03-29 10:51:47 -0700 (Sun, 29 Mar 2009) $
  Revision:       $Revision: 19585 $

  Description -   Serial Transfer Application (no Profile).


  Copyright 2004-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

/*********************************************************************
  This sample application is basically a cable replacement
  and it should be customized for your application. A PC
  (or other device) sends data via the serial port to this
  application's device.  This device transmits the message
  to another device with the same application running. The
  other device receives the over-the-air message and sends
  it to a PC (or other device) connected to its serial port.
				
  This application doesn't have a profile, so it handles everything directly.

  Key control:
    SW1:
    SW2:  initiates end device binding
    SW3:
    SW4:  initiates a match description request
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include <stdio.h>
#include <string.h>
#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "SerialApp.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "OSAL_NV.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"

/*include jstack inc */
#include "JStack\jstack_defs.h"
#include "JStack\jstack_structs.h"
#include "JStack\jstack_func.h"

/* include w5100 model def */
#include "driver\W5x00\W5100.h"

/* include eem headers */
#include <eem\eem_defs.h>
#include <eem\eem_struct.h>
#include "eem\eem.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#if !defined( SERIAL_APP_PORT )
#define SERIAL_APP_PORT  0
#endif

#if !defined( SERIAL_APP_BAUD )
  //#define SERIAL_APP_BAUD  HAL_UART_BR_38400
  #define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX )
#define SERIAL_APP_TX_MAX  80
#endif

/* header + delay value */
#define SERIAL_APP_RSP_CNT  (JMI_MSG_HEADER_LEN + 2)

/* retry 3 times, or we drop this packet */
#define SERIAL_APP_SEND_AGAIN_MAX_TIMES 3

// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[SERIALAPP_MAX_CLUSTERS] =
{
  SERIALAPP_CLUSTER_DATA_TRANS_REQ,
  SERIALAPP_CLUSTER_DATA_TRANS_RESP,
  SERIALAPP_CLUSTER_EP_CONNECT_REQ,            
  SERIALAPP_CLUSTER_EP_CONNECT_RSP,
  SERIALAPP_CLUSTER_COMMAND_REQ,
  SERIALAPP_CLUSTER_COMMAND_RSP
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
  SERIALAPP_ENDPOINT,              //  int   Endpoint;
  SERIALAPP_PROFID,                //  uint16 AppProfId[2];
  SERIALAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  SERIALAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  SERIALAPP_FLAGS,                 //  int   AppFlags:4;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)SerialApp_ClusterList,  //  byte *pAppInClusterList;
  SERIALAPP_MAX_CLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)SerialApp_ClusterList   //  byte *pAppOutClusterList;
};

endPointDesc_t SerialApp_epDesc =
{
  SERIALAPP_ENDPOINT,
 &SerialApp_TaskID,
  (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
  noLatencyReqs
};

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
devStates_t         SampleApp_NwkState;   
uint8               SerialApp_TaskID;           // Task ID for internal task/event processing.
uint8               gucMaxEpid  = 0;


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*-+-+-+-+-+-+-+-+-+-+ common global vars define here -+-+-+-+-+-+-+-+-+-+-+*/
static uint8            SerialApp_MsgID         = 0;        /* for afsend callback msg seq */
static afAddrType_t     SerialApp_TxAddr        = {0};      /* save remote ep addr */
static afAddrType_t     SerialApp_RxAddr        = {0};      /* save for addr too */

/*-+-+-+-+-+-+-+-+-+-+ ep or router global vars define -+-+-+-+-+-+-+-+-+-+*/
static uint8            SerialApp_TxBuf[SERIAL_APP_TX_MAX+1];   /* uart recv buff, and send to coordi */
static uint8            SerialApp_TxLen = 0;                    /* last msg len, need if we retrans message */

#if !ZDO_COORDINATOR
static volatile uint8   gucAckFlag              = 0;            /* indicate send data whether is recved, 1 is set to need ack, when ack, set 0 */
static uint8            gucSendAgainCnt         = 0;            /* indicate retry count, max is 3 times*/
#endif

/*-+-+-+-+-+-+-+-+-+-+ coordinator global vars define -+-+-+-+-+-+-+-+-+-+*/
/* broadcast addr for coordi */
static afAddrType_t     gBroadCastAddr          = {0};

#ifdef ZDO_COORDINATOR

/* buff for send to ep */
static uint8 SerialApp_RspBuf[SERIAL_APP_RSP_CNT];

/* cp control block, max 254 */
static uint16 gausSerialAppCb[JMI_INVALID_EPID] = {0};
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
/*-+-+-+-+-+-+-+-+-+-+ common local functions define -+-+-+-+-+-+-+-+-+-+*/
static void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );
static void SerialApp_CallBack(uint8 port, uint8 event);      
static void SerialApp_DataTransReq(uint8* , uint16);
static void SerialApp_DataTransResp(uint8* , uint16);
static void SerialApp_ConnectReqProcess(uint8* , uint16); 
static void SerialApp_DeviceConnect(void); 
static void SerialApp_DeviceConnectRsp(uint8* , uint16);       
static void SerialApp_CommandReq(uint8* , uint16);

/*-+-+-+-+-+-+-+-+-+-+ ep or router local functions define -+-+-+-+-+-+-+-+-+-+*/
#if !ZDO_COORDINATOR
static void SerialApp_SendAgain(void);
#endif

/*-+-+-+-+-+-+-+-+-+-+ coordi local functions define -+-+-+-+-+-+-+-+-+-+*/
#ifdef ZDO_COORDINATOR
static void SerialApp_Resp(void);
#endif

/*********************************************************************
 * @fn      SerialApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void SerialApp_Init( uint8 task_id )
{
    halUARTCfg_t uartConfig;

    SerialApp_TaskID = task_id;
    SampleApp_NwkState = DEV_INIT;

    afRegister( (endPointDesc_t *)&SerialApp_epDesc );

    RegisterForKeys( task_id );

    uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
    uartConfig.baudRate             = HAL_UART_BR_9600;
    uartConfig.flowControl          = FALSE;
    uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
    uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
    uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
    uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
    uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
    uartConfig.callBackFunc         = SerialApp_CallBack;
    HalUARTOpen (SERIAL_APP_PORT, &uartConfig);

    ZDO_RegisterForZDOMsg( SerialApp_TaskID, End_Device_Bind_rsp );
    ZDO_RegisterForZDOMsg( SerialApp_TaskID, Match_Desc_rsp );
  
    // Setup for the periodic message's destination address
    // Broadcast to everyone
    gBroadCastAddr.addrMode        = (afAddrMode_t)AddrBroadcast;
    gBroadCastAddr.endPoint        = SERIALAPP_ENDPOINT;
    gBroadCastAddr.addr.shortAddr  = 0xFFFF; 
}

/*********************************************************************
 * @fn      SerialApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{
  uint8                     ucRet       = 0;
  JMI_NV_ITEM_CUR_EPID_S    stNvCurEpid = {0};
  
  (void)task_id;  // Intentionally unreferenced parameter
  
  if ( events & SYS_EVENT_MSG )
  {
    afIncomingMSGPacket_t *MSGpkt;

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SerialApp_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
      case AF_INCOMING_MSG_CMD:        
        SerialApp_ProcessMSGCmd( MSGpkt );
        break;
        
      case ZDO_STATE_CHANGE:        
        SampleApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
        
        if ( (SampleApp_NwkState == DEV_ZB_COORD)
            || (SampleApp_NwkState == DEV_ROUTER)
            || (SampleApp_NwkState == DEV_END_DEVICE) )
        {
            // Start sending the periodic message in a regular interval.
            //HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);

            #if !ZDO_COORDINATOR
            /* clear flags */
            gucAckFlag      = 0;
            gucSendAgainCnt = 0;
            #endif

            /* init jstack */
            JMI_Init();

            /* 如果配置为协调器，需要初始化网络(第一版不用网络，涉及到与外购网络模块匹配问题，用串口兼容) */
            #if (defined(ZDO_COORDINATOR) && defined(USING_W5100) && (TRUE == USING_W5100))
            /* start a timer to init W5100 */
            osal_start_timerEx( SerialApp_TaskID, SERIALAPP_INIT_W5100_EVT, 1000 ); 
            #endif   
            
            /* if not a coordi devivce, need regist self to coordi */
            if(SampleApp_NwkState != DEV_ZB_COORD)
            {
                /* init nv for epid config */
                ucRet = osal_nv_item_init(JMI_NV_CONFIG_ITEM_CUR_EP_ID, sizeof(JMI_NV_ITEM_CUR_EPID_S), NULL);
                if ((SUCCESS == ucRet) || (NV_ITEM_UNINIT == ucRet))
                {
                    /* read self epid */
                    ucRet = osal_nv_read(JMI_NV_CONFIG_ITEM_CUR_EP_ID, 0, sizeof(JMI_NV_ITEM_CUR_EPID_S), (void *) &stNvCurEpid);
                    if (ZSUCCESS == ucRet)
                    {
                        /* set cur epid */
                        JMI_SetEpId(stNvCurEpid.ucEpid);
                    }                
                }
                
                /* start to regist */
                SerialApp_DeviceConnect();
            }

#if ZDO_COORDINATOR	
            u16             usTotalLen  = 0;    
            EEM_HEADER_S    *pHeader    = NULL;
            u8              *pcBuff     = NULL;
            
        	pHeader = EEM_CreateHeader(0, EEM_COMMAND_COOR_STATCHAG, UCORE_ERR_SUCCESS);
        	if (NULL != pHeader)
        	{
        		pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                if (NULL != pcBuff)
                {
                    /* send to uart */
                    HalUARTWrite(0, (uint8 *) pcBuff, usTotalLen);                    
                }
                
                /* clean buffer */
                EEM_Delete((void **) &pHeader);
            }  
#endif            
        }
        else
        {
          // Device is no longer in the network
        }
        break;

      default:
        break;
      }

      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    return ( events ^ SYS_EVENT_MSG );
  }

/* ep or router event process */
#if !ZDO_COORDINATOR
  if ( events & SERIALAPP_SEND_EVT )
  {
    SerialApp_SendAgain();
    return ( events ^ SERIALAPP_SEND_EVT );
  }

  /* regist timer, only if we regist failed */
  if ( events & SERIALAPP_REGIST_EVT )
  {
    SerialApp_DeviceConnect();
    return ( events ^ SERIALAPP_REGIST_EVT );  
  }

  /* regist ack check timer, when coordi not response, here we regist again */
  if ( events & SERIALAPP_REGIST_ACK_CHECK_EVT )
  {
    SerialApp_DeviceConnect();
    return ( events ^ SERIALAPP_REGIST_ACK_CHECK_EVT );  
  }
  
#endif

/* coordi event process */
#ifdef ZDO_COORDINATOR
#if (defined(USING_W5100) && (TRUE == USING_W5100))
    if ( events & SERIALAPP_INIT_W5100_EVT )
    {
        /* W5100端口初始化配置 */
        W5100_Initialization();		//W5100初始化配置

        JMIDebug("W5100 inited.\r\n");

        /* set a event to connect server */
        osal_set_event( SerialApp_TaskID, SERIALAPP_TCP_CONN_EVT);  

        return ( events ^ SERIALAPP_INIT_W5100_EVT );
    }

    if ( events & SERIALAPP_TCP_CONN_EVT )
    {
        JMIDebug("conn trying.\r\n");

        /* W5100端口初始化配置 */
        W5100_Socket_Start();

        return ( events ^ SERIALAPP_TCP_CONN_EVT );
    }
#endif

    if ( events & SERIALAPP_RESP_EVT )
    {
        SerialApp_Resp();
        return ( events ^ SERIALAPP_RESP_EVT );
    }
#endif
  
  return ( 0 );  // Discard unknown events.
}

/*********************************************************************
 * @fn      SerialApp_ProcessMSGCmd
 *
 * @brief   Data message processor callback. This function processes
 *          any incoming data - probably from other devices. Based
 *          on the cluster ID, perform the intended action.
 *
 * @param   pkt - pointer to the incoming message packet
 *
 * @return  TRUE if the 'pkt' parameter is being used and will be freed later,
 *          FALSE otherwise.
 */
void SerialApp_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )
{
#if defined ( LCD_SUPPORTED )
    uint8 LQI = pkt->LinkQuality;
    int16 iRSSI = -(81-(LQI*91)/255);
    char sRSSI[10] = {0};
    
    sprintf(sRSSI, "RSSI:%d", iRSSI);    
    HalLcdWriteString(sRSSI, HAL_LCD_LINE_3 );
#endif

    switch ( pkt->clusterId )
    {
        case SERIALAPP_CLUSTER_DATA_TRANS_REQ:
        {
            // Store the address for sending and retrying.
            osal_memcpy(&SerialApp_RxAddr, &(pkt->srcAddr), sizeof( afAddrType_t ));

            SerialApp_DataTransReq((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        case SERIALAPP_CLUSTER_DATA_TRANS_RESP:
        {
            SerialApp_DataTransResp((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;        
        }
        case SERIALAPP_CLUSTER_EP_CONNECT_REQ:
        {
            SerialApp_ConnectReqProcess((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        case SERIALAPP_CLUSTER_EP_CONNECT_RSP:
        {
            SerialApp_DeviceConnectRsp((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        case SERIALAPP_CLUSTER_COMMAND_REQ:
        {
            SerialApp_CommandReq((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        default:
        {
            break;
        }
    }
}

void SerialApp_CommandReq(uint8 *pcBuf, uint16 usLen)
{
    JMI_MSG_HEADER_S        *pstHeader          = NULL;
    EEM_HEADER_S            *pHeader            = NULL;    
    uint8                   *pcDataBuf          = NULL;
    uint16                  usDataLen           = 0;

    JMI_PARA_ASSERT(pcBuf);

    if (false == JMI_ValidateMessage(pcBuf, usLen))
    {        
        return;
    }

    pstHeader = (JMI_MSG_HEADER_S *) pcBuf;

    /* judge command */
    switch (pstHeader->ucCommand)
    {
        /* trans command directly send to remote */
        case JMI_COMMAND_TRANSPARENT_COM:
        {
            pcDataBuf = pcBuf + JMI_MSG_HEADER_LEN;
            usDataLen = usLen - JMI_MSG_HEADER_LEN;

            /* send to uart 0 */
            if ( !HalUARTWrite2( SERIAL_APP_PORT, pcDataBuf, usDataLen) )
            {
                //better to start a timer retrying...
            }
            break;
        }
        case JMI_COMMAND_REPORT_GPS:
        {            
            pcDataBuf = pcBuf + JMI_MSG_HEADER_LEN;
            usDataLen = usLen - JMI_MSG_HEADER_LEN;
            
            /* send to uart 0 */
            HalUARTWrite(0, pcDataBuf, usDataLen);

#if 0/* for gps loop back testing */            
        	pHeader = (EEM_HEADER_S *) pcDataBuf;

            /* change ep id */
            //pHeader->ucEpId = JMI_GetEpId();
            
            if ((NULL != pcDataBuf) && (usDataLen + JMI_MSG_HEADER_LEN < SERIAL_APP_TX_MAX))
            {
                JMI_MSG_HEADER_S *pstMsgHeaer = (JMI_MSG_HEADER_S *) SerialApp_TxBuf;

                /* fill msg header */
                JMI_FillMessageReqHeader(pstMsgHeaer, JMI_COMMAND_REPORT_GPS, usDataLen);
                /* fill body */
                JMI_FillMessageBody(pstMsgHeaer, usDataLen, (void *) pcDataBuf);

                SerialApp_TxLen = JMI_MSG_HEADER_LEN + usDataLen;
                                             
                /* broadcast data */
                if (afStatus_SUCCESS != AF_DataRequest(&gBroadCastAddr,
                                                     (endPointDesc_t *)&SerialApp_epDesc,
                                                      SERIALAPP_CLUSTER_COMMAND_REQ,
                                                      SerialApp_TxLen, SerialApp_TxBuf,
                                                      &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
                {
                    /* oops */
                    break;
                }
            }
#endif
            break;            
        }
        default:
        {
            break;
        }
    }
}

#if !ZDO_COORDINATOR
#pragma optimize=none
void SerialApp_SendAgain()
{
    uint8 ucRegistStatus = 0;

    /* first check regist status */
    ucRegistStatus = JMI_GetRegistStatus();
    if (JMI_REGIST_REGITED != ucRegistStatus)
    {
        return;
    }

    /* check if exceed max retry times */
    if (SERIAL_APP_SEND_AGAIN_MAX_TIMES <= gucSendAgainCnt)
    {
        /* clear ack flag to allow next packet in */
        gucAckFlag      = 0;
        gucSendAgainCnt = 0;

        /* stop retrans timer event first */
        osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_SEND_EVT); 
    
        return;
    }

    /* here we enter retrans process */
    if (( 0 < SerialApp_TxLen) && (0 != gucAckFlag))
    {
        /* no matter whether af_send is successed, count is inc */
        gucSendAgainCnt++;
    
        if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_TxAddr,
                                             (endPointDesc_t *)&SerialApp_epDesc,
                                              SERIALAPP_CLUSTER_DATA_TRANS_REQ,
                                              SerialApp_TxLen, SerialApp_TxBuf,
                                              &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
        {
            osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
            return;
        }

        /* start a timer event to wait coord response (1s) */
        osal_start_timerEx( SerialApp_TaskID, SERIALAPP_SEND_EVT, 1000 );
    }
    
    return;
}
#endif

/*********************************************************************
 * @fn      SerialApp_Resp
 *
 * @brief   Send data OTA.
 *
 * @param   none
 *
 * @return  none
 */
#ifdef ZDO_COORDINATOR
static void SerialApp_Resp(void)
{
  if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_RxAddr,
                                         (endPointDesc_t *)&SerialApp_epDesc,
                                          SERIALAPP_CLUSTER_DATA_TRANS_RESP,
                                          SERIAL_APP_RSP_CNT, SerialApp_RspBuf,
                                         &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
  {
    osal_set_event(SerialApp_TaskID, SERIALAPP_RESP_EVT);
  }
}
#endif  

/*********************************************************************
 * @fn      SerialApp_CallBack
 *
 * @brief   Send data OTA.
 *
 * @param   port - UART port.
 * @param   event - the UART port event flag.
 *
 * @return  none
 */
#pragma optimize=none
void SerialApp_CallBack(uint8 port, uint8 event)
{
    uint8           ucResult        = 0;
    uint16          usRecvLen       = 0;
    EEM_HEADER_S    *pHeader        = NULL;

    /* check event type, and when ack flag is 0, prepare to read from uart */
    if (event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT))
    {
        /* clear buff first */
        osal_memset(SerialApp_TxBuf, 0, SERIAL_APP_TX_MAX);

        /* get the data head */        
        usRecvLen = HalUARTRead(SERIAL_APP_PORT, SerialApp_TxBuf, SERIAL_APP_TX_MAX); 
        /* check if we read data */
        if ( 0 < usRecvLen )
        {
            /* parse message in buffer */
            ucResult = EEM_GetMessage(SerialApp_TxBuf, (uint8 *) &usRecvLen, &pHeader);
            if (UCORE_ERR_SUCCESS == ucResult)
            {
    		    switch (pHeader->usCommand)
                {
                    case EEM_COMMAND_QUERY_EPINFO:
                        {
                            u16                     usTotalLen          = 0;    
                            EEM_HEADER_S            *pRspHeader         = NULL;
                            u8                      *pcBuff             = NULL;
                            EEM_EP_INFO_S           stSelfInfo          = {0};

                            /* prepare ep info */                                                        
#if defined(ZDO_COORDINATOR)
                            stSelfInfo.ucEpId     = 0;
                            stSelfInfo.ucEpType   = 2;    /* coord device */
                            stSelfInfo.usEpAddr   = NLME_GetShortAddr();
#elif defined(RTR_NWK)
                            stSelfInfo.ucEpId     = JMI_GetEpId();
                            stSelfInfo.ucEpType   = 1;    /* router device */
                            stSelfInfo.usEpAddr   = NLME_GetShortAddr();
#else
                            stSelfInfo.ucEpId     = JMI_GetEpId();
                            stSelfInfo.ucEpType   = 0;    /* end device */
                            stSelfInfo.usEpAddr   = NLME_GetShortAddr();
#endif
                            /* send ep online message main board */  
                            pRspHeader = EEM_CreateRespHeader(pHeader, UCORE_ERR_SUCCESS);
                        	if (NULL != pRspHeader)
                        	{
                                /* apend info */
                            	ucResult = EEM_AppendPayload(&pRspHeader, EEM_PAYLOAD_TYPE_EP_INFO, sizeof(EEM_EP_INFO_S), (void *) &(stSelfInfo));
                                if (UCORE_ERR_SUCCESS == ucResult)
                                {
                                    /* get buff */
                                	pcBuff = EEM_GetBuff(pRspHeader, &usTotalLen);
                                    if (NULL != pcBuff)
                                    {
                                        /* send to uart */
                                        HalUARTWrite(0, (uint8 *) pcBuff, usTotalLen);        
                                    }
                                }

                                /* clean buffer */
                                EEM_Delete((void **) &pRspHeader);
                            }
                        }
                        break;
                    case EEM_COMMAND_TRANS_COM:
                        {
                            uint16          usPayLen    = 0;
                            uint16          *pEpAddr    = NULL;
                            void            *pRawData   = NULL;
                            
                            /* here get message payload */
                            pEpAddr  = (uint16 *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_ADDR, &usPayLen);
                            pRawData = EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, &usPayLen);
                            if ((NULL != pEpAddr) && (NULL != pRawData) && (usPayLen + JMI_MSG_HEADER_LEN < SERIAL_APP_TX_MAX))
                            {
                                JMI_MSG_HEADER_S *pstMsgHeaer = (JMI_MSG_HEADER_S *) SerialApp_TxBuf;

                                /* fill msg header */
                                JMI_FillMessageReqHeader(pstMsgHeaer, JMI_COMMAND_TRANSPARENT_COM, usPayLen);
                                /* fill body */
                                JMI_FillMessageBody(pstMsgHeaer, usPayLen, pRawData);

                                SerialApp_TxLen = JMI_MSG_HEADER_LEN + usPayLen;

                                /* set address */
                                SerialApp_TxAddr.addrMode       = (afAddrMode_t)Addr16Bit;
                                SerialApp_TxAddr.endPoint       = SERIALAPP_ENDPOINT;
                                SerialApp_TxAddr.addr.shortAddr = *pEpAddr;
                                                                
                                /* send data, otherwise start a timer to retrans*/
                                if (afStatus_SUCCESS != AF_DataRequest(&SerialApp_TxAddr,
                                                                     (endPointDesc_t *)&SerialApp_epDesc,
                                                                      SERIALAPP_CLUSTER_COMMAND_REQ,
                                                                      SerialApp_TxLen, SerialApp_TxBuf,
                                                                      &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
                                {
                                    /* set retrans event immediatlly */
                                    osal_set_event(SerialApp_TaskID, SERIALAPP_SEND_EVT);
                                    break;
                                }
                            }
                        }
                        break;
                    case EEM_COMMAND_REPORT_GPS:
                        {
                            u16                 usTotalLen  = 0;
                            u8                  *pcBuff     = NULL;

                            /* broadcast this message */
                        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                            if ((NULL != pcBuff) && (usTotalLen + JMI_MSG_HEADER_LEN < SERIAL_APP_TX_MAX))
                            {
                                JMI_MSG_HEADER_S *pstMsgHeaer = (JMI_MSG_HEADER_S *) SerialApp_TxBuf;

                                /* fill msg header */
                                JMI_FillMessageReqHeader(pstMsgHeaer, JMI_COMMAND_REPORT_GPS, usTotalLen);
                                /* fill body */
                                JMI_FillMessageBody(pstMsgHeaer, usTotalLen, (void *) pcBuff);

                                SerialApp_TxLen = JMI_MSG_HEADER_LEN + usTotalLen;
                                                             
                                /* broadcast data */
                                if (afStatus_SUCCESS != AF_DataRequest(&gBroadCastAddr,
                                                                     (endPointDesc_t *)&SerialApp_epDesc,
                                                                      SERIALAPP_CLUSTER_COMMAND_REQ,
                                                                      SerialApp_TxLen, SerialApp_TxBuf,
                                                                      &SerialApp_MsgID, 0, AF_DEFAULT_RADIUS))
                                {
                                    /* oops */
                                    break;
                                }       
                            }
                        }
                        break;
                    default:
                        break;
                }                    

                /* all message delete here */
    			EEM_Delete((void **) &pHeader);
            }
        }
    }  
    
    return;
}

/*********************************************************************
*********************************************************************/
void  SerialApp_DeviceConnect()              
{
#if !ZDO_COORDINATOR
    uint16  nwkAddr                 = 0;
    uint16  parentNwkAddr           = 0;
    uint16  usLen                   = 0;
    char    szRegistBuff[30]        = {0};

    //HalLedBlink( HAL_LED_2, 3, 50, (1000 / 4) );

    /* get self and coordi short addr */
    nwkAddr = NLME_GetShortAddr();
    
    /* parentNwkAddr = NLME_GetCoordShortAddr(); */
    parentNwkAddr = 0; /* we fix parent addr is the root device which is coordi */

    /* prepare dest addr */
    SerialApp_TxAddr.addrMode       = (afAddrMode_t)Addr16Bit;
    SerialApp_TxAddr.endPoint       = SERIALAPP_ENDPOINT;
    SerialApp_TxAddr.addr.shortAddr = parentNwkAddr;

    /* get buff len */
    usLen = sizeof(szRegistBuff);

    /* make regist buff */
    JMI_MakeRegistInfo(nwkAddr, (uint8 *) szRegistBuff, &usLen);

    if ( AF_DataRequest( &SerialApp_TxAddr, &SerialApp_epDesc,
                       SERIALAPP_CLUSTER_EP_CONNECT_REQ,
                       usLen,
                       (uint8*)szRegistBuff,
                       &SerialApp_MsgID, 
                       0, 
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
        JMI_SetRegistStatus(JMI_REGIST_REGISTING);
    }
    else
    {
        // Error occurred in request to send.
    }

    /* start a timer to check regist result*/
    osal_start_timerEx( SerialApp_TaskID, SERIALAPP_REGIST_ACK_CHECK_EVT, SERIALAPP_REGIST_ACK_CHECK_TIMER_DELAY );     
  
#endif    //!ZDO_COORDINATOR
}

#if defined(JSTACK_DEBUG) && defined(ZDO_COORDINATOR)
static uint16   usDumpLen       = 0;
static uint8    szDumpMsg[64]   = {0};
#endif
void SerialApp_DataTransReq(uint8 *pcBuf, uint16 usLen)
{
#ifdef ZDO_COORDINATOR
    uint8                     stat            = 0;
    uint8                     delay           = 0;
    uint8                     ucEpid          = 0;
    uint16                    usSeq           = 0;
    uint8                     ucRet           = 0;
    uint16                    usTotalLen      = 0;    
    JMI_COMM_DELAY_ACK_S      *pstDelayAck    = NULL;
    JMI_MSG_HEADER_S          *pstMsgHeader   = NULL;
    EEM_HEADER_S              *pHeader        = NULL;
    uint8                     *pcEmOut        = NULL;
  
    
    #ifdef JSTACK_DEBUG
    char                      szDebugBuff[32] = {0};
    #endif
    
    JMI_PARA_ASSERT(pcBuf);
    
    /* validate header */
    if (false == JMI_ValidateMessage(pcBuf, usLen))
    {
        JMIDebug("check message failed.\r\n");
        
        return;
    }

    /* get message header */
    pstMsgHeader = (JMI_MSG_HEADER_S *) pcBuf;

    /* save epid before reasign ptr */
    ucEpid = pstMsgHeader->ucEPId;    

    /* check epid */
    if (0xFF <= ucEpid)
    {
        JMIDebug("check epid failed.\r\n");
        
        return;
    }
    
    // Keep message if not a repeat packet
    usSeq = pstMsgHeader->usSeq;
    if (usSeq > gausSerialAppCb[ucEpid])
    {
        /* send ep online message main board */    
    	pHeader = EEM_CreateHeader(pstMsgHeader->ucEPId, EEM_COMMAND_TRANS_COM, UCORE_ERR_SUCCESS);
    	if (NULL == pHeader)
    	{
            return;
        }

        /* append raw data */
    	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, usLen - JMI_MSG_HEADER_LEN, (void *) (pcBuf + JMI_MSG_HEADER_LEN));
        if (UCORE_ERR_SUCCESS != ucRet)
        {
            /* clean buffer */
            EEM_Delete((void **) &pHeader);
            return;
        }

        /* get buff */
    	pcEmOut = EEM_GetBuff(pHeader, &usTotalLen);
        if (NULL == pcEmOut)
        {
            /* clean buffer */
            EEM_Delete((void **) &pHeader);
            return;
        }
        
        // Transmit the data on the serial port or net.        
        #if (defined(USING_W5100) && (TRUE == USING_W5100))
        if (W5100_SocketSend(0, pcEmOut, usTotalLen))
        #else
        if ( HalUARTWrite( SERIAL_APP_PORT, pcEmOut, usTotalLen ) )
        #endif
        {
        
          #ifdef JSTACK_DEBUG
          usDumpLen = usLen - JMI_MSG_HEADER_LEN;
          osal_memcpy(szDumpMsg, pcBuf + JMI_MSG_HEADER_LEN, usDumpLen);
          #endif
        
          // Save for next incoming message
          gausSerialAppCb[ucEpid] = usSeq;
          stat = JMI_RESULT_SUCCESS;
        }
        else
        {
          stat = JMI_RESULT_OTA_SER_BUSY;
        }

        /* clean buffer */
        EEM_Delete((void **) &pHeader);        
    }
    else
    {
        #ifdef JSTACK_DEBUG        
        sprintf(szDebugBuff, "dump seq:%d\r\nold:\r\n", usSeq);
        JMIDebug(szDebugBuff);
        for (uint8 i=0; i < usDumpLen; i++)
        {
            sprintf(szDebugBuff, "%X ", szDumpMsg[i]);
            JMIDebug(szDebugBuff);
        }
        JMIDebug("\r\nnew:\r\n");
        uint8 *pcDump = pcBuf + JMI_MSG_HEADER_LEN;
        for (uint8 i=0; i < usLen - JMI_MSG_HEADER_LEN; i++)
        {
            sprintf(szDebugBuff, "%X ", pcDump[i]);
            JMIDebug(szDebugBuff);
        }        
        JMIDebug("\r\n");
        #endif
        
        stat = JMI_RESULT_OTA_DUP_MSG;
        /* gausSerialAppCb[ucEpid] = usSeq; */
    }

    /* check if we need response to ep */
#if ((defined SERIALAPP_TRANS_NEED_ACK) && (SERIALAPP_TRANS_NEED_ACK == TRUE))
    // Select approproiate OTA flow-control delay.
    delay = (stat == JMI_RESULT_OTA_SER_BUSY) ? SERIALAPP_NAK_DELAY : SERIALAPP_ACK_DELAY;
    
    // Build & send OTA response message.
    pstMsgHeader = (JMI_MSG_HEADER_S *) SerialApp_RspBuf;

    /* fill msg header */
    JMI_FillMessageRspHeader(pstMsgHeader, ucEpid, gausSerialAppCb[ucEpid], JMI_COMMAND_TRANSPARENT_COM, stat, 2);

    /* tans to private struct */
    pstDelayAck = (JMI_COMM_DELAY_ACK_S *) SerialApp_RspBuf;

    /* set delay value */
    pstDelayAck->usDelay = delay;

    /* need send ack event */
    osal_set_event( SerialApp_TaskID, SERIALAPP_RESP_EVT );
    
    //osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_RESP_EVT); 
#endif

#endif

    return;
}

#pragma optimize=none
void SerialApp_DataTransResp(uint8 *pcBuf, uint16 usLen)
{
#if !ZDO_COORDINATOR
    uint8                     ucResult        = 0;
    uint8                     delay           = 0;    
    JMI_COMM_DELAY_ACK_S      *pstDelayAck    = NULL;

    /* validate header */
    if (false == JMI_ValidateMessage(pcBuf, usLen))
    {
        JMIDebug("check message failed.\r\n");
        
        return;
    }

    /* get message header */
    pstDelayAck = (JMI_COMM_DELAY_ACK_S *) pcBuf;

    /* check ack seq*/
    if (pstDelayAck->stHeaer.usSeq != JMI_GetCurrentSeq())
    {
        return;
    }

    /* stop retrans timer event first */
    osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_SEND_EVT); 

    /* check trans result */
    ucResult = pstDelayAck->stHeaer.ucResult;

    /* if we trans success, start next data trans */
    if ((JMI_RESULT_SUCCESS == ucResult) || (JMI_RESULT_OTA_DUP_MSG == ucResult))
    {        
        /* reset flag to start next trans */
        gucAckFlag      = 0;
        gucSendAgainCnt = 0;
    }

    /* if coord response busy status, we start a timer to retrans */
    if (JMI_RESULT_OTA_SER_BUSY == ucResult)
    {
        // Re-start timeout according to delay sent from other device.
        delay = pstDelayAck->usDelay;

        /* check delay value */
        if ((SERIALAPP_NAK_DELAY == delay) || (SERIALAPP_ACK_DELAY == delay))
        {
            gucSendAgainCnt = 0;
            
            osal_start_timerEx( SerialApp_TaskID, SERIALAPP_SEND_EVT, delay );            
        }
    }
#endif
    return;
}

#pragma optimize=none/* 解决断点跳跃的问题，告诉编译器不要对该函数优化，否则断点可能会偏移 */
void SerialApp_DeviceConnectRsp(uint8 *pcBuf, uint16 usLen)
{
#if !ZDO_COORDINATOR
    uint8                   ucRet               = 0;
    JMI_MSG_HEADER_S        *pstHeader          = NULL;
    JMI_NV_ITEM_CUR_EPID_S  stCurEpid           = {0};

    JMI_PARA_ASSERT(pcBuf);

    /* stop ack check timer first */
    osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_REGIST_ACK_CHECK_EVT);    

    if (false == JMI_ValidateMessage(pcBuf, usLen))
    {        
        return;
    }

    pstHeader = (JMI_MSG_HEADER_S *) pcBuf;
    
    /* check if regist success */
    if (JMI_RESULT_SUCCESS != pstHeader->ucResult)
    {
        /* set a timer to regist again */
        osal_start_timerEx( SerialApp_TaskID, SERIALAPP_REGIST_EVT, SERIALAPP_REGIST_TIMER_DELAY ); 
        
        return;
    }

    /* stop timer first */
    osal_stop_timerEx(SerialApp_TaskID, SERIALAPP_REGIST_EVT);

    /* here set epid */
    stCurEpid.ucEpid = pstHeader->ucEPId;
    ucRet= osal_nv_write(JMI_NV_CONFIG_ITEM_CUR_EP_ID, 0, sizeof(JMI_NV_ITEM_CUR_EPID_S), (void *) &stCurEpid);
    if (ZSUCCESS == ucRet)
    {
        JMI_SetEpId(stCurEpid.ucEpid);
    }
    
    /* set regist status */
    JMI_SetRegistStatus(JMI_REGIST_REGITED);
#endif
}

void SerialApp_ConnectReqProcess(uint8 *pcBuf, uint16 usLen)
{
#ifdef ZDO_COORDINATOR
    uint8                   ucRet               = 0;
    char                    szRespBuff[30]      = {0};
    JMI_MSG_HEADER_S        *pstHeader          = NULL;
    JMI_REGIST_INFO_S       *pstRegistInfo      = NULL;
    JMI_NV_ITEM_MAX_EPID_S  stNvMaxEpid         = {0};
    u16                     usTotalLen          = 0;    
    EEM_HEADER_S            *pHeader            = NULL;
    u8                      *pcBuff             = NULL;
    EEM_EP_INFO_S           stEpInfo            = {0};
    
    JMI_PARA_ASSERT(pcBuf);

    if (false == JMI_ValidateMessage(pcBuf, usLen))
    {        
        JMIDebug("invalid regist info\r\n");
        return;
    }

    pstRegistInfo = (JMI_REGIST_INFO_S *) pcBuf;

    /* set ep addr */
    SerialApp_TxAddr.addrMode = (afAddrMode_t)Addr16Bit;
    SerialApp_TxAddr.endPoint = SERIALAPP_ENDPOINT;
    SerialApp_TxAddr.addr.shortAddr = pstRegistInfo->usShortAddr;

    /* first read max asigned epid from nv */
    ucRet = osal_nv_item_init(JMI_NV_CONFIG_ITEM_MAX_EP_ID, sizeof(JMI_NV_ITEM_MAX_EPID_S), (void *) &stNvMaxEpid);
    if ((SUCCESS == ucRet) || (NV_ITEM_UNINIT == ucRet))
    {
        ucRet = osal_nv_read(JMI_NV_CONFIG_ITEM_MAX_EP_ID, 0, sizeof(JMI_NV_ITEM_MAX_EPID_S), (void *) &stNvMaxEpid);
        if (ZSUCCESS != ucRet)
        {
            sprintf(szRespBuff, "read nv fail,ret:%d\r\n", ucRet);
            JMIDebug(szRespBuff);
                            
            pstRegistInfo->stHeaer.ucEPId = ++gucMaxEpid;
        }
        else
        {
            /* here asign epid, if ep regist whih 0xFF epid, then....*/
            if (0xFF == pstRegistInfo->stHeaer.ucEPId)
            {
                if (0xFF <= stNvMaxEpid.ucEpid )
                {
                    /* need init max id value */
                    stNvMaxEpid.ucEpid = 0;
                    ucRet= osal_nv_write(JMI_NV_CONFIG_ITEM_MAX_EP_ID, 0, sizeof(JMI_NV_ITEM_MAX_EPID_S), (void *) &stNvMaxEpid);
                    if (ZSUCCESS != ucRet)
                    {
                        sprintf(szRespBuff, "init max epid fail,ret:%d\r\n", ucRet);
                        JMIDebug(szRespBuff);
                    }
                    
                    JMIDebug("max epid init again\r\n");
                }
                else
                {
                    #ifdef JSTACK_DEBUG
                    sprintf(szRespBuff, "max epid:%d\r\n", stNvMaxEpid.ucEpid);
                    JMIDebug(szRespBuff);
                    #endif
                    
                    /* normal condition, two will equal */
                    if (gucMaxEpid == stNvMaxEpid.ucEpid)
                    {
                        gucMaxEpid = ++stNvMaxEpid.ucEpid;
                    }
                    else
                    {
                        stNvMaxEpid.ucEpid = ++gucMaxEpid;
                    }

                    /* change epid for ep */
                    pstRegistInfo->stHeaer.ucEPId = gucMaxEpid;
                    
                    /* write back max epid for next ep device */
                    ucRet= osal_nv_write(JMI_NV_CONFIG_ITEM_MAX_EP_ID, 0, sizeof(JMI_NV_ITEM_MAX_EPID_S), (void *) &stNvMaxEpid);
                    if (ZSUCCESS != ucRet)
                    {
                        JMIDebug("write max epid failed\r\n");
                    }
                }
            }
        }     
    }  
    else
    {
        sprintf(szRespBuff, "init nv fail,ret:%d\r\n", ucRet);
        JMIDebug(szRespBuff);
                        
        pstRegistInfo->stHeaer.ucEPId = ++gucMaxEpid;    
    }

    /* prepare resp info */
    pstHeader = (JMI_MSG_HEADER_S *) szRespBuff;
    JMI_FillMessageRspHeader(pstHeader, 
                             pstRegistInfo->stHeaer.ucEPId, 
                             pstRegistInfo->stHeaer.usSeq,
                             pstRegistInfo->stHeaer.ucCommand, 
                             JMI_RESULT_SUCCESS, 
                             0);

    if ( AF_DataRequest( &SerialApp_TxAddr, &SerialApp_epDesc,
                       SERIALAPP_CLUSTER_EP_CONNECT_RSP,
                       sizeof(JMI_MSG_HEADER_S),
                       (uint8*)szRespBuff,
                       &SerialApp_MsgID, 
                       0, 
                       AF_DEFAULT_RADIUS ) == afStatus_SUCCESS )
    {
    }
    else
    {
        // Error occurred in request to send.
        JMIDebug("AF send error!\r\n");
        return;
    }

    //here reset recv seq to 0
    if (0xFF > pstRegistInfo->stHeaer.ucEPId)
    {
        gausSerialAppCb[pstRegistInfo->stHeaer.ucEPId] = 0;
    }    

    /* send ep online message main board */    
	pHeader = EEM_CreateHeader(pstRegistInfo->stHeaer.ucEPId, EEM_COMMAND_EP_ONLINE, UCORE_ERR_SUCCESS);
	if (NULL == pHeader)
	{
        return;
    }

    /* prepare ep info */
    stEpInfo.ucEpId     = pstRegistInfo->stHeaer.ucEPId;
    stEpInfo.ucEpType   = pstRegistInfo->ucEpType;
    stEpInfo.usEpAddr   = SerialApp_TxAddr.addr.shortAddr;

    /* apend info */
	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_EP_INFO, sizeof(EEM_EP_INFO_S), (void *) &(stEpInfo));
    if (UCORE_ERR_SUCCESS != ucRet)
    {
        /* clean buffer */
        EEM_Delete((void **) &pHeader);        
        return;
    }

    /* get buff */
	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
    if (NULL != pcBuff)
    {
        /* send to uart */
        HalUARTWrite(0, (uint8 *) pcBuff, usTotalLen);        
    }

    /* clean buffer */
    EEM_Delete((void **) &pHeader);
#endif    

    return;
}

