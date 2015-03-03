/**************************************************************************************************
  Filename:       jstack_defs.h
  Revised:         
  Revision:       

  Description:    This file contains jstack base defs.
**************************************************************************************************/

#ifndef __JSTACK_DEFS_H_
#define __JSTACK_DEFS_H_

#ifdef __cplusplus
extern "C"
{
#endif

// NV Items Reserved for applications (user applications)
// 0x0201 - 0x0FFF
typedef enum tagJmiNvConfigItem
{
    JMI_NV_CONFIG_ITEM_MAX_EP_ID = 0x0201,
    JMI_NV_CONFIG_ITEM_CUR_EP_ID = 0x0202,

    JMI_NV_CONFIG_ITEM_BUTT,
}JMI_NV_CONFIG_ITEM_E;

typedef enum tagJMIResult
{
    JMI_RESULT_SUCCESS              = 0,
    JMI_RESULT_COMMON_FAIL          = 1,
    JMI_RESULT_INVALID_PARA         = 2,
    JMI_RESULT_OTA_DUP_MSG          = 3,
    JMI_RESULT_OTA_SER_BUSY         = 4,
    

    JMI_RESULT_BUTT,
}JMI_RESULT_E;

typedef enum tagJMIRegistStatus
{
    JMI_REGIST_UNREGISTED       = 0,
    JMI_REGIST_REGISTING        = 1,
    JMI_REGIST_REGITED          = 2,

    JMI_REGIST_BUTT,
}JMI_REGIST_STATUS_E;

typedef enum tagJMICommand
{
    /*0 - 49 is reseved */
    JMI_COMMAND_REGIST                  = 50,
    JMI_COMMAND_UNREGIST                = 51,
    JMI_COMMAND_KEEP_ALIVE              = 52,

    JMI_COMMAND_TRANSPARENT_COM         = 53,
    JMI_COMMAND_REPORT_GPS              = 54,

    JMI_COMMAND_TRANSPARENT_BUTT,
}JMI_COMMAND_E;

/* Jstack Media Interface header */
typedef struct tagJMIHeader
{
    uint8       ucMagic;            /**< every talk start with magic word 0xFF */
    uint8       ucEPId;              /**< endpoint id, private using, 0xFF is valid value*/
    uint16      usSeq;              /**< 0xFFFF max*/
    uint8       ucCommand;          /**< message id*/
    uint8       ucResult;           /**< 0 if success, otherwise to see error code */
    uint16      usDataLen;          /**< payload len */
}JMI_MSG_HEADER_S;

/* Macros define here */
//#define JSTACK_DEBUG

#define IN
#define OUT
#define INOUT

#define JMI_INVALID_EPID        (0xFF)

#define JMI_MSG_HEADER_LEN      sizeof(JMI_MSG_HEADER_S)

#define JMI_PARA_ASSERT(p)      do { if ( NULL == p ) return; }while(0)
        
#define JMI_PARA_ASSERT_RETURN(p, ucRetVal) do { if ( NULL == p ) return ucRetVal; }while(0)


#ifdef __cplusplus
}
#endif

#endif /* __JSTACK_DEFS_H_ */

