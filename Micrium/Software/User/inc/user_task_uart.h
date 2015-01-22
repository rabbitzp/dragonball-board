/*
*********************************************************************************************************
*
*                                      User uart task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_uart.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_TASK_UART_H__
#define  __USER_TASK_UART_H__
/*--------------macros declare here-----------------------------*/

/*--------------enums declare here------------------------------*/

/*--------------struct declare here-----------------------------*/

/*--------------functions declare here-----------------------------*/
u8  UUart_Start(void);

/*---------------extern vars & functinos here-----------------------*/
extern OS_EVENT *g_QSemUart2MsgRecv;

#endif

