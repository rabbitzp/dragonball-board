/*
*********************************************************************************************************
*
*                                      eem types and errorcode define
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : eem_defs.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __EEM_DEFS_H__
#define __EEM_DEFS_H__

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

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
	UCORE_ERR_NET_INIT_FAILED               = 8,
	UCORE_ERR_NET_CONNECT_FAILED            = 9,
	UCORE_ERR_NET_CONNECT_TIMEOUT           = 10,
	UCORE_ERR_NET_IO_ERROR                  = 11,	

    /* bottom */
    UCORE_ERR_BUTT,
    UCORE_ERR_END,    
}UCORE_ERROR_E;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif

