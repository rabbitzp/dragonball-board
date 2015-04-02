/*
*********************************************************************************************************
*
*                                          user core command processor 
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore_cmd.c
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

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
#ifdef ZDO_COORDINATOR
uint8               gucMaxEpid  = 0;
#endif

/*********************************************************************
 * Extern VARIABLES
 */
extern endPointDesc_t gEndPointDesc;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void SerialApp_CommandReq(uint8* , uint16);
static void SerialApp_DeviceConnectRsp(uint8* , uint16);       
static void SerialApp_ConnectReqProcess(uint8* , uint16); 


 /*********************************************************************
 * @fn      UCORE_ProcessMSGCmd
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
#pragma optimize=none
void UCORE_ProcessMSGCmd( afIncomingMSGPacket_t *pkt )
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
        case ZP_SA_CLUSTER_CONNECT_REQ:
        {
            SerialApp_ConnectReqProcess((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        case ZP_SA_CLUSTER_CONNECT_RSP:
        {
            SerialApp_DeviceConnectRsp((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }        
        case ZP_SA_CLUSTER_COMMAND_REQ:
        {
            SerialApp_CommandReq((uint8*)pkt->cmd.Data, pkt->cmd.DataLength);
            break;
        }
        case ZP_SA_CLUSTER_KEEP_ALIVE:
        {
#if defined(ZDO_COORDINATOR)    /* coordinator send ack, and other devices clear reset counter */
            /* here we just send back message */
            if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(pkt->srcAddr.addr.shortAddr, 
                                                    ZP_SA_DRAGONBALL_ENDPOINT,
                                                    &gEndPointDesc, 
                                                    ZP_SA_CLUSTER_KEEP_ALIVE, 
                                                    pkt->cmd.DataLength, pkt->cmd.Data,
                                                    AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
            {
                /* oops */
            }
#else
            gKeepAliveReset = 0;
#endif                       
            break;            
        }
        default:
        {
            break;
        }
    }
}

