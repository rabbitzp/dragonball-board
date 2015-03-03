/**************************************************************************************************
  Filename:       jstack_structs.h
  Revised:         
  Revision:       

  Description:    This file contains jstack structs defs.
**************************************************************************************************/

#ifndef __JSTACK_STRUCTS_H_
#define __JSTACK_STRUCTS_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagJmiNvItemMaxEpId
{
    uint8   ucEpid;
    uint8   ucRev[3];
}JMI_NV_ITEM_MAX_EPID_S;

typedef struct tagJmiNvItemCurEpId
{
    uint8   ucEpid;
    uint8   ucRev[3];
}JMI_NV_ITEM_CUR_EPID_S;

typedef struct tagJmiRegistInfo
{
    /* msg header */
    JMI_MSG_HEADER_S stHeaer;

    /* short addr */
    uint16  usShortAddr;

    /* device id */
    uint8   ucEpid;

    /* device type */
    uint8   ucEpType;
}JMI_REGIST_INFO_S;

typedef struct tagJmiCommDelayAck
{
    /* msg header */
    JMI_MSG_HEADER_S stHeaer;

    /* delay value */
    uint16 usDelay;
}JMI_COMM_DELAY_ACK_S;

typedef struct tagJmiMcuStatus
{
    /* msg header */
    JMI_MSG_HEADER_S stHeaer;

    /* mcu vol */
    float fVol;

    /* mcu temp */
    float fTemp;
}JMI_MCU_STATUS_S;

#ifdef __cplusplus
}
#endif

#endif /* __JSTACK_STRUCTS_H_ */


