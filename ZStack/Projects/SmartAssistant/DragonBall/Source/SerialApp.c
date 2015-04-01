/*********************************************************************
  Filename:       SerialApp.c
  Revised:        $Date: 2011-04-20 10:16:44 -0700 (Wed, 20 Apr 2011) $
  Revision:       $Revision: 25771 $

  Description:    Stub APS processing functions


  Copyright       2015- J.B.S All rights reserved.
*********************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include <stdio.h>
#include <string.h>
#include "AF.h"
#include "OnBoard.h"
#include "OSAL_Tasks.h"
#include "SerialApp.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "OSAL_NV.h"

#include "hal_drivers.h"
#include "hal_key.h"
#if defined ( LCD_SUPPORTED )
  #include "hal_lcd.h"
#endif
#include "hal_led.h"
#include "hal_uart.h"

#include "ZProfile\zprofile_def.h"

/*include jstack inc */
#include "ucore\ucore.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */



#if !defined( SERIAL_APP_BAUD )
  //#define SERIAL_APP_BAUD  HAL_UART_BR_38400
  #define SERIAL_APP_BAUD  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_APP_THRESH )
#define SERIAL_APP_THRESH  64
#endif

#if !defined( SERIAL_APP_RX_SZ )
#define SERIAL_APP_RX_SZ  128
#endif

#if !defined( SERIAL_APP_TX_SZ )
#define SERIAL_APP_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_APP_IDLE )
#define SERIAL_APP_IDLE  6
#endif



/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
uint8               SerialApp_TaskID;           // Task ID for internal task/event processing.


// This list should be filled with Application specific Cluster IDs.
const cId_t SerialApp_ClusterList[ZP_SA_MAX_CLUSTERS] =
{
  ZP_SA_CLUSTER_ID_GEN_BASIC,
  ZP_SA_CLUSTER_CONNECT_REQ,            
  ZP_SA_CLUSTER_CONNECT_RSP,
  ZP_SA_CLUSTER_COMMAND_REQ,
  ZP_SA_CLUSTER_COMMAND_RSP,
  ZP_SA_CLUSTER_KEEP_ALIVE
};

const SimpleDescriptionFormat_t SerialApp_SimpleDesc =
{
  ZP_SA_DRAGONBALL_ENDPOINT,          //  int   Endpoint;
  ZP_SA_PROFILE_ID,                     //  uint16 AppProfId[2];
  ZP_SA_DEVICEID_DRAGONBALL,            //  int   device id;
  ZP_DEVICE_APP_VERSION,                //  int   AppDevVer:4;
  ZP_DEVICE_APP_FLAGS,                  //  int   AppFlags:4;
  ZP_SA_MAX_CLUSTERS,                   //  byte  AppNumInClusters;
  (cId_t *)SerialApp_ClusterList,       //  byte *pAppInClusterList;
  ZP_SA_MAX_CLUSTERS,                   //  byte  AppNumOutClusters;
  (cId_t *)SerialApp_ClusterList        //  byte *pAppOutClusterList;
};

endPointDesc_t SerialApp_epDesc =
{
  ZP_SA_DRAGONBALL_ENDPOINT,          //  int   Endpoint;
  &SerialApp_TaskID,
  (SimpleDescriptionFormat_t *)&SerialApp_SimpleDesc,
  noLatencyReqs
};


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      SerialApp_Init
 *
 * @brief   This is called during OSAL tasks' initialization.
 *
 * @param   task_id - the Task ID assigned by OSAL.
 *
 * @return  none
 */
void SerialApp_Init( uint8 task_id )
{
    halUARTCfg_t uartConfig;

    SerialApp_TaskID = task_id;

    afRegister( (endPointDesc_t *)&SerialApp_epDesc );

    RegisterForKeys( task_id );

    uartConfig.configured           = TRUE;              // 2x30 don't care - see uart driver.
    uartConfig.baudRate             = HAL_UART_BR_9600;
    uartConfig.flowControl          = FALSE;
    uartConfig.flowControlThreshold = SERIAL_APP_THRESH; // 2x30 don't care - see uart driver.
    uartConfig.rx.maxBufSize        = SERIAL_APP_RX_SZ;  // 2x30 don't care - see uart driver.
    uartConfig.tx.maxBufSize        = SERIAL_APP_TX_SZ;  // 2x30 don't care - see uart driver.
    uartConfig.idleTimeout          = SERIAL_APP_IDLE;   // 2x30 don't care - see uart driver.
    uartConfig.intEnable            = TRUE;              // 2x30 don't care - see uart driver.
    uartConfig.callBackFunc         = UCore_SerialCallBack;
    HalUARTOpen (UCORE_EX_COMM_UART_PORT, &uartConfig);

    ZDO_RegisterForZDOMsg( SerialApp_TaskID, End_Device_Bind_rsp );
    ZDO_RegisterForZDOMsg( SerialApp_TaskID, Match_Desc_rsp );
  
    /* init user core */
    UCORE_Init(SerialApp_TaskID);

#if !ZDO_COORDINATOR    /* only allow endpoint and router keep alive from coordinator */
    /* start a keep alive timer */
    osal_start_timerEx( SerialApp_TaskID, UCORE_APP_EVENT_ID_KEEP_ALIVE, UCORE_APP_EVENT_KEEPALIVE_TIME );
#endif

}

/*********************************************************************
 * @fn      SerialApp_ProcessEvent
 *
 * @brief   Generic Application Task event processor.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events   - Bit map of events to process.
 *
 * @return  Event flags of all unprocessed events.
 */
UINT16 SerialApp_ProcessEvent( uint8 task_id, UINT16 events )
{
    uint8 i = 0;
        
    /* we search event processor */
    for (i=0; i<UCORE_MAX_EVENT_PROCESSOR; i++)
    {
        if (events & gCoreEventProcessor[i].usEventId)
        {        
            /* call event processor */
            if (UCORE_ERR_SUCCESS != gCoreEventProcessor[i].fEvProcessor((void *) &SerialApp_TaskID))
            {
                /* oops */
            }

            return (events ^ gCoreEventProcessor[i].usEventId);
        }
    }  
    
    return ( 0 );  // Discard unknown events.
}

