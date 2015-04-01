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
#define DEFAULT_USER_TASK_CONSOLE_PRIO          8

#define DEFAULT_USER_TASK_KEY_PRIO              11
#define DEFAULT_USER_TASK_UART_RECV_PRIO        13
#define DEFAULT_USER_TASK_UART_PROCESS_PRIO     14
#define DEFAULT_USER_TASK_HMC_PRIO              15
#define DEFAULT_USER_TASK_GPS_RECV_PRIO         16
#define DEFAULT_USER_TASK_GPS_PROCESS_PRIO      17
#define DEFAULT_USER_TASK_NET_PROCESS_PRIO      18


#define DEFAULT_USER_KEY_STK_SIZE               64
#define DEFAULT_USER_UART_STK_SIZE              64
#define DEFAULT_USER_HMC_STK_SIZE               64
#define DEFAULT_USER_GPS_RECV_STK_SIZE          128
#define DEFAULT_USER_ETHERNET_STK_SIZE          64

#define DEFAULT_USER_CONSOLE_STK_SIZE           64

#define UCORE_MAX_MESSAGE_BUFF_LEN              (1024)                  /* max 1k dynic buff size */

#define gTCP_SOCK_TO_SERVER                     0

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
	UCORE_ERR_NET_INIT_FAILED               = 8,
	UCORE_ERR_NET_CONNECT_FAILED            = 9,
	UCORE_ERR_NET_CONNECT_TIMEOUT           = 10,
	UCORE_ERR_NET_IO_ERROR                  = 11,
	UCORE_ERR_NET_SOCKET_ERROR              = 12,
	UCORE_ERR_NET_INVALID_IP                = 13,
	
	
    /* bottom */
    UCORE_ERR_BUTT,
    UCORE_ERR_END,    
}UCORE_ERROR_E;

typedef enum _tagUCMsgType
{
    /* 0 - 99 reserved*/

    /* 100- 199 system*/
    UCORE_MESSAGE_TYPE_COOR_STATCHAG        = 100,
    UCORE_MESSAGE_TYPE_EP_ONLINE            = 101,
    UCORE_MESSAGE_TYPE_EP_OFFLINE           = 102,
    UCORE_MESSAGE_TYPE_QUERY_EPINFO         = 103,
    UCORE_MESSAGE_TYPE_REPORT_GPS           = 104,

    /* 200 - 1000 for user*/
    UCORE_MESSAGE_TYPE_TRANS_COM_READ       = 200,
    UCORE_MESSAGE_TYPE_TRANS_COM_WRITE      = 201,  

    /* for LH Tech Project */
    UCORE_MESSAGE_TYPE_CHANGE_EPID          = 600, 
    
    UCORE_MESSAGE_TYPE_TEST1                = 1000,
    UCORE_MESSAGE_TYPE_TEST2                = 1001,
    
}UCORE_MESSAGE_TYPE_E;


/*--------------struct declare here-----------------------------*/
typedef struct _tagUCoreMsg
{
    u16     usMsgType;  
    
    u16     usSrcId;
    u16     usDstId;

    u16     usSequence;
    u16     usTransId;
    
    u16     usBufLen;
    void    *pBuf;    
}UCORE_MSG_S;

typedef struct _tagUDEV_CONFIG
{
    u16      PanId;
    u8       enDebug;
    u8       enGpsDebug;    
    u8       rev[28];
}UDEV_CONFIG_S;


/*--------------functions declare here-----------------------------*/
u8   UCore_Start(void);

u8   UCore_PostMessage1(u16 usMessageType, u16 usBufLen, void *pBuf);
u8   UCore_PostMessage2(u16 usMessageType, u16 usBufLen, void *pBuf, u16 srcId, u16 dstId);
u8   UCore_PostMessage3(u16 usMessageType, u16 usBufLen, void *pBuf, u16 srcId, u16 dstId, u16 usSeq, u16 usTransId);


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
            
extern UDEV_CONFIG_S       g_udev_config;
            
#endif

