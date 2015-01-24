/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_core.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_CORE_H__
#define  __USER_CORE_H__
/*--------------macros declare here-----------------------------*/
#define DEFAULT_USER_TASK_KEY_PRIO              11
#define DEFAULT_USER_TASK_UART_RECV_PRIO        13
#define DEFAULT_USER_TASK_UART_PROCESS_PRIO     14
#define DEFAULT_USER_TASK_HMC_PRIO              15

#define DEFAULT_USER_KEY_STK_SIZE               64
#define DEFAULT_USER_UART_STK_SIZE              64
#define DEFAULT_USER_HMC_STK_SIZE               64

#define UCORE_MAX_MESSAGE_BUFF_LEN              (1024)                  /* max 1k dynic buff size */

/*--------------enums declare here------------------------------*/
typedef enum _tagUCErrCode
{   /* from 0 - 255 */
    UCORE_ERR_SUCCESS                       = 0,
    UCORE_ERR_COMMON_FAILED                 = 1,
    UCORE_ERR_MEM_ALLOC_FAILED              = 2,
    UCORE_ERR_MSG_POST_FAILED               = 3,
    UCORE_ERR_CREATE_TASK_FAILED            = 4,
    UCORE_ERR_PAPA_ERROR                    = 5,
    UCORE_ERR_INVALID_MESSAGE               = 6,
    UCORE_ERR_NO_MESSAGE                    = 7,

    /* bottom */
    UCORE_ERR_BUTT,
    UCORE_ERR_END,    
}UCORE_ERROR_E;

typedef enum _tagUCMsgType
{
    /* 0 - 100 reserved*/

    /* 100- 200 system*/
    

    /* 200 - 1000 for user*/     
    UCORE_MESSAGE_TYPE_TEST1        = 200,
    UCORE_MESSAGE_TYPE_TEST2        = 201,
    
}UCORE_MESSAGE_TYPE_E;


/*--------------struct declare here-----------------------------*/
typedef struct _tagUCoreMsg
{
    u16     usMsgType;
    
    u16     usBufLen;
    void    *pBuf;
}UCORE_MSG_S;


/*--------------functions declare here-----------------------------*/
u8   UCore_Start(void);

u8   UCore_PostMessage2(UCORE_MSG_S *pMsg);
u8   UCore_PostMessage1(u16 usMessageType, u16 usBufLen, void *pBuf);


/*---------------extern vars & functinos here-----------------------*/
#define UCORE_DEL_MESSAGE(p)            \
do                                      \
{                                       \
            if (NULL != p->pBuf)        \
            {                           \
                free(p->pBuf);          \
                p->pBuf = NULL;         \
            }                           \
            free(p);                    \
            p = NULL;                   \
}while(0)                               \

#endif

