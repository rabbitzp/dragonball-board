/*
*********************************************************************************************************
*
*                                      User debug function
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_debug.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#ifndef __USER_DEBUG_H__
#define __USER_DEBUG_H__

/*--------------functions declare here-----------------------------*/
#if defined(ENABLE_UART_DEBUG_CONSOLE) && (1 == ENABLE_UART_DEBUG_CONSOLE)
void UCore_DebugPrint(char* fmt, ...);

#define UCORE_DEBUG_PRINT UCore_DebugPrint
#else
#define UCORE_DEBUG_PRINT
#endif

#endif

