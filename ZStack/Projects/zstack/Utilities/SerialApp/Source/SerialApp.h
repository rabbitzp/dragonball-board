/**************************************************************************************************
  Filename:       SerialApp.h
  Revised:        $Date: 2009-02-25 17:31:49 -0800 (Wed, 25 Feb 2009) $
  Revision:       $Revision: 19273 $

  Description:    This file contains the Serial Transfer Application definitions.


  Copyright 2004-2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

#ifndef SERIALAPP_H
#define SERIALAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"

/*********************************************************************
 * CONSTANTS
 */

// These constants are only for example and should be changed to the
// device's needs
#define SERIALAPP_ENDPOINT           11

#define SERIALAPP_PROFID             0x0F05
#define SERIALAPP_DEVICEID           0x0001
#define SERIALAPP_DEVICE_VERSION     0
#define SERIALAPP_FLAGS              0

#define SERIALAPP_MAX_CLUSTERS                      6
#define SERIALAPP_CLUSTER_DATA_TRANS_REQ            1
#define SERIALAPP_CLUSTER_DATA_TRANS_RESP           2
#define SERIALAPP_CLUSTER_EP_CONNECT_REQ            3  
#define SERIALAPP_CLUSTER_EP_CONNECT_RSP            4    
#define SERIALAPP_CLUSTER_COMMAND_REQ               5    
#define SERIALAPP_CLUSTER_COMMAND_RSP               6    

/* 
    user event from 0x0001 to 0x4000，采用独热码方式，多少bit代码多少位，
    六状态的独热码可以表示成0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, ……, 0x4000
*/
/* ep send event */
#define SERIALAPP_SEND_EVT                  0x0001

/* coordi response event */
#define SERIALAPP_RESP_EVT                  0x0002

/* reRegist timer */
#define SERIALAPP_REGIST_EVT                0x0004

/* regist ack check */
#define SERIALAPP_REGIST_ACK_CHECK_EVT      0x0008

/* w5100 init event */
#define SERIALAPP_INIT_W5100_EVT            0x0010

/* w5100 connect event */
#define SERIALAPP_TCP_CONN_EVT              0x0020

/* comm query event for connected 485 or other device */
#define SERIALAPP_QUERY_REMOTE_EVT          0x0040

// OTA Flow Control Delays
#define SERIALAPP_ACK_DELAY          1
#define SERIALAPP_NAK_DELAY          16

/* regist timer delay(100ms) */
#define SERIALAPP_REGIST_TIMER_DELAY                100

/* regist ack check timer delay(5s) */
#define SERIALAPP_REGIST_ACK_CHECK_TIMER_DELAY      5000

/* query event timer default value */
#define SERIALAPP_QUERY_EVENT_TIMER                 5000

#define USING_W5100                         FALSE   /* default using uart */

#define SERIALAPP_TRANS_NEED_ACK            FALSE   /* default FALSE */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern byte SerialApp_TaskID;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Serial Transfer Application
 */
extern void SerialApp_Init( byte task_id );

/*
 * Task Event Processor for the Serial Transfer Application
 */
extern UINT16 SerialApp_ProcessEvent( byte task_id, UINT16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SERIALAPP_H */
