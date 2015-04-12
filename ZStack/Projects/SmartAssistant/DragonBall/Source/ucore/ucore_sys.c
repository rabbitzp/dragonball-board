/*
*********************************************************************************************************
*
*                                          user system functions
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore_sys.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "OSAL_NV.h"
#include "osal_clock.h"
#include "hal_drivers.h"
#include "hal_uart.h"
#include "hal_key.h"
#include "sapi.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"

/*include user inc */
#include "ucore_defs.h"
#include "ucore_struct.h"
#include "ucore.h"

#include "ZProfile\zprofile_def.h"

/* include eem headers */
#include <eem\eem_struct.h>
#include "eem\eem.h"

/*********************************************************************
 * MACROS
 */
 // This is the max byte count per OTA message.
#if !defined( SERIAL_APP_TX_MAX_BUF_LEN )
#define SERIAL_APP_TX_MAX_BUF_LEN  128
#endif

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * Extern VARIABLES
 */
extern endPointDesc_t gEndPointDesc;


static uint8 gSerialTxBuf[SERIAL_APP_TX_MAX_BUF_LEN + 1];   /* uart recv buff, and send to coordi */

/*********************************************************************
 * LOCAL FUNCTIONS
 */


void UCore_DelayMS(unsigned int msec)
{ 
    unsigned int i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<530; j++);
}


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
void UCore_SerialCallBack(uint8 port, uint8 event)
{
    uint8           ucResult        = 0;
    uint32          ulRecvLen       = 0;
    EEM_HEADER_S    *pHeader        = NULL;

    /* check event type, and when ack flag is 0, prepare to read from uart */
    if (event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL | HAL_UART_RX_TIMEOUT))
    {
        /* clear buff first */
        osal_memset(gSerialTxBuf, 0, SERIAL_APP_TX_MAX_BUF_LEN);

        /* get the data head */        
        ulRecvLen = HalUARTRead(UCORE_EX_COMM_UART_PORT, gSerialTxBuf, SERIAL_APP_TX_MAX_BUF_LEN); 
        /* check if we read data */
        if ( 0 < ulRecvLen )
        {
            /* parse message in buffer */
            ucResult = EEM_GetMessage(gSerialTxBuf, &ulRecvLen, &pHeader);
            if (UCORE_ERR_SUCCESS == ucResult)
            {
    		    switch (pHeader->usCommand)
                {
                    case EEM_COMMAND_SET_PANDID:
                        {
                            uint16          usPayLen    = 0;
                            uint16          *pPanId     = NULL;
                            u16             usTotalLen  = 0;
                            u8              *pcBuff     = NULL;
                            
                            /* here get message payload */
                            pPanId  = (uint16 *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_COORD_NWK_ID, &usPayLen);                            
                            if (NULL != pPanId)
                            {
                                /* check range */
                                if ((*pPanId > 16383) && (65535 != *pPanId))
                                {
                                    break;
                                }
                            }

#if defined(ZDO_COORDINATOR)    /* 如果是协调器，则需要广播此消息 */
                            /* 2 steps, first broadcast this message to all nodes */
                        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                            if (NULL != pcBuff)
                            {
                                /* just broadcast data, maybe rough, but will be rewrite later  */
                                if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(UCORE_BROADCAST_ADDR,
                                                                        ZP_SA_DRAGONBALL_ENDPOINT,
                                                                        &gEndPointDesc, 
                                                                        ZP_SA_CLUSTER_COMMAND_REQ, 
                                                                        usTotalLen, pcBuff,
                                                                        AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
                                {
                                    /* oops */
                                    break;
                                }
                            }

                            /* force update */
                            if (1 == pHeader->ucSPara)
                            {
                                _NIB.nwkPanId = *pPanId;

                                /* try to init */
                                NLME_InitNV();

                                NLME_UpdateNV(NWK_NV_NIB_ENABLE); 
                                
                                SystemReset();
                            } 

                            /* change panid */
                            gChangedPanId = *pPanId;
                            
#else   /* 普通设备到此就可以了，修改完成后重启 */
                            _NIB.nwkPanId = *pPanId;

                            /* try to init */
                            NLME_InitNV();

                            NLME_UpdateNV(NWK_NV_NIB_ENABLE); 
                            
                            SystemReset();
#endif                            
                        }
                        break;
                    case EEM_COMMAND_SET_EPID:
                        {
                            uint16          usPayLen    = 0;
                            uint16          *pEpId      = NULL;

                            /* here get message payload */
                            pEpId  = (uint16 *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_ID, &usPayLen);                            
                            if (NULL != pEpId)
                            {
                                /* check range */
                                if (*pEpId > 255)
                                {
                                    break;
                                }
                            }

                            /* save epid */
                            UCORE_SetEpId(*pEpId);
                        }
                        break; 
                    case EEM_COMMAND_GET_EPID:
                        {
                            u16                     usTotalLen          = 0;    
                            u16                     usEpId              = 0;
                            EEM_HEADER_S            *pRspHeader         = NULL;
                            u8                      *pcBuff             = NULL;

                            /* get ep id */
                            usEpId = UCORE_GetEpId();

                            /* send ep online message main board */  
                            pRspHeader = EEM_CreateRespHeader(pHeader, UCORE_ERR_SUCCESS);
                        	if (NULL != pRspHeader)
                        	{
                                /* apend info */
                            	ucResult = EEM_AppendPayload(&pRspHeader, EEM_PAYLOAD_TYPE_EP_ID, sizeof(u16), (void *) &(usEpId));
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
                    case EEM_COMMAND_QUERY_EPINFO:
                        {
                            u16                     usTotalLen          = 0;    
                            EEM_HEADER_S            *pRspHeader         = NULL;
                            u8                      *pcBuff             = NULL;
                            EEM_EP_INFO_S           stSelfInfo          = {0};

                            /* prepare ep info */                                                        

                            stSelfInfo.ucEpId     = UCORE_GetEpId();
#if defined(ZDO_COORDINATOR)
                            stSelfInfo.ucEpType   = 2;    /* coord device */
#elif defined(RTR_NWK)
                            stSelfInfo.ucEpType   = 1;    /* router device */
#else
                            stSelfInfo.ucEpType   = 0;    /* end device */
#endif                             
                            stSelfInfo.usEpAddr   = NLME_GetShortAddr();

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
                    case EEM_COMMAND_TRANS_COM_WRITE:
                        {
                            uint16          usPayLen    = 0;
                            u16             usTotalLen  = 0;
                            u8              *pcBuff     = NULL;                            
                            uint16          *pEpAddr    = NULL;
                            void            *pRawData   = NULL;
                            
                            /* here get message payload */
                            pEpAddr  = (uint16 *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_ADDR, &usPayLen);
                            pRawData = EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, &usPayLen);
                            
                            /* prepare get buffer to trans to target device */
                            pcBuff = EEM_GetBuff(pHeader, &usTotalLen);

                            /* check values */
                            if ((NULL != pEpAddr) && (NULL != pRawData) && (NULL != pcBuff))
                            {                                                                
                                /* send data, otherwise start a timer to retrans*/
                                if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(*pEpAddr,
                                                                        ZP_SA_DRAGONBALL_ENDPOINT, 
                                                                        &gEndPointDesc, 
                                                                        ZP_SA_CLUSTER_COMMAND_REQ, 
                                                                        usTotalLen, pcBuff,
                                                                        AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )                                                                        
                                {
                                    /* oops, maybe resend? */
                                    break;
                                }
                            }
                        }
                        break;
                    case EEM_COMMAND_REPORT_GPS_DATA:
                        {
                            u16                 usTotalLen  = 0;
                            u8                  *pcBuff     = NULL;

#if !ZDO_COORDINATOR /* 如果不是协调器，需要检查注册状态 */

                            if (UCORE_REGIST_REGITED != UCORE_GetRegistStatus())
                            {
                                break;
                            }

#endif
                            /* broadcast this message */
                        	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                            if (NULL != pcBuff)
                            {
                                /* just broadcast data, maybe rough, but will be rewrite later  */
                                if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(UCORE_BROADCAST_ADDR,
                                                                        ZP_SA_DRAGONBALL_ENDPOINT,
                                                                        &gEndPointDesc, 
                                                                        ZP_SA_CLUSTER_COMMAND_REQ, 
                                                                        usTotalLen, pcBuff,
                                                                        AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
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

uint16 UCore_KeepAliveEventProc(void *para)
{
    uint16 nwkAddr = 0;
    
    if (gKeepAliveReset >= 3)
    {
        SystemReset();
    }

    /* get self short addr */
    nwkAddr = NLME_GetShortAddr();    
    
    if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(0, /* send to coordinator */
                                            ZP_SA_DRAGONBALL_ENDPOINT, 
                                            &gEndPointDesc, 
                                            ZP_SA_CLUSTER_KEEP_ALIVE, 
                                            sizeof(uint16), (uint8 *) &nwkAddr,
                                            AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
    {
        /* oops, maybe resend? */
    }

    /* increase reset counter */
    gKeepAliveReset++;

    /* start a keep alive timer */
    osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_KEEP_ALIVE, UCORE_APP_EVENT_KEEPALIVE_TIME );

    return UCORE_ERR_SUCCESS;
}

uint32 gTxBytesCount = 0;

#pragma optimize=none
uint16 UCore_TestSpeedEventProc(void *para)
{    
    if ( UCORE_ERR_SUCCESS == UCORE_AF_Send(0, /* send to coordinator */
                                            ZP_SA_DRAGONBALL_ENDPOINT, 
                                            &gEndPointDesc, 
                                            ZP_SA_CLUSTER_TEST_SPEED, 
                                            80, gSerialTxBuf,
                                            AF_TX_OPTIONS_NONE, AF_DEFAULT_RADIUS) )
    {
        gTxBytesCount += 80;

#if defined ( LCD_SUPPORTED )
        sprintf(gsLCDWriteBuf, "TX:%u/s", gTxBytesCount / (uint32) osal_getClock());    
        HalLcdWriteString(gsLCDWriteBuf, HAL_LCD_LINE_4 );
#endif
    }

    /* start again */
    osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_TEST_SPEED, UCORE_APP_EVENT_TEST_SPEED_SEND_INTER_TIME ); 

    return UCORE_ERR_SUCCESS;
}

