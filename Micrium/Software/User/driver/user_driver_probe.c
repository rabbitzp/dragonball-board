/*
*********************************************************************************************************
*
*                                      User driver probe 
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_driver_probe.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <user_driver_probe.h>
#include "driver_24Cxx.h"
#include "tfs.h"

u8 User_DriverProbe()
{
    u8 ucResult = UCORE_ERR_COMMON_FAILED;    

    /* Probe EEPROM driver */
    ucResult = Driver_AT24CXXProbe();
    if (UCORE_ERR_SUCCESS != ucResult)
    {
        UCORE_DEBUG_PRINT("Probe EEPROM failed.\r\n");
        
        return UCORE_ERR_NET_INIT_FAILED;
    }

    /* tfs init */
    if (TFS_RESULT_SUCESS == TFS_InstalFS(0, AT24C128, AT24CXX_ReadBytes, AT24CXX_WriteBytes))
    {
        if (TFS_RESULT_SUCESS != TFS_InitFS())
        {
            if (TFS_RESULT_SUCESS != TFS_Format())
            {
                UCORE_DEBUG_PRINT("TFS init failed.\r\n");
            }
        }
    }

    return UCORE_ERR_SUCCESS;
}

