/*
*********************************************************************************************************
*
*                                          user core 
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore.c
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

#include "ZProfile\zprofile_def.h"

/*include user inc */
#include "ucore_defs.h"
#include "ucore_struct.h"
#include "ucore.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint16                          gusCurTransactionId = 0;
uint16                          gChangedPanId = INVALID_PAN_ID;
uint8                           gKeepAliveReset = 0;

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8                    gucEpid             = UCORE_INVALID_EPID;
static uint8                    gucRegistStatus     = UCORE_REGIST_UNREGISTED;
static devStates_t              gemNwkState         = DEV_INIT;   
static uint8                    gucTaskID           = 0;


UCORE_EVENT_PROCESSOR_S gCoreEventProcessor[UCORE_MAX_EVENT_PROCESSOR] = 
{
    {SYS_EVENT_MSG,                         UCore_SysEventProc},
    {UCORE_APP_EVENT_ID_REGIST,             UCore_RegistEventProc},
    {UCORE_APP_EVENT_ID_CHANGE_PANID,       UCore_ChangePanIdEventProc},
    {UCORE_APP_EVENT_ID_REBOOT_DEVICE,      UCore_RebootEventProc},
    {UCORE_APP_EVENT_ID_KEEP_ALIVE,         UCore_KeepAliveEventProc},
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void UCORE_Debug(const char *buf, ...)
{
    return;
} 

/* stack init */
void UCORE_Init(uint8 ucTaskId)
{
#if defined(ZDO_COORDINATOR)
    /* set cur epid */
    UCORE_SetEpId(0);  
#else
    uint8                       ucRet       = 0;
    UCORE_NV_ITEM_CUR_EPID_S    stNvCurEpid = {0};

    /* init nv for epid config */
    ucRet = osal_nv_item_init(UCORE_NV_CONFIG_ITEM_CUR_EP_ID, sizeof(UCORE_NV_ITEM_CUR_EPID_S), NULL);
    if ((SUCCESS == ucRet) || (NV_ITEM_UNINIT == ucRet))
    {
        /* read self epid */
        ucRet = osal_nv_read(UCORE_NV_CONFIG_ITEM_CUR_EP_ID, 0, sizeof(UCORE_NV_ITEM_CUR_EPID_S), (void *) &stNvCurEpid);
        if (ZSUCCESS == ucRet)
        {
            /* set cur epid */
            UCORE_SetEpId(stNvCurEpid.ucEpid);
        }                
    }
#endif

    /* set task id */
    UCORE_SetTaskID(ucTaskId);

    return;
}

void UCORE_SetRegistStatus(IN uint8 ucRegistStatus)
{
    gucRegistStatus = ucRegistStatus;
}

uint8 UCORE_GetRegistStatus(void)
{
    return gucRegistStatus;
}

void UCORE_SetEpId(IN uint8 ucEpid)
{
    UCORE_NV_ITEM_CUR_EPID_S stCurEpid = {0};
    
    gucEpid = ucEpid;

    /* write to flash */
    stCurEpid.ucEpid = ucEpid;    
    (void) osal_nv_write(UCORE_NV_CONFIG_ITEM_CUR_EP_ID, 0, sizeof(UCORE_NV_ITEM_CUR_EPID_S), (void *) &stCurEpid);
}

uint8 UCORE_GetEpId(void)
{
    return gucEpid;
}

void UCORE_SetTaskID(uint8 ucTaskId)
{
    gucTaskID = ucTaskId;
}

uint8 UCORE_GetTaskID(void)
{
    return gucTaskID;
}

void UCORE_SetNetworkState(devStates_t emNwkState)
{
    gemNwkState = emNwkState;
}

devStates_t UCORE_GetNetworkState(void)
{
    return gemNwkState;
}

uint8 UCORE_AF_Send(uint16 usEpAddr, uint8 ucDestEndPoint, endPointDesc_t *pSrcEP, uint16 usClusterID, uint16 usDatalen, uint8 *pBuf, uint8 ucOptions, uint8 ucRadius)
{
    uint8        ucRet     = 0;
    uint8        i         = 0;
    uint8        ucTransID = 0;
    afAddrType_t stAfAddr  = {0};

    /* check para */
    UCORE_PARA_ASSERT_RETURN(0 == usDatalen, UCORE_ERR_PAPA_ERROR);
    UCORE_PARA_ASSERT_RETURN(255 < usDatalen, UCORE_ERR_PAPA_ERROR);    
    UCORE_PARA_ASSERT_RETURN(NULL == pBuf, UCORE_ERR_PAPA_ERROR);
    UCORE_PARA_ASSERT_RETURN(NULL == pSrcEP, UCORE_ERR_PAPA_ERROR);

    if (UCORE_BROADCAST_ADDR == usEpAddr)
    {
        stAfAddr.addrMode        = (afAddrMode_t)AddrBroadcast;
        stAfAddr.endPoint        = ucDestEndPoint;
        stAfAddr.addr.shortAddr  = UCORE_BROADCAST_ADDR;
    }
    else
    {
        /* set ep addr */
        stAfAddr.addrMode       = (afAddrMode_t)Addr16Bit;
        stAfAddr.endPoint       = ucDestEndPoint;
        stAfAddr.addr.shortAddr = usEpAddr;
    }
    
    if (afStatus_SUCCESS != AF_DataRequest(&stAfAddr, pSrcEP, usClusterID, usDatalen, pBuf, &ucTransID, ucOptions, ucRadius))
    {
        /* retry UCORE_AF_SEND_MAX_RETRY_TIMES times, if still failed, then discuss this message */
        if (AF_ACK_REQUEST == ucOptions)
        {
            for (i = 0; i < UCORE_AF_SEND_MAX_RETRY_TIMES; i++)
            {
                ucRet = AF_DataRequest(&stAfAddr, pSrcEP, usClusterID, usDatalen, pBuf, &ucTransID, ucOptions, ucRadius);
                if (afStatus_SUCCESS == ucRet)
                {
                    return UCORE_ERR_SUCCESS;
                }
            }
        }
        
        return UCORE_RESULT_AF_SEND_ERROR;
    }
        
    return UCORE_ERR_SUCCESS;
}

