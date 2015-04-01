/*
*********************************************************************************************************
*
*                                      User debug function
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_debug.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <stdio.h>
#include <stdarg.h>
#include <user_core.h>
#include <user_debug.h>

#if defined(ENABLE_UART_DEBUG_CONSOLE) && (1 == ENABLE_UART_DEBUG_CONSOLE)

void UCore_DebugPrint(char* fmt, ...)
{
    va_list list;

    /* check debug switch */
    if (1 != g_udev_config.enDebug)
    {
        return;
    }
    
    va_start(list, fmt);

    vprintf(fmt, list);

    va_end(list);    
}

#endif