#pragma optimize=none
void SerialApp_CommandReq(uint8 *pcBuf, uint16 usLen)
{
    uint8                   stat                = 0;
    uint8                   ucResult            = 0;
    u16                     usTotalLen          = 0;
    u8                      *pcEmOutBuff        = NULL;
    EEM_HEADER_S            *pHeader            = NULL;        

    /* check para */
    UCORE_PARA_ASSERT_NULL(pcBuf);
    UCORE_PARA_ASSERT(0 == usLen);

    /* parse message in buffer */
    ucResult = EEM_GetMessage(pcBuf, (uint32 *) &usLen, &pHeader);
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        return;
    }

    /* judge command */
    switch (pHeader->usCommand)
    {
        case EEM_COMMAND_SET_PANDID:
        {
#if defined(ZDO_COORDINATOR) /* when coordinator recv ack, then do change panid and reboot self */

            if (INVALID_PAN_ID != gChangedPanId)
            {                
                /* wait a while, 1 second(s)  
                *  此处的设计意图为在收到至少一个终端回应后，才启动定时器修改自身的PANID，
                *  避免因网络延迟造成其它终端无法收到协调器发出的修改PANID指令。
                */
                osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_CHANGE_PANID, 5000 );
            }
            
#else/* only endpoint or router support this command */ 
            uint16          usPayLen    = 0;
            uint16          *pPanId     = NULL;
            EEM_HEADER_S    *pRespHead  = NULL;
            u8              *pcEmOut    = NULL;
            
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

            /* send ack to coordinator */
            pRespHead = EEM_CreateRespHeader(pHeader, UCORE_ERR_SUCCESS);
            if (NULL != pRespHead)
            {
                /* get buff */
            	pcEmOut = EEM_GetBuff(pRespHead, &usTotalLen);
                if (NULL != pcEmOut)
                {
                    /* send to coordinator to regist */
                    if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(0, /* 0 is coordinator */
                                                            ZP_SA_DRAGONBALL_ENDPOINT,
                                                            &gEndPointDesc, 
                                                            ZP_SA_CLUSTER_COMMAND_REQ, 
                                                            usTotalLen, pcEmOut,
                                                            AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
                    {
                    }  
                }
                
                EEM_Delete((void **) &pRespHead);
            }            
            gChangedPanId = *pPanId;

            /* wait a while, 1 second(s)  */
            osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_CHANGE_PANID, 1000 );
#endif            
            break;
        }
        case EEM_COMMAND_REPORT_EP:
        {
            EEM_HEADER_S *pHdr = NULL;
            
            /* 所有设备都向总控器声明是自己上线 */
        	pHdr = EEM_CreateHeader(UCORE_GetEpId(), EEM_COMMAND_COOR_STATCHAG, UCORE_ERR_SUCCESS);
        	if (NULL != pHdr)
        	{
            	ucResult = EEM_AppendPayload(&pHdr, EEM_PAYLOAD_TYPE_COORD_NWK_ID, sizeof(u16), (void *) &(_NIB.nwkPanId));
                if (UCORE_ERR_SUCCESS != ucResult)
                {
                    /* clean buffer */
                    EEM_Delete((void **) &pHdr);
                    break;
                }

                /* get buffer and send */
        		pcEmOutBuff = EEM_GetBuff(pHdr, &usTotalLen);
                if (NULL != pcEmOutBuff)
                {
                    /* send to uart */
                    HalUARTWrite(0, (uint8 *) pcEmOutBuff, usTotalLen);
                }
                
                /* clean buffer */
                EEM_Delete((void **) &pHdr);
            } 
            
            /* start to regist */
            osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST );
            break;
        }
        case EEM_COMMAND_TRANS_COM_READ:
        {
            /* get buff */
        	pcEmOutBuff = EEM_GetBuff(pHeader, &usTotalLen);
            if (NULL != pcEmOutBuff)
            {
                if ( HalUARTWrite( UCORE_EX_COMM_UART_PORT, pcEmOutBuff, usTotalLen ) )
                {               
                  // Save for next incoming message
                  
                  stat = UCORE_ERR_SUCCESS;
                }
                else
                {
                  stat = UCORE_RESULT_OTA_SER_BUSY;
                }
            }           
            
            break;            
        }
        /* trans command directly send to remote */
        case EEM_COMMAND_TRANS_COM_WRITE:
        {
            uint16  usPayLen    = 0;
            void    *pRawData   = NULL;
            
            /* here get message payload */
            pRawData = EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, &usPayLen);
            if (NULL != pRawData)
            {
                /* save trans id first, later send data will bee write back */
                gusCurTransactionId = pHeader->usTransId;
                
                /* send to uart 0 */
                if ( !HalUARTWrite2( UCORE_EX_COMM_UART_PORT, pRawData, usPayLen) )
                {
                    //better to start a timer retrying...
                }
            }
            break;
        }
        case EEM_COMMAND_REPORT_GPS_DATA:
        {
            /* get buff */
        	pcEmOutBuff = EEM_GetBuff(pHeader, &usTotalLen);
            if (NULL != pcEmOutBuff)
            {
                /* send to uart 0 */
                HalUARTWrite(UCORE_EX_COMM_UART_PORT, pcEmOutBuff, usTotalLen);
            }
            break;            
        }
        default:
        {
            break;
        }
    }

    /* free buffer */
    EEM_Delete((void **) &pHeader);
}

