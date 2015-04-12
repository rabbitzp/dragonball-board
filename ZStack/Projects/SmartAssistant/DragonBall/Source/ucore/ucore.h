/*
*********************************************************************************************************
*
*                                          user core 
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : ucore.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __UCORE_H__
#define __UCORE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ucore\ucore_defs.h"
#include "ucore\ucore_struct.h"

/*********************************************************************
 * MACROS
 */
#define UCORE_MAX_EVENT_PROCESSOR  6

/*********************************************************************
 * CONSTANTS
 */ 
extern UCORE_EVENT_PROCESSOR_S gCoreEventProcessor[UCORE_MAX_EVENT_PROCESSOR];

extern uint16                  gusCurTransactionId;

extern uint16                  gChangedPanId;

extern uint8                   gKeepAliveReset;
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */
void            UCORE_Init(uint8 ucTaskId);

uint8           UCORE_AF_Send(uint16 usEpAddr, uint8 ucDestEndPoint, endPointDesc_t *pSrcEP, uint16 usClusterID, uint16 usDatalen, uint8 *pBuf, uint8 ucOptions, uint8 ucRadius);

void            UCORE_SetRegistStatus(IN uint8 ucRegistStatus);

uint8           UCORE_GetRegistStatus(void);

void            UCORE_SetEpId(IN uint8 ucEpid);

uint8           UCORE_GetEpId(void);

void            UCORE_SetTaskID(uint8 ucTaskId);

uint8           UCORE_GetTaskID(void);

void            UCORE_SetNetworkState(devStates_t emNwkState);

devStates_t     UCORE_GetNetworkState(void);

void            UCORE_ProcessMSGCmd( afIncomingMSGPacket_t *pkt );

/* event process functions */

uint16          UCore_SysEventProc(void *para);

uint16          UCore_RegistEventProc(void *para);

uint16          UCore_ChangePanIdEventProc(void *para);

uint16          UCore_RebootEventProc(void *para);

uint16          UCore_KeepAliveEventProc(void *para);

uint16          UCore_TestSpeedEventProc(void *para);

/* system call back functions */
void            UCore_SerialCallBack(uint8 port, uint8 event);


void            UCORE_Debug(const char *buf, ...);


/* system functions */
void            UCore_DelayMS(unsigned int msec);

#ifdef __cplusplus
}
#endif

#endif

