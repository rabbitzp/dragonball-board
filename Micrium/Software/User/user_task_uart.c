/*
*********************************************************************************************************
*
*                                      User uart task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_uart.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <user_task_uart.h>
#include "GUI_WndDef.h"  /* valid LCD configuration */
#include <eem.h>
#include <eem_struct.h>
#include <user_os_func.h>
#include <user_ep_manage.h>

/*----------------macros declare here---------------------*/
#define             MAX_UART_RECV_MSG_BUFF_SIZE              63

/*---------------- global vars declare here---------------------*/
OS_EVENT            *g_QSemUart2MsgRecv = NULL;


/*----------------local global vars declare here---------------------*/
static OS_STK       g_UserTaskUartRecvSTK[DEFAULT_USER_UART_STK_SIZE];
static OS_STK       g_UserTaskUartProcessSTK[DEFAULT_USER_UART_STK_SIZE];

static void         *g_ucUart2RxdBuff[MAX_UART_RECV_MSG_BUFF_SIZE];

static u8           g_ucUart2RXArray[MAX_UART_RECV_MSG_BUFF_SIZE];
static u8           g_ucUartBuffPos = 0;
static OS_EVENT     *g_OSemBuffRwEvent;

/*----------------local global funcs declare here---------------------*/
static void UUart_TaskRecvProc(void *p_arg);
static void UUart_TaskProcessProc(void *p_arg);
static void UUart_Init(void);
static void UUart_EventRecvLoop(void);

/* for testing */
static void UUart_DumpMemory(void);

u8 UUart_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    printf("starting user task uart...\r\n");

    /* call init first */
    UUart_Init();    

    ucResult = OSTaskCreate(UUart_TaskRecvProc, (void *)0, (OS_STK *)&g_UserTaskUartRecvSTK[DEFAULT_USER_UART_STK_SIZE - 1], DEFAULT_USER_TASK_UART_RECV_PRIO);
    if (OS_ERR_NONE != ucResult)
    {
        printf("User task uart receive create Failed, value:%d.\r\n", ucResult);
        return UCORE_ERR_CREATE_TASK_FAILED;
    }  

    ucResult = OSTaskCreate(UUart_TaskProcessProc, (void *)0, (OS_STK *)&g_UserTaskUartProcessSTK[DEFAULT_USER_UART_STK_SIZE - 1], DEFAULT_USER_TASK_UART_PROCESS_PRIO);
    if (OS_ERR_NONE != ucResult)
    {
        printf("User task uart process create Failed, value:%d.\r\n", ucResult);
        return UCORE_ERR_CREATE_TASK_FAILED;
    }     
   
    return UCORE_ERR_SUCCESS;
}

void UUart_TaskRecvProc(void *p_arg)
{
    printf("User uart recv task start.\r\n");

    /* enter event loop */
    UUart_EventRecvLoop();

    printf("User uart recv task exit.\r\n");
}

