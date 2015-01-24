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

/*----------------macros declare here---------------------*/
#define             MAX_UART_RECV_MSG_BUFF_SIZE              255

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
    g_QSemUart2MsgRecv = OSQCreate(&g_ucUart2RxdBuff[0], MAX_UART_RECV_MSG_BUFF_SIZE);
    if (NULL == g_QSemUart2MsgRecv)
    {
        printf("Create g_OSemBuffRwEvent failed.\r\n");
    }

    g_OSemBuffRwEvent = OSSemCreate(1);
    if (NULL == g_OSemBuffRwEvent)
    {
        printf("Create g_OSemBuffRwEvent failed.\r\n");
    }

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
    				EEM_DumpMessage(pHeader);
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

