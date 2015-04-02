/*
*********************************************************************************************************
*
*                                          user core event processor
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore_event.c
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

/*********************************************************************
 * Extern VARIABLES
 */
extern endPointDesc_t gEndPointDesc;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
uint16 UCore_SysEventProc(void *para)
{
    uint8                       ucTaskId    = 0;
    afIncomingMSGPacket_t       *MSGpkt     = NULL;

    /* check para */
    UCORE_PARA_ASSERT_NULL_RETURN(para, UCORE_ERR_PAPA_ERROR);

    ucTaskId = *((uint8 *) para);

    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( ucTaskId )) )
    {
      switch ( MSGpkt->hdr.event )
      {
      case AF_INCOMING_MSG_CMD:        
        UCORE_ProcessMSGCmd( MSGpkt );
        break;
        
      case ZDO_STATE_CHANGE:        
        /* save network state */
        UCORE_SetNetworkState((devStates_t)(MSGpkt->hdr.status));
        
        if (    ((devStates_t)(MSGpkt->hdr.status) == DEV_ZB_COORD)
            ||  ((devStates_t)(MSGpkt->hdr.status) == DEV_ROUTER)
            ||  ((devStates_t)(MSGpkt->hdr.status) == DEV_END_DEVICE) )
        {
            // Start sending the periodic message in a regular interval.
            //HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);

            u8              ret         = 0;
            u16             usTotalLen  = 0;    
            EEM_HEADER_S    *pHeader    = NULL;
            u8              *pcBuff     = NULL;

#if ZDO_COORDINATOR /* 如果是协调器，则向网络中所有采集终端发送上线通知 */            
            pHeader = EEM_CreateHeader(0, EEM_COMMAND_REPORT_EP, UCORE_ERR_SUCCESS);
            if (NULL != pHeader)
        	{
                /* get buffer and send */
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
                    }
                }    

                /* clean buffer */
                EEM_Delete((void **) &pHeader);                
            }
#endif
            /* 所有设备都向总控器声明是自己上线 */
        	pHeader = EEM_CreateHeader(0, EEM_COMMAND_COOR_STATCHAG, UCORE_ERR_SUCCESS);
        	if (NULL != pHeader)
        	{
            	ret = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_COORD_NWK_ID, sizeof(u16), (void *) &(_NIB.nwkPanId));
                if (UCORE_ERR_SUCCESS != ret)
                {
                    /* clean buffer */
                    EEM_Delete((void **) &pHeader);
                    break;
                }

                /* get buffer and send */
        		pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                if (NULL != pcBuff)
                {
                    /* send to uart */
                    HalUARTWrite(0, (uint8 *) pcBuff, usTotalLen);
                }
                
                /* clean buffer */
                EEM_Delete((void **) &pHeader);
            } 
            
#if !defined(ZDO_COORDINATOR)   /* 非协调器类还应启动注册 */
        /* set regist status */
        UCORE_SetRegistStatus(UCORE_REGIST_UNREGISTED);

        /* start to regist */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST ); 
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
    
    return UCORE_ERR_SUCCESS;
}

uint16  UCore_RegistEventProc(void *para)
{
#if !ZDO_COORDINATOR
    uint8                   ucTaskId                = 0;
    uint8                   ucRet                   = 0;
    uint16                  nwkAddr                 = 0;
    uint16                  usTotalLen              = 0; 
    uint8                   *pcEmOut                = NULL;
    EEM_HEADER_S            *pHeader                = NULL;
    UCORE_REGIST_INFO_S     stRegistInfo            = {0};

    /* check para */
    UCORE_PARA_ASSERT_NULL_RETURN(para, UCORE_ERR_PAPA_ERROR);

    ucTaskId = *((uint8 *) para);    

    /* check epid valid? */
    if (UCORE_INVALID_EPID == UCORE_GetEpId())
    {
        /* reset timer */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST );     

        return UCORE_ERR_PAPA_ERROR;
    }
    
    /* send ep online message main board */    
	pHeader = EEM_CreateHeader(UCORE_GetEpId(), EEM_COMMAND_REGIST_EP, UCORE_ERR_SUCCESS);
	if (NULL == pHeader)
	{
        /* reset timer */
        osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST );     
	
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* get self and coordi short addr */
    nwkAddr = NLME_GetShortAddr();
    
    /* parentNwkAddr = NLME_GetCoordShortAddr(); */

    /* prepare regist info */
    stRegistInfo.usShortAddr    = nwkAddr;
    stRegistInfo.ucEpid         = UCORE_GetEpId();
#if defined(ZDO_COORDINATOR)
    stRegistInfo.ucEpType       = 2;    /* coord device */
#elif defined(RTR_NWK)
    stRegistInfo.ucEpType       = 1;    /* router device */
#else
    stRegistInfo.ucEpType       = 0;    /* end device */
#endif 

    /* apend info */
	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_REGIST_INFO, sizeof(UCORE_REGIST_INFO_S), (void *) &(stRegistInfo));
    if (UCORE_ERR_SUCCESS != ucRet)
    {
        /* clean buffer */
        EEM_Delete((void **) &pHeader);        
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* get buff */
	pcEmOut = EEM_GetBuff(pHeader, &usTotalLen);
    if (NULL == pcEmOut)
    {
        /* clean buffer */
        EEM_Delete((void **) &pHeader);        
        return UCORE_ERR_MEM_ALLOC_FAILED;    
    }

    /* send to coordinator to regist */
    if ( UCORE_ERR_SUCCESS == UCORE_AF_Send(0, /* 0 is coordinator */
                                            ZP_SA_DRAGONBALL_ENDPOINT,
                                            &gEndPointDesc, 
                                            ZP_SA_CLUSTER_CONNECT_REQ, 
                                            usTotalLen, pcEmOut,
                                            AF_ACK_REQUEST, AF_DEFAULT_RADIUS) )                                            
    {
        UCORE_SetRegistStatus(UCORE_REGIST_REGISTING);
    }   

    /* clean buffer */
    EEM_Delete((void **) &pHeader);    
    
    /* start a timer to check regist result*/
    osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REGIST, UCORE_APP_EVENT_DELAY_REGIST );  
    
#endif    //!ZDO_COORDINATOR

    return UCORE_ERR_SUCCESS;
}

uint16 UCore_ChangePanIdEventProc(void *para)
{
    if (INVALID_PAN_ID == gChangedPanId)
    {
        return UCORE_ERR_SUCCESS;
    }
    
    /* then change self pandid */
    _NIB.nwkPanId = gChangedPanId;

    /* try to init */
    NLME_InitNV();

    /* update */
    NLME_UpdateNV(NWK_NV_NIB_ENABLE);

    /* delay reboot, 1 second(s) */
    osal_start_timerEx( UCORE_GetTaskID(), UCORE_APP_EVENT_ID_REBOOT_DEVICE, 1000 );    

    return UCORE_ERR_SUCCESS;
}

uint16 UCore_RebootEventProc(void *para)
{
    SystemReset(); 
}


