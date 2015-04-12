/*
*********************************************************************************************************
*
*                                      user core types and errorcode define
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore_defs.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define IN
#define OUT
#define INOUT

#define UCORE_INVALID_EPID              (0xFF)
#define UCORE_BROADCAST_ADDR            (0xFFFF)

#define UCORE_AF_SEND_MAX_RETRY_TIMES   (3)

#if !defined( UCORE_EX_COMM_UART_PORT )
#define UCORE_EX_COMM_UART_PORT  0
#endif

/* 
    user event from 0x0001 to 0x4000，采用独热码方式，多少bit代码多少位，
    六状态的独热码可以表示成0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, ……, 0x4000
*/
/* reRegist timer */
#define UCORE_APP_EVENT_ID_REGIST                   0x0001
  
/* change pandid  */
#define UCORE_APP_EVENT_ID_CHANGE_PANID             0x0004

/* reboot device  */
#define UCORE_APP_EVENT_ID_REBOOT_DEVICE            0x0008

/* keep alive event */
#define UCORE_APP_EVENT_ID_KEEP_ALIVE               0x0020

/* speed test event */
#define UCORE_APP_EVENT_ID_TEST_SPEED               0x0040

/* regist timer delay(1000ms) */
#define UCORE_APP_EVENT_DELAY_REGIST                5000

/* query event timer default value */
#define UCORE_APP_EVENT_DELAY_SCAN_DEVICE           500

/* keep alive from coordinator in 5 seconds */
#define UCORE_APP_EVENT_KEEPALIVE_TIME              5000

/* speed test packet send time val */
#define UCORE_APP_EVENT_TEST_SPEED_SEND_INTER_TIME  20

/* Exported types ------------------------------------------------------------*/
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

/* error code define */
typedef enum _tagUCErrCode
{   /* from 0 - 255 */
    UCORE_ERR_SUCCESS                       = 0,
    UCORE_ERR_COMMON_FAILED                 = 1,
    UCORE_ERR_MEM_ALLOC_FAILED              = 2,
    UCORE_ERR_MSG_POST_FAILED               = 3,
    UCORE_ERR_CREATE_TASK_FAILED            = 4,
	UCORE_ERR_PAPA_ERROR					= 5,
	UCORE_ERR_INVALID_MESSAGE				= 6,
	UCORE_ERR_NO_MESSAGE                    = 7,
    UCORE_RESULT_INVALID_EPDEV              = 8,
    UCORE_RESULT_DUMP_EPDEV                 = 9,
    
    /* for zstack using 100 - 150 */
    UCORE_RESULT_OTA_DUP_MSG                = 100,
    UCORE_RESULT_OTA_SER_BUSY               = 101,
    UCORE_RESULT_AF_SEND_ERROR              = 102,

    /* bottom */
    UCORE_ERR_BUTT,
    UCORE_ERR_END,    
}UCORE_ERROR_E;

// NV Items Reserved for applications (user applications)
// 0x0201 - 0x0FFF
typedef enum tagUCoreNvConfigItem
{
    UCORE_NV_CONFIG_ITEM_MAX_EP_ID = 0x0201,
    UCORE_NV_CONFIG_ITEM_CUR_EP_ID = 0x0202,

    UCORE_NV_CONFIG_ITEM_BUTT,
}UCORE_NV_CONFIG_ITEM_E;

/* regist status define */
typedef enum tagUCoreRegistStatus
{
    UCORE_REGIST_UNREGISTED       = 0,
    UCORE_REGIST_REGISTING        = 1,
    UCORE_REGIST_REGITED          = 2,

    UCORE_REGIST_BUTT,
}UCORE_REGIST_STATUS_E;

/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
#define UCORE_PARA_ASSERT_NULL(p)                   do { if ( NULL == p ) return; }while(0)

#define UCORE_PARA_ASSERT(p)                        do { if ( p ) return; }while(0)
        
#define UCORE_PARA_ASSERT_NULL_RETURN(p, ucRetVal)  do { if ( NULL == p ) return ucRetVal; }while(0)

#define UCORE_PARA_ASSERT_RETURN(p, ucRetVal)       do { if ( p ) return ucRetVal; }while(0)


#ifdef __cplusplus
}
#endif


#endif

