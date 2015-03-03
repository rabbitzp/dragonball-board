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
#include <user_ep_manage.h>
#include <user_task_key.h>
#include <user_task_hmc.h>
#include <user_task_uart.h>
#include <user_task_gps.h>
#include <user_os_func.h>
#include <eem.h>
#include <eem_struct.h>
#include "GUI_WndDef.h"  /* valid LCD configuration */
#include "GUI_WndMain.h"
#include "nmea_parser.h"

#define DEFAULT_USER_TASK_PRIO              5  //优先级高
#define DEFAULT_USER_STK_SIZE               128
#define MAX_USER_CORE_MSG_SIZE              16

extern EP_INFO_S g_stMyEpInfo;

/*----------------local global vars declare here---------------------*/
OS_STK      g_UserCoreSTK[DEFAULT_USER_STK_SIZE];

void        *g_UserCoreMsgQueue[MAX_USER_CORE_MSG_SIZE];
OS_EVENT    *g_QSemCoreMsg = NULL;

/* static local global functions here */
static void UCore_TaskProc(void *p_arg);
static void UCoreInit(void);
static void UCore_EventLoop(void);

u8 UCore_Start(void)
{    
    printf("starting user core....\r\n");

    if (OS_ERR_NONE == OSTaskCreate(UCore_TaskProc, (void *)0, (OS_STK *)&g_UserCoreSTK[DEFAULT_USER_STK_SIZE - 1], DEFAULT_USER_TASK_PRIO))
    {
        return UCORE_ERR_SUCCESS;
    }

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void UCore_TaskProc(void *p_arg)
{
    u8 ucResult = UCORE_ERR_COMMON_FAILED;
    
    printf("User Core Task Start.\r\n");

    /* call init */
    UCoreInit();

    /* init ep manager */
    ucResult = UEM_Init();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Init ep manager failed.\r\n");
    }    

    /* create sub task */
    ucResult = UKey_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user key task failed.\r\n");
    }

    ucResult = UHMC_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user HMC task failed.\r\n");
    } 

    ucResult = UUart_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user uart task failed.\r\n");
    }    

    ucResult = GPS_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user gps task failed.\r\n");
    }     

    /* enter event loop */
    UCore_EventLoop();

    printf("User Core Task Exit.\r\n");
}

void UCoreInit(void)
{
    /* create message queue */
    g_QSemCoreMsg = OSQCreate(&g_UserCoreMsgQueue[0], MAX_USER_CORE_MSG_SIZE);

    printf("User Core init finished.\r\n");
}

void UCore_EventLoop(void)
{
    u16             i           = 0;
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

                            printf("My ep info:id:%d type:%d addr:%x name:%s\r\n", g_stMyEpInfo.ucEpId, g_stMyEpInfo.ucEpType, g_stMyEpInfo.usEpAddr, g_stMyEpInfo.sEpName);
                        }
                    }
                    break;
                case UCORE_MESSAGE_TYPE_COOR_STATCHAG:
                    {
                        printf("Coordinator state change.\r\n");
                    }
                    break;
                case UCORE_MESSAGE_TYPE_EP_ONLINE:
                    {
                        u8           ucRet      = 0;
                        u16          usTotalLen = 0;  
                        u8           *pcBuff    = NULL;
                        EP_INFO_S    *pEpInfo   = NULL;
                        EEM_HEADER_S *pHeader   = NULL;
                        char         sRawData[] = {0x04, 0x00, 0x01, 0x11, 0x00, 0x16};

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
                                    printf("Update ep[%d] failed.\r\n", pEpInfo->ucEpId);
                                    break;
                                }
                                
                                printf("Update EP[%d] info, type:%d, addr:%02X, name:%s.\r\n", pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                            }
                            else/* add new ep */
                            {
                                /* add to ep list */
                                ucResult = UEM_AddEp(pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                                if (UCORE_ERR_SUCCESS != ucResult)
                                {
                                    printf("Add ep[%d] failed.\r\n", pEpInfo->ucEpId);
                                    break;
                                }
                                
                                printf("New EP[%d] online, type:%d, addr:%02X, name:%s.\r\n", pEpInfo->ucEpId, pEpInfo->ucEpType, pEpInfo->usEpAddr, pEpInfo->sEpName);
                            }
                        }

                        /* for testing, send a comm query message */
                        /* send ep online message main board */    
                    	pHeader = EEM_CreateHeader(pEpInfo->ucEpId, EEM_COMMAND_TRANS_COM, UCORE_ERR_SUCCESS);
                    	if (NULL == pHeader)
                    	{
                            return;
                        }

                        /* apend ep addr */
                    	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_EP_ADDR, sizeof(u16), (void *) &(pEpInfo->usEpAddr));
                        if (UCORE_ERR_SUCCESS != ucRet)
                        {
                            /* clean buffer */
                            EEM_Delete((void **) &pHeader);        
                            return;
                        }

                        /* append raw data */
                    	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, sizeof(sRawData), (void *) sRawData);
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
                            USART_Send(USART2, usTotalLen, (void *) pcBuff);
                        }

                        /* dump message */
                        EEM_DumpMessage(pHeader);

                        /* clean buffer */
                        EEM_Delete((void **) &pHeader);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TRANS_COM:
                    {
                        char *pcBuff = (char *) pMsg->pBuf;
                        printf("Trans COM:");
                        for (i=0; i<pMsg->usBufLen; i++)
                            printf("%02X ", pcBuff[i]);
                        printf("\r\n");
                    }
                    break;
                case UCORE_MESSAGE_TYPE_REPORT_GPS:
                    {
                        EEM_EP_GPS_INFO_S   *pGpsInfo = NULL;
                        
                        /* get ep gps info */
                        pGpsInfo = (EEM_EP_GPS_INFO_S *) pMsg->pBuf;
                        if (NULL != pGpsInfo)
                        {
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

                            printf("recv gps info lon:%.5f lat:%.5f\r\n", stNemaInfo.lon, stNemaInfo.lat);
                        }                        
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TEST1:
                    {
                        printf("Message testing 1.\r\n");
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TEST2:
                    {
                        printf("Message testing 2.\r\n");
                    }
                    break;                    
                default:
                    printf("Unknown message type:%d\r\n", pMsg->usMsgType);
                    break;
            }

            /* delete message */
            UCORE_DEL_MESSAGE(pMsg);
        }
    }
}

u8 UCore_PostMessage2(UCORE_MSG_S *pMsg)
{
    return OSQPost(g_QSemCoreMsg, (void *)pMsg);
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

