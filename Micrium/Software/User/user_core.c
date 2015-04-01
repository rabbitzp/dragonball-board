/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_core.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <math.h>
#include <includes.h>
#include <user_core.h>
#include <user_driver_probe.h>
#include <user_ep_manage.h>
#include <user_os_func.h>
#include <eem.h>
#include <eem_struct.h>
#include "nmea_parser.h"

#include <user_task_key.h>

#include <user_task_console.h>

#include <tfs.h>

#if (DEF_ENABLED == OS_USER_HMC_SUPPORT)
#include <user_task_hmc.h>
#endif

#include <user_task_uart.h>

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
#include <user_task_gps.h>
#endif

#if (DEF_ENABLED == OS_USER_LCD_SUPPORT)
#include "GUI_WndDef.h"     /* valid LCD configuration */
#include "GUI_WndMain.h"
#endif

#include <user_debug.h>

#define DEFAULT_USER_TASK_PRIO              5  //优先级高
#define DEFAULT_USER_STK_SIZE               128
#define MAX_USER_CORE_MSG_SIZE              16

extern EP_INFO_S g_stMyEpInfo;

/*----------------local global vars declare here---------------------*/
OS_STK               g_UserCoreSTK[DEFAULT_USER_STK_SIZE];

void                *g_UserCoreMsgQueue[MAX_USER_CORE_MSG_SIZE];
OS_EVENT            *g_QSemCoreMsg = NULL;

UDEV_CONFIG_S       g_udev_config;

/* static local global functions here */
static void UCore_TaskProc(void *p_arg);
static void UCoreInit(void);
static void UCore_EventLoop(void);

u8 UCore_Start(void)
{    
    UCORE_DEBUG_PRINT("starting user core....\r\n");

    if (OS_ERR_NONE == OSTaskCreate(UCore_TaskProc, (void *)0, (OS_STK *)&g_UserCoreSTK[DEFAULT_USER_STK_SIZE - 1], DEFAULT_USER_TASK_PRIO))
    {
        return UCORE_ERR_SUCCESS;
    }

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void UCore_TaskProc(void *p_arg)
{
    u8 ucResult = UCORE_ERR_COMMON_FAILED;
    
    UCORE_DEBUG_PRINT("User Core Task Start.\r\n");

    /* for console print */
    g_udev_config.enDebug = 1;

    /* console init first */
    ucResult = UConsole_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Init console failed.\r\n");
    }
    
    /* probe driver first */
    ucResult = User_DriverProbe();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("User_Driver Probe failed.\r\n");
    } 
    
    /* call init */
    UCoreInit();

    /* init ep manager */
    ucResult = UEM_Init();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Init ep manager failed.\r\n");
    }    

    /* create sub task */
    ucResult = UKey_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Start user key task failed.\r\n");
    }

#if (DEF_ENABLED == OS_USER_HMC_SUPPORT)
    ucResult = UHMC_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Start user HMC task failed.\r\n");
    } 
#endif

    ucResult = UUart_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Start user uart task failed.\r\n");
    }    

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
    ucResult = GPS_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Start user gps task failed.\r\n");
    }
#endif

    /* enter event loop */
    UCore_EventLoop();

    UCORE_DEBUG_PRINT("User Core Task Exit.\r\n");
}

void UCoreInit(void)
{   
    if (TFS_RESULT_SUCESS != TFS_Read(TFS_CONFIG_ITEM_DEV_CONF, sizeof(UDEV_CONFIG_S), &g_udev_config))
    {
        g_udev_config.enDebug = 1;
    
        UCORE_DEBUG_PRINT("read dev config failed, using default config.\r\n");
    }

    /* create message queue */
    g_QSemCoreMsg = OSQCreate(&g_UserCoreMsgQueue[0], MAX_USER_CORE_MSG_SIZE);    
    
    UCORE_DEBUG_PRINT("User Core init finished.\r\n");
}