#pragma optimize=none
void SerialApp_ConnectReqProcess(uint8 *pcBuf, uint16 usLen)
{
#ifdef ZDO_COORDINATOR
    uint8                       ucRet               = 0;
    u16                         usTotalLen          = 0; 
    u16                         usPayLen            = 0;
    uint8                       ucAsignEpId         = 0;
    uint8                       ucAsignEpType       = 0; 
    u16                         usEpShortAddr       = 0;    
    UCORE_REGIST_INFO_S         *pstRegistInfo      = NULL;    
    EEM_HEADER_S                *pHeader            = NULL;
    EEM_HEADER_S                *pRspHeader         = NULL;    
    u8                          *pcBuff             = NULL;
    UCORE_NV_ITEM_MAX_EPID_S    stNvMaxEpid         = {0};    
    EEM_EP_INFO_S               stEpInfo            = {0};
    
    /* check para */
    UCORE_PARA_ASSERT_NULL(pcBuf);
    UCORE_PARA_ASSERT(0 == usLen);

    /* parse message in buffer */
    ucRet = EEM_GetMessage(pcBuf, (uint32 *) &usLen, &pHeader);
    if (UCORE_ERR_SUCCESS != ucRet)
    {
        return;
    }

    /* check command */
    if (EEM_COMMAND_REGIST_EP != pHeader->usCommand)
    {
        /* free buffer */
        EEM_Delete((void **) &pHeader);    

        return;
    }

    /* get regist info */
    pstRegistInfo  = (UCORE_REGIST_INFO_S *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_REGIST_INFO, &usPayLen);
    if (NULL == pstRegistInfo)
    {
        /* free buffer */
        EEM_Delete((void **) &pHeader);    

        return;    
    }

    /* set ep addr */
    usEpShortAddr   = pstRegistInfo->usShortAddr;

    /* save epid */
    ucAsignEpId     = pstRegistInfo->ucEpid;
    ucAsignEpType   = pstRegistInfo->ucEpType;

    /* create response message here */
    if (UCORE_INVALID_EPID == ucAsignEpId)
    {
        pRspHeader = EEM_CreateRespHeader(pHeader, UCORE_RESULT_INVALID_EPDEV);    
    }
    else
    {
        pRspHeader = EEM_CreateRespHeader(pHeader, UCORE_ERR_SUCCESS); 

        /* set epid */
        pRspHeader->ucEpId = ucAsignEpId;
    }

    /* free previous buff first */
    EEM_Delete((void **) &pHeader);

    /* check this buffer */
    if (NULL != pRspHeader)
    {
        /* get buff */
    	pcBuff = EEM_GetBuff(pRspHeader, &usTotalLen);
        if (NULL == pcBuff)
        {
            /* clean buffer */
            EEM_Delete((void **) &pRspHeader);        
            return;      
        }
        
        if ( UCORE_ERR_SUCCESS != UCORE_AF_Send(usEpShortAddr, 
                                                ZP_SA_DRAGONBALL_ENDPOINT,
                                                &gEndPointDesc, 
                                                ZP_SA_CLUSTER_CONNECT_RSP, 
                                                usTotalLen, pcBuff,
                                                AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )
        {
            /* clean buffer */
            EEM_Delete((void **) &pRspHeader); 
            
            // Error occurred in request to send.
            UCORE_Debug("AF send error!\r\n");
            return;    
        }

        /* clean buffer first */
        EEM_Delete((void **) &pRspHeader);
    }

    /* 如果是非法设备ID，到这里就返回了 */
    if (UCORE_INVALID_EPID == ucAsignEpId)
    {
        return;
    }

    /* send ep online message main board */    
	pHeader = EEM_CreateHeader(ucAsignEpId, EEM_COMMAND_EP_ONLINE, UCORE_ERR_SUCCESS);
	if (NULL != pHeader)
	{
        /* prepare ep info */
        stEpInfo.ucEpId     = ucAsignEpId;
        stEpInfo.ucEpType   = ucAsignEpType;
        stEpInfo.usEpAddr   = usEpShortAddr;

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
    }
#endif    

    return;
}

#pragma optimize=none/* 解决断点跳跃的问题，告诉编译器不要对该函数优化，否则断点可能会偏移 */
void SerialApp_DeviceConnectRsp(uint8 *pcBuf, uint16 usLen)
{
#if !ZDO_COORDINATOR
    uint8                       ucResult            = 0;
    EEM_HEADER_S                *pHeader            = NULL;        

    /* check para */
    UCORE_PARA_ASSERT_NULL(pcBuf);
    UCORE_PARA_ASSERT(0 == usLen);

    /* parse message in buffer */
    ucResult = EEM_GetMessage(pcBuf, (uint32 *) &usLen, &pHeader);
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        /* set a timer to regist again */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST ); 
    
        return;
    }

    if (EEM_COMMAND_REGIST_EP != pHeader->usCommand)
    {
       /* clean buffer */
        EEM_Delete((void **) &pHeader);
       
        /* set a timer to regist again */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST ); 
    
        return;
    }
    
    /* check if regist success */
    if (UCORE_ERR_SUCCESS != pHeader->ucResult)
    {
       /* clean buffer */
        EEM_Delete((void **) &pHeader);
       
        /* set a timer to regist again */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST ); 
        
        return;
    }
    
    /* stop timer first */
    osal_stop_timerEx(UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST);
    
    /* set regist status */
    UCORE_SetRegistStatus(UCORE_REGIST_REGITED);

    sprintf(gsLCDWriteBuf, "EPID:%d", UCORE_GetEpId());    
    HalLcdWriteString(gsLCDWriteBuf, HAL_LCD_LINE_4 );   

   /* clean buffer */
    EEM_Delete((void **) &pHeader);
   
#endif
}

