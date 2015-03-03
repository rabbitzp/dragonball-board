/**************************************************************************************************
  Filename:       jstack_func.h
  Revised:         
  Revision:       

  Description:    This file contains jstack functions.
**************************************************************************************************/

#ifndef __JSTACK_FUNC_H_
#define __JSTACK_FUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define JMI_MSG_STRUCT_DATA_LEN(a)    (sizeof(a) - sizeof(JMI_MSG_HEADER_S))

void JMI_Init(void);

void JMI_SetRegistStatus(uint8 ucRegistStatus);

uint8 JMI_GetRegistStatus(void);

void JMI_FillMessageReqHeader(JMI_MSG_HEADER_S *pstHeader, uint8 ucCommand, uint16 uslen);

void JMI_FillMessageRspHeader(JMI_MSG_HEADER_S *pstHeader, uint8 ucEpid, uint16 usSeq, uint8 ucCommand, uint8 ucResult, uint16 uslen);

void JMI_FillMessageBody(JMI_MSG_HEADER_S *pstHeader, uint16 usBodyLen, void *pData);

bool JMI_ValidateMessage(uint8 *pcBuff, uint16 usLen);

uint16 JMI_GetCurrentSeq(void);

uint8 JMI_MakeRegistInfo(IN uint16 usShortAddr, IN uint8 *pcBuff, INOUT uint16 *usMaxBuffLen);

void JMI_SetEpId(IN uint8 ucEpid);

uint8 JMI_GetEpId(void);

void JMIDebug(char *buf);

#ifdef __cplusplus
}
#endif

#endif /* __JSTACK_FUNC_H_ */