void UCore_EventLoop(void)
{
    u8              ucResult    = 0;
    INT8U           err         = 0;
    UCORE_MSG_S     *pMsg       = NULL;
    
    /* main process here */
    while (1)
    {
        pMsg = (UCORE_MSG_S *) OSQPend(g_QSemCoreMsg, 0, &err);
        if ((OS_NO_ERR == err) && (NULL != pMsg))
        {
            switch (pMsg->usMsgType)
            {
                case UCORE_MESSAGE_TYPE_QUERY_EPINFO:
                    {
                        EP_INFO_S    *pEpInfo   = NULL;
                        
                        /* get ep info */
                        pEpInfo = (EP_INFO_S *) pMsg->pBuf;
                        if (NULL != pEpInfo)
                        {
                            /* yes, i'm ready! */                        
                            memcpy(&g_stMyEpInfo, pEpInfo, sizeof(EP_INFO_S));

                            UCORE_DEBUG_PRINT("My ep info:id:%d type:%d addr:%x name:%s\r\n", g_stMyEpInfo.ucEpId, g_stMyEpInfo.ucEpType, g_stMyEpInfo.usEpAddr, g_stMyEpInfo.sEpName);
                        }
                    }
                    break;
                case UCORE_MESSAGE_TYPE_COOR_STATCHAG:
                    {
                        u16             usTotalLen  = 0;  
                        u8              *pcBuff     = NULL;
                        EEM_HEADER_S    *pHeader    = NULL;
                        u16             *pPanid     = NULL;

                        pPanid = (u16 *) pMsg->pBuf;
                        if (NULL == pPanid)
                        {
                            break;
                        }

                        /* save current pan id */
                        g_udev_config.PanId = *pPanid;
                        
                        UCORE_DEBUG_PRINT("Coordinator state change, panid:0X%02X.\r\n", g_udev_config.PanId);

                        /* query my basic info */
                    	pHeader = EEM_CreateHeader(0, EEM_COMMAND_QUERY_EPINFO, UCORE_ERR_SUCCESS);
                    	if (NULL == pHeader)
                    	{
                    	    UCORE_DEBUG_PRINT("EEM CreateHeader failed.\r\n");
                            return;
                        }
                    
                        /* get buff */
                    	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                        if (NULL != pcBuff)
                        {
                            USART_Send(USART2, usTotalLen, (void *) pcBuff);
                        }
                                                            
                        /* clean buffer */
                        EEM_Delete((void **) &pHeader);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_EP_ONLINE:
                    {
                        EP_INFO_S    *pEpInfo   = NULL;

                        /* get ep info */
                        pEpInfo = (EP_INFO_S *) pMsg->pBuf;
                        if (NULL != pEpInfo)
                        {
                            /* try find */
                            if (NULL != UEM_FindEp(pEpInfo->ucEpId))
                            {
                                /* so update ep info */
                                ucResult = UEM_UpdateEp(pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                                if (UCORE_ERR_SUCCESS != ucResult)
                                {
                                    UCORE_DEBUG_PRINT("Update ep[%d] failed.\r\n", pEpInfo->ucEpId);
                                    break;
                                }
                                
                                UCORE_DEBUG_PRINT("Update EP[%d] info, type:%d, addr:%02X, name:%s.\r\n", pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                            }
                            else/* add new ep */
                            {
                                /* add to ep list */
                                ucResult = UEM_AddEp(pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                                if (UCORE_ERR_SUCCESS != ucResult)
                                {
                                    UCORE_DEBUG_PRINT("Add ep[%d] failed.\r\n", pEpInfo->ucEpId);
                                    break;
                                }
                                
                                UCORE_DEBUG_PRINT("New EP[%d] online, type:%d, addr:%02X, name:%s.\r\n", pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                            }
                        }
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TRANS_COM_READ:
                    {
#if 0
                        u8              ucRet       = 0;
                        u16             usTotalLen  = 0;  
                        u8              *pcBuff     = NULL;
                        EEM_HEADER_S    *pHeader    = NULL;

                        pHeader = EEM_CreateHeader(pMsg->usSrcId, EEM_COMMAND_LH_QUERY_FLOW_DATA, UCORE_ERR_SUCCESS);
                    	if (NULL != pHeader)
                    	{              
                    	    /* set transaction id */
                            pHeader->usTransId = pMsg->usTransId;
                            
                        	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, pMsg->usBufLen, pMsg->pBuf);
                            if (UCORE_ERR_SUCCESS != ucRet)
                            {
                                /* clean buffer */
                                EEM_Delete((void **) &pHeader);        
                                break;
                            }

                            /* now get buffer to send */
                    		pcBuff = EEM_GetBuff(pHeader, &usTotalLen);

                            ucResult = NET_TcpSyncSend(gTCP_SOCK_TO_SERVER, (u8 *) pcBuff, usTotalLen);
                            if (UCORE_ERR_SUCCESS != ucResult)
                            {
                                UCORE_DEBUG_PRINT("Send trans data to server failed, errno:%d\r\n", ucResult);
                            }

                    		EEM_Delete((void **) &pHeader);
                    	}
#endif
                    }
                    break;
                case UCORE_MESSAGE_TYPE_REPORT_GPS:
                    {
                        EEM_EP_GPS_INFO_S   *pGpsInfo = NULL;
                        
                        /* get ep gps info */
                        pGpsInfo = (EEM_EP_GPS_INFO_S *) pMsg->pBuf;
                        if (NULL != pGpsInfo)
                        {
#if (DEF_ENABLED == OS_USER_LCD_SUPPORT)                        
                            /* 送到显示模块去 */
                            WM_MESSAGE  stUiMessage = {0};
                            nmeaINFO    stNemaInfo = {0};
                            
                            stNemaInfo.id = pGpsInfo->epid;
                        	stNemaInfo.status = pGpsInfo->status;			/**< Status (A = active or V = void) */
                        	stNemaInfo.ns = pGpsInfo->ns;				    /**< [N]orth or [S]outh */
                        	stNemaInfo.ew = pGpsInfo->ew;				    /**< [E]ast or [W]est */
                        	stNemaInfo.lat = pGpsInfo->lat;	                /**< Latitude in NDEG - [degree][min].[sec/60] */
                        	stNemaInfo.lon = pGpsInfo->lon;	                /**< Longitude in NDEG - [degree][min].[sec/60] */
                        	stNemaInfo.speed = pGpsInfo->speed;	            /**< Speed over the ground in knots */    
                            stNemaInfo.course = pGpsInfo->course;           /**< direction, 000.0~359.9, base N */
                            
                            stUiMessage.MsgId  = GUI_USER_MSG_GPS_UPDATE;   
                            stUiMessage.Data.p = (void *) &stNemaInfo;

                            /* send to radar window */
                            WM_SendMessage(GUI_GetCurrentWnd(), &stUiMessage);
#endif

                            /* trace gps info */
                            if (1 == g_udev_config.enGpsDebug)
                            {
                                UCORE_DEBUG_PRINT("Recv GPS info from[%d] >> lon:%.5f lat:%.5f\r\n", stNemaInfo.id, stNemaInfo.lon, stNemaInfo.lat); 
                            }
                        }
                    }
                    break;
                case UCORE_MESSAGE_TYPE_CHANGE_EPID:
                    {
                        u16 *pEpid = NULL;
                        
                        pEpid = (u16 *) pMsg->pBuf;
                        if (NULL != pEpid)
                        {
                            g_stMyEpInfo.ucEpId = *pEpid;

                            UCORE_DEBUG_PRINT("Change epid to:%d.\r\n", g_stMyEpInfo.ucEpId);
                        }
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TEST1:
                    {
                        UCORE_DEBUG_PRINT("Message testing 1.\r\n");
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TEST2:
                    {
                        UCORE_DEBUG_PRINT("Message testing 2.\r\n");
                    }
                    break;                    
                default:
                    UCORE_DEBUG_PRINT("Unknown message type:%d\r\n", pMsg->usMsgType);
                    break;
            }

            /* delete message */
            UCORE_DEL_MESSAGE(pMsg);
        }
    }
}

u8 UCore_PostMessage1(u16 usMessageType, u16 usBufLen, void *pBuf)
{
    UCORE_MSG_S *pMsg       = NULL;

    /* alloc message buf */
    pMsg = malloc(sizeof(UCORE_MSG_S));
    if (NULL == pMsg)
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    memset(pMsg, 0, sizeof(UCORE_MSG_S));
    pMsg->usMsgType = usMessageType;
    pMsg->usBufLen  = usBufLen;
    pMsg->usSrcId = 0;
    pMsg->usDstId = 0;
    pMsg->usSequence = 0;
    pMsg->usTransId = 0;
    
    if (0 != usBufLen)
    {
        /* check exceed max buff size? */
        if (UCORE_MAX_MESSAGE_BUFF_LEN <= usBufLen)
        {
            free(pMsg);
            return UCORE_ERR_MEM_ALLOC_FAILED;
        }

        /* alloc user buff */
        pMsg->pBuf = malloc(usBufLen);

        /* copy buff */
        memcpy(pMsg->pBuf, pBuf, usBufLen);
    }

    /* ok, now post this message */
    if (OS_ERR_NONE != OSQPost(g_QSemCoreMsg, (void *)pMsg))
    {
        /* free message */
        if (pMsg->pBuf) free(pMsg->pBuf);
        if (pMsg) free(pMsg);
        return UCORE_ERR_MSG_POST_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}

u8 UCore_PostMessage2(u16 usMessageType, u16 usBufLen, void *pBuf, u16 srcId, u16 dstId)
{
    UCORE_MSG_S *pMsg       = NULL;

    /* alloc message buf */
    pMsg = malloc(sizeof(UCORE_MSG_S));
    if (NULL == pMsg)
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    memset(pMsg, 0, sizeof(UCORE_MSG_S));
    pMsg->usMsgType = usMessageType;
    pMsg->usBufLen  = usBufLen;
    pMsg->usSrcId = srcId;
    pMsg->usDstId = dstId;
    pMsg->usSequence = 0;
    pMsg->usTransId = 0;

    if (0 != usBufLen)
    {
        /* check exceed max buff size? */
        if (UCORE_MAX_MESSAGE_BUFF_LEN <= usBufLen)
        {
            free(pMsg);
            return UCORE_ERR_MEM_ALLOC_FAILED;
        }

        /* alloc user buff */
        pMsg->pBuf = malloc(usBufLen);

        /* copy buff */
        memcpy(pMsg->pBuf, pBuf, usBufLen);
    }

    /* ok, now post this message */
    if (OS_ERR_NONE != OSQPost(g_QSemCoreMsg, (void *)pMsg))
    {
        /* free message */
        if (pMsg->pBuf) free(pMsg->pBuf);
        if (pMsg) free(pMsg);
        return UCORE_ERR_MSG_POST_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}

u8 UCore_PostMessage3(u16 usMessageType, u16 usBufLen, void *pBuf, u16 srcId, u16 dstId, u16 usSeq, u16 usTransId)
{
    UCORE_MSG_S *pMsg       = NULL;

    /* alloc message buf */
    pMsg = malloc(sizeof(UCORE_MSG_S));
    if (NULL == pMsg)
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    memset(pMsg, 0, sizeof(UCORE_MSG_S));
    pMsg->usMsgType = usMessageType;
    pMsg->usBufLen  = usBufLen;
    pMsg->usSrcId = srcId;
    pMsg->usDstId = dstId;
    pMsg->usSequence = usSeq;
    pMsg->usTransId = usTransId;

    if (0 != usBufLen)
    {
        /* check exceed max buff size? */
        if (UCORE_MAX_MESSAGE_BUFF_LEN <= usBufLen)
        {
            free(pMsg);
            return UCORE_ERR_MEM_ALLOC_FAILED;
        }

        /* alloc user buff */
        pMsg->pBuf = malloc(usBufLen);

        /* copy buff */
        memcpy(pMsg->pBuf, pBuf, usBufLen);
    }

    /* ok, now post this message */
    if (OS_ERR_NONE != OSQPost(g_QSemCoreMsg, (void *)pMsg))
    {
        /* free message */
        if (pMsg->pBuf) free(pMsg->pBuf);
        if (pMsg) free(pMsg);
        return UCORE_ERR_MSG_POST_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}


