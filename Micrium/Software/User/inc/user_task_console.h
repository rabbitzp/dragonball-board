/*
*********************************************************************************************************
*
*                                      User console task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_console.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_TASK_CONSOLE_H__
#define  __USER_TASK_CONSOLE_H__
/*----------------macros declare here---------------------*/
#define  ASCII_CR		0x0d
#define  ASCII_LF		0x0a
#define  ASCII_BS		0x08
#define  ASCII_BELL		0x07
#define  ASCII_TAB		0x09
#define  ASCII_XON		0x11
#define  ASCII_XOFF		0x13
#define  ASCII_ESC		0x1B
#define  ASCII_DEL		0x7F
#define	 ASCII_BACK		0x08
#define	 ASCII_SPACE    0x20

/*--------------enums declare here------------------------------*/

/*--------------struct declare here-----------------------------*/

/*--------------functions declare here-----------------------------*/
u8  UConsole_Start(void);

/*---------------extern vars & functinos here-----------------------*/
extern OS_EVENT *g_QSemConsoleMsgRecv;

#endif
