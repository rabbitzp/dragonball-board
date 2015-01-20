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

#include <includes.h>
#include <user_core.h>
#include <user_task_key.h>
#include <user_task_lcd.h>
#include <user_task_hmc.h>

#define DEFAULT_USER_TASK_PRIO              5  //优先级高
#define DEFAULT_USER_STK_SIZE               128
#define MAX_USER_CORE_MSG_SIZE              16


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

    /* create sub task */
    ucResult = UKey_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user key task failed.\r\n");
    }

    ucResult = ULCD_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user LCD task failed.\r\n");
    }

    ucResult = UHMC_Start();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        printf("Start user HMC task failed.\r\n");
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
    INT8U           err     = 0;
    UCORE_MSG_S     *pMsg   = NULL;
    
    /* main process here */
    while (1)
    {
        pMsg = (UCORE_MSG_S *) OSQPend(g_QSemCoreMsg, 0, &err);
        if ((OS_NO_ERR == err) && (NULL != pMsg))
        {
            switch (pMsg->usMsgType)
            {
                /* 以下消息发送到LCD任务，直接传递，注意不在这里销毁 */
                case UCORE_MESSAGE_TYPE_KEY_HOME:
                case UCORE_MESSAGE_TYPE_KEY_OK:
                case UCORE_MESSAGE_TYPE_KEY_UP:
                case UCORE_MESSAGE_TYPE_KEY_DOWN:
                case UCORE_MESSAGE_TYPE_KEY_RETURN:
                    {
                        OSMboxPost(g_QMboxUIMsg, pMsg);
                    }
                    break;                    
                case UCORE_MESSAGE_TYPE_TEST1:
                    {
                        printf("Message testing 1.\r\n");
                        UCORE_DEL_MESSAGE(pMsg);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_TEST2:
                    {
                        printf("Message testing 2.\r\n");
                        UCORE_DEL_MESSAGE(pMsg);
                    }
                    break;                    
                default:
                    printf("Unknown message type:%d\r\n", pMsg->usMsgType);

                    UCORE_DEL_MESSAGE(pMsg);
                    break;
            }
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

