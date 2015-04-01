/*
*********************************************************************************************************
*
*                                      user core structs define
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore_struct.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#ifndef __UCORE_STRUCT_H__
#define __UCORE_STRUCT_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint16 (*fEventProcessor)(void *para);

typedef struct tagUCoreEventProcessor
{
    uint16          usEventId;
    fEventProcessor fEvProcessor;
}UCORE_EVENT_PROCESSOR_S;

typedef struct tagUCoreNvItemMaxEpId
{
    uint8   ucEpid;
    uint8   ucRev[3];
}UCORE_NV_ITEM_MAX_EPID_S;

typedef struct tagUCoreNvItemCurEpId
{
    uint8   ucEpid;
    uint8   ucRev[3];
}UCORE_NV_ITEM_CUR_EPID_S;

typedef struct tagUCoreRegistInfo
{
    /* short addr */
    uint16  usShortAddr;

    /* device id */
    uint8   ucEpid;

    /* device type */
    uint8   ucEpType;
}UCORE_REGIST_INFO_S;

typedef struct tagUCoreCommDelayAck
{
    /* delay value */
    uint16 usDelay;
}UCORE_COMM_DELAY_ACK_S;

typedef struct tagUCoreMcuStatus
{
    /* mcu vol */
    float fVol;

    /* mcu temp */
    float fTemp;
}UCORE_MCU_STATUS_S;

#ifdef __cplusplus
}
#endif

#endif  /* __UCORE_STRUCT_H__ */