void UUart_Init(void)
{
    u16          usTotalLen = 0; 
    EEM_HEADER_S *pHeader   = NULL;
    u8           *pcBuff    = NULL;
    
    g_QSemUart2MsgRecv = OSQCreate(&g_ucUart2RxdBuff[0], MAX_UART_RECV_MSG_BUFF_SIZE);
    if (NULL == g_QSemUart2MsgRecv)
    {
        printf("Create g_OSemBuffRwEvent failed.\r\n");
        return;
    }

    g_OSemBuffRwEvent = OSSemCreate(1);
    if (NULL == g_OSemBuffRwEvent)
    {
        printf("Create g_OSemBuffRwEvent failed.\r\n");
        return;
    }

    /* query ep basic info */
	pHeader = EEM_CreateHeader(0, EEM_COMMAND_QUERY_EPINFO, UCORE_ERR_SUCCESS);
	if (NULL == pHeader)
	{
	    printf("EEM CreateHeader failed.\r\n");
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
            
    printf("User task uart init finished.\r\n");
}

void UUart_EventRecvLoop(void)
{
    INT8U           err         = 0;    
    u8              *pbuff      = NULL;   	
    
    while (1)
    {
        /* wait for message in */
        pbuff = (u8 *) OSQPend(g_QSemUart2MsgRecv, 0, &err);
        if ((OS_NO_ERR == err) && (NULL != pbuff))
        {
            OSSemPend(g_OSemBuffRwEvent, 1000, &err);
            if (OS_NO_ERR == err)
            {
                g_ucUart2RXArray[g_ucUartBuffPos++] = *pbuff;
                
                OSSemPost(g_OSemBuffRwEvent);
            }            
            
            /* clean buff */
            free(pbuff);
        }
    }
}

void UUart_TaskProcessProc(void *p_arg)
{
    INT8U           err       = 0;
    u8              ucResult  = 0;
    EEM_HEADER_S    *pHeader  = NULL;
    
    printf("User uart process task start.\r\n");

    /* enter event loop */
    while (1)
    {
        OSSemPend(g_OSemBuffRwEvent, 1000, &err);
        if (OS_NO_ERR == err)
        {
    		while (UCORE_ERR_NO_MESSAGE != (ucResult = EEM_GetMessage(g_ucUart2RXArray, &g_ucUartBuffPos, &pHeader)))
    		{
    			if (UCORE_ERR_SUCCESS == ucResult)
    			{
    			    switch (pHeader->usCommand)
                    {
                        case EEM_COMMAND_QUERY_EPINFO:
                            {
                                u16             usPayLen  = 0;
                                EEM_EP_INFO_S   *pPayload = NULL;
                                EP_INFO_S       *pEpInfo  = NULL;
                                
                                /* here get message payload */
                                pPayload = (EEM_EP_INFO_S *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_INFO, &usPayLen);
                                if (NULL != pPayload) /* must not be null */
                                {
                                    /* alloc buff */
                                    pEpInfo = (EP_INFO_S *) malloc(sizeof(EP_INFO_S));
                                    if (NULL != pEpInfo)
                                    {
                                        /* clear buffer */
                                        memset(pEpInfo, 0, sizeof(EP_INFO_S));

                                        /* asign values */
                                    	pEpInfo->ucEpId     = pHeader->ucEpId;
                                        pEpInfo->ucEpType   = pPayload->ucEpType;
                                        pEpInfo->usEpAddr   = pPayload->usEpAddr;    

                                        if (EP_TYPE_EP == pEpInfo->ucEpType)
                                            strncpy((char *) pEpInfo->sEpName, "EP", MAX_EP_NAME_LEN - 1);
                                        else if (EP_TYPE_ROUTER == pEpInfo->ucEpType)
                                            strncpy((char *) pEpInfo->sEpName, "Router", MAX_EP_NAME_LEN - 1);
                                        else
                                            strncpy((char *) pEpInfo->sEpName, "Coord", MAX_EP_NAME_LEN - 1);

                                        /* ok, finnally send a message to core */
                                        ucResult = UCore_PostMessage1(UCORE_MESSAGE_TYPE_QUERY_EPINFO, sizeof(EP_INFO_S), (void *)pEpInfo);

                                        /* free buff first */                                        
                                        free((void *)pEpInfo);
                                        pEpInfo = NULL;

                                        /* check result */                                        
                                        if (UCORE_ERR_SUCCESS != ucResult)
                                        {
                                            printf("Send Ep[%d] query ep info message Failed.\r\n", pHeader->ucEpId);
                                        }
                                    }
                                }
                            }
                            break;
                        case EEM_COMMAND_COOR_STATCHAG:
                            {   
                                ucResult = UCore_PostMessage1(UCORE_MESSAGE_TYPE_COOR_STATCHAG, 0, NULL);
                                if (UCORE_ERR_SUCCESS != ucResult)
                                {
                                    printf("Send coordinator state change message Failed.\r\n");
                                }
                            }
                            break;
                        case EEM_COMMAND_EP_ONLINE:
                            {
                                u16             usPayLen  = 0;
                                EEM_EP_INFO_S   *pPayload = NULL;
                                EP_INFO_S       *pEpInfo  = NULL;
                                
                                /* here get message payload */
                                pPayload = (EEM_EP_INFO_S *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_INFO, &usPayLen);
                                if (NULL != pPayload) /* must not be null */
                                {
                                    /* alloc buff */
                                    pEpInfo = (EP_INFO_S *) malloc(sizeof(EP_INFO_S));
                                    if (NULL != pEpInfo)
                                    {
                                        /* clear buffer */
                                        memset(pEpInfo, 0, sizeof(EP_INFO_S));

                                        /* asign values */
                                    	pEpInfo->ucEpId     = pHeader->ucEpId;
                                        pEpInfo->ucEpType   = pPayload->ucEpType;
                                        pEpInfo->usEpAddr   = pPayload->usEpAddr;    

                                        if (EP_TYPE_EP == pEpInfo->ucEpType)
                                            strncpy((char *) pEpInfo->sEpName, "EP", MAX_EP_NAME_LEN - 1);
                                        else if (EP_TYPE_ROUTER == pEpInfo->ucEpType)
                                            strncpy((char *) pEpInfo->sEpName, "Router", MAX_EP_NAME_LEN - 1);
                                        else
                                            strncpy((char *) pEpInfo->sEpName, "Coord", MAX_EP_NAME_LEN - 1);

                                        /* ok, finnally send a message to core */
                                        ucResult = UCore_PostMessage1(UCORE_MESSAGE_TYPE_EP_ONLINE, sizeof(EP_INFO_S), (void *)pEpInfo);

                                        /* free buff first */                                        
                                        free((void *)pEpInfo);
                                        pEpInfo = NULL;

                                        /* check result */                                        
                                        if (UCORE_ERR_SUCCESS != ucResult)
                                        {
                                            printf("Send Ep[%d] online message Failed.\r\n", pHeader->ucEpId);
                                        }
                                    }
                                }
                            }
                            break;
                        case EEM_COMMAND_EP_OFFLINE:
                            break;
                        case EEM_COMMAND_TRANS_COM:
                            {
                                u16             usPayLen  = 0;
                                u8              *pPayload = NULL;
                                
                                /* here get message payload */
                                pPayload = (u8 *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_RAW_DATA, &usPayLen);
                                if (NULL != pPayload) /* must not be null */
                                {
                                    /* ok, send a message to core */
                                    ucResult = UCore_PostMessage1(UCORE_MESSAGE_TYPE_TRANS_COM, usPayLen, (void *) pPayload);
                                    if (UCORE_ERR_SUCCESS != ucResult)
                                    {
                                        printf("Send Ep[%d] trans com message Failed.\r\n", pHeader->ucEpId);
                                    }
                                }
                            }
                            break;
                        case EEM_COMMAND_REPORT_GPS:
                            {
                                u16                 usPayLen   = 0;
                                EEM_EP_GPS_INFO_S   *pGpsInfo = NULL;
                                
                                /* here get message payload */
                                pGpsInfo = (EEM_EP_GPS_INFO_S *) EEM_GetPayload(pHeader, EEM_PAYLOAD_TYPE_EP_GPS_INFO, &usPayLen);
                                if (NULL != pGpsInfo) /* must not be null */
                                {
                                    /* check ep id */
                                    if (pHeader->ucEpId != pGpsInfo->epid)
                                    {
                                        printf("Check ep invalid, header:%d info:%d bring to same!\r\n", pHeader->ucEpId, pGpsInfo->epid);

                                        pGpsInfo->epid = pHeader->ucEpId;
                                    }
                                    
                                    /* ok, send a message to core */
                                    ucResult = UCore_PostMessage1(UCORE_MESSAGE_TYPE_REPORT_GPS, usPayLen, (void *) pGpsInfo);
                                    if (UCORE_ERR_SUCCESS != ucResult)
                                    {
                                        printf("Send Ep[%d] report GPS message Failed.\r\n", pHeader->ucEpId);
                                    }
                                }                                
                            }
                            break;
                        default:
                            printf("Unknown message type[%d] from uart.\r\n", pHeader->usCommand);
                            break;
                    }                    

                    /* all message delete here */
    				EEM_Delete((void **) &pHeader);	
    			}
    		}
                        
            OSSemPost(g_OSemBuffRwEvent);
        }
        
        /* delay for a while */
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
}

static void UUart_DumpMemory(void)
{
    u8    i = 0;

    for (i = 0; i < g_ucUartBuffPos; i++)
    {
        printf("%02X ", g_ucUart2RXArray[i]);
    }
    
    printf("\r\n");
}

