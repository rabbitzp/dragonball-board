/*********************************************************************
 * INCLUDES
 */

#include <stdio.h>
#include <string.h>
#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"

/*include jstack inc */
#include "jstack_defs.h"
#include "jstack_structs.h"
#include "jstack_func.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8                    gucEpid             = JMI_INVALID_EPID;
static uint16                   gusSeq              = 0;
static uint8                    gucRegistStatus     = JMI_REGIST_UNREGISTED;

/* stack init */
void JMI_Init(void)
{
    gusSeq = 0;

    /* here should read nv, get epid */
}

void JMI_SetRegistStatus(uint8 ucRegistStatus)
{
    gucRegistStatus = ucRegistStatus;
}

uint8 JMI_GetRegistStatus(void)
{
    return gucRegistStatus;
}

/* global functions */
void JMI_FillMessageReqHeader(JMI_MSG_HEADER_S *pstHeader, uint8 ucCommand, uint16 uslen)
{
    JMI_PARA_ASSERT(pstHeader);
    
    pstHeader->ucMagic      = 0xFF;
    pstHeader->ucEPId       = gucEpid;
    pstHeader->usSeq        = ++gusSeq;
    pstHeader->ucCommand    = ucCommand;
    pstHeader->ucResult     = JMI_RESULT_SUCCESS;
    pstHeader->usDataLen    = uslen;
}

/* global functions */
void JMI_FillMessageRspHeader(JMI_MSG_HEADER_S *pstHeader, uint8 ucEpid, uint16 usSeq, uint8 ucCommand, uint8 ucResult, uint16 uslen)
{
    JMI_PARA_ASSERT(pstHeader);
    
    pstHeader->ucMagic      = 0xFF;
    pstHeader->ucEPId       = ucEpid;
    pstHeader->usSeq        = usSeq;
    pstHeader->ucCommand    = ucCommand;
    pstHeader->ucResult     = ucResult;
    pstHeader->usDataLen    = uslen;
}

void JMI_FillMessageBody(JMI_MSG_HEADER_S *pstHeader, uint16 usBodyLen, void *pData)
{
    char *pBody = NULL;
    
    JMI_PARA_ASSERT(pstHeader);
    JMI_PARA_ASSERT(pData);

    /* position */
    pBody = (char *) pstHeader + JMI_MSG_HEADER_LEN;

    /* copy buffer */
    osal_memcpy(pBody, pData, usBodyLen);
}

bool JMI_ValidateMessage(uint8 *pcBuff, uint16 usLen)
{
    JMI_PARA_ASSERT_RETURN(pcBuff, false);

    /* check msg len first */
    if (JMI_MSG_HEADER_LEN > usLen)
    {
        return false;
    }

    JMI_MSG_HEADER_S *pstHeader = (JMI_MSG_HEADER_S *) pcBuff;

    /* check header */
    if (0xFF == pstHeader->ucMagic)
    {
        return true;
    }

    return false;
}

uint16 JMI_GetCurrentSeq(void)
{
    return gusSeq;
}

uint8 JMI_MakeRegistInfo(IN uint16 usShortAddr, IN uint8 *pcBuff, INOUT uint16 *usMaxBuffLen)
{
    JMI_MSG_HEADER_S        *pstHeader      = NULL;
    JMI_REGIST_INFO_S       *pstRegistInfo  = NULL;
    
    JMI_PARA_ASSERT_RETURN(pcBuff, JMI_RESULT_INVALID_PARA);
    JMI_PARA_ASSERT_RETURN(usMaxBuffLen, JMI_RESULT_INVALID_PARA);


    // Build & send OTA response message.
    pstHeader = (JMI_MSG_HEADER_S *) pcBuff;

    uint8 ucDataLen = JMI_MSG_STRUCT_DATA_LEN(JMI_REGIST_INFO_S);

    /* fill msg header */
    JMI_FillMessageReqHeader(pstHeader, JMI_COMMAND_REGIST, ucDataLen);

    pstRegistInfo = (JMI_REGIST_INFO_S *) pcBuff;

    /* set regist info */
    pstRegistInfo->usShortAddr = usShortAddr;
    pstRegistInfo->ucEpid = gucEpid;
#if defined(ZDO_COORDINATOR)
    pstRegistInfo->ucEpType = 2;    /* coord device */
#elif defined(RTR_NWK)
    pstRegistInfo->ucEpType = 1;    /* router device */
#else
    pstRegistInfo->ucEpType = 0;    /* end device */
#endif    

    /* return new buff len */
    *usMaxBuffLen = sizeof(JMI_REGIST_INFO_S);

    return JMI_RESULT_SUCCESS;
}

void JMI_SetEpId(IN uint8 ucEpid)
{
    gucEpid = ucEpid;
}

uint8 JMI_GetEpId(void)
{
    return gucEpid;
}

void JMIDebug(char *buf)
{
    /* check para */
    if (NULL == buf)
    {
        return;
    }

    /* uart 0 when ep mode, is using by 485, not a debug interface */
#ifdef ZDO_COORDINATOR    
    HalUARTWrite(0, (uint8 *) buf, strlen(buf));
#endif
}


/* debug functions */


