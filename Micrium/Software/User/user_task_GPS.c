/*
*********************************************************************************************************
*
*                                      User gps receive task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_gps.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <user_task_gps.h>
#include "GUI_WndDef.h"  /* valid LCD configuration */
#include "GUI_WndMain.h"
#include "nmea_parser.h"
#include <user_os_func.h>
#include <eem.h>
#include <eem_struct.h>
#include <user_ep_manage.h>


#define MAX_GPS_OSQ_SIZE 32

extern EP_INFO_S g_stMyEpInfo;


/*---------------- global vars declare here---------------------*/
OS_EVENT            *g_QSemGPSMsgRecv = NULL;


/*----------------local global vars declare here---------------------*/
__align(8) static OS_STK     g_UserTaskGPSRecvSTK[DEFAULT_USER_GPS_RECV_STK_SIZE];
static void                 *g_GPSOSQ[MAX_GPS_OSQ_SIZE];
/*----------------local global funcs declare here---------------------*/
static void GPS_TaskRecvProc(void *p_arg);
static void GPS_Init(void);
static void GPS_EventRecvLoop(void);

u8 GPS_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    printf("starting user task GPS...\r\n");

    /* call init first */
    GPS_Init();    

    ucResult = OSTaskCreate(GPS_TaskRecvProc, (void *)0, (OS_STK *)&g_UserTaskGPSRecvSTK[DEFAULT_USER_GPS_RECV_STK_SIZE - 1], DEFAULT_USER_TASK_GPS_RECV_PRIO);
    if (OS_ERR_NONE != ucResult)
    {
        printf("User task GPS receive create Failed, value:%d.\r\n", ucResult);
        return UCORE_ERR_CREATE_TASK_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}

void GPS_TaskRecvProc(void *p_arg)
{
    printf("User GPS recv task start.\r\n");

    /* enter event loop */
    GPS_EventRecvLoop();

    printf("User GPS recv task exit.\r\n");

    return;
}

void GPS_Init(void)
{
    g_QSemGPSMsgRecv = OSQCreate(&g_GPSOSQ[0], MAX_GPS_OSQ_SIZE);
    if (NULL == g_QSemGPSMsgRecv)
    {
        printf("Create g_OSemGPSMsgRecv failed.\r\n");
    }

    printf("User task GPS init finished.\r\n");
}

void GPS_EventRecvLoop(void)
{
    u8              ucRet           = 0;
    u16             usTotalLen      = 0;  
    INT8U           err             = 2;    
    nmeaINFO        *pNmeaInfo      = NULL; 
    EEM_HEADER_S    *pHeader        = NULL;
    u8              *pcBuff         = NULL;
        
    
    while (1)
    {
        /* wait for message in */
        pNmeaInfo = (nmeaINFO *) OSQPend(g_QSemGPSMsgRecv, 0, &err);
        if ((OS_NO_ERR == err) && (NULL != pNmeaInfo))
        {
            WM_MESSAGE          stUiMessage;
            EEM_EP_GPS_INFO_S   stEemGpsInfo;
            
            pNmeaInfo->id = g_stMyEpInfo.ucEpId;
            
            stUiMessage.MsgId  = GUI_USER_MSG_GPS_UPDATE;   
            stUiMessage.Data.p = (void *) pNmeaInfo;

            /* send to radar window */
            WM_SendMessage(GUI_GetCurrentWnd(), &stUiMessage);  

            /* 如果是普通终端，需要发送GPS信息给CC2530，由CC2530上再进行处理，如果是EP，则透传给COORD
               如果是COORD，有两种处理方式，如果是本机的GPS，则直接广播，如果是EP透传的，也将GPS广播 */
        	pHeader = EEM_CreateHeader(g_stMyEpInfo.ucEpId, EEM_COMMAND_REPORT_GPS, UCORE_ERR_SUCCESS);
        	if (NULL != pHeader)
        	{
                /* prepare struct */
            	stEemGpsInfo.epid = pNmeaInfo->id;		    /**< device id */    
            	stEemGpsInfo.status = pNmeaInfo->status;			/**< Status (A = active or V = void) */
            	stEemGpsInfo.ns = pNmeaInfo->ns;				/**< [N]orth or [S]outh */
            	stEemGpsInfo.ew = pNmeaInfo->ew;				/**< [E]ast or [W]est */
            	stEemGpsInfo.lat = pNmeaInfo->lat;	        /**< Latitude in NDEG - [degree][min].[sec/60] */
            	stEemGpsInfo.lon = pNmeaInfo->lon;	        /**< Longitude in NDEG - [degree][min].[sec/60] */
            	stEemGpsInfo.speed = pNmeaInfo->speed;	        /**< Speed over the ground in knots */    
                stEemGpsInfo.course = pNmeaInfo->course;         /**< direction, 000.0~359.9, base N */  

                /* apend ep addr */
            	ucRet = EEM_AppendPayload(&pHeader, EEM_PAYLOAD_TYPE_EP_GPS_INFO, sizeof(EEM_EP_GPS_INFO_S), (void *) &stEemGpsInfo);
                if (UCORE_ERR_SUCCESS == ucRet)
                {
                    /* get buff */
                	pcBuff = EEM_GetBuff(pHeader, &usTotalLen);
                    if (NULL != pcBuff)
                    {
                        USART_Send(USART2, usTotalLen, (void *) pcBuff);
                        
                        /* dump message */
                        //EEM_DumpMessage(pHeader);
                    }
                }

                /* clean buffer */
                EEM_Delete((void **) &pHeader);
            }

            /* clean buff */
            free(pNmeaInfo);
        }
    }
}

