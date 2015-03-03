/*
*********************************************************************************************************
*
*                                      User gps receive task process
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_gps.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_TASK_GPS_H__
#define  __USER_TASK_GPS_H__
/*--------------macros declare here-----------------------------*/

/*--------------enums declare here------------------------------*/

/*--------------struct declare here-----------------------------*/

/*--------------functions declare here-----------------------------*/
u8  GPS_Start(void);

/*---------------extern vars & functinos here-----------------------*/
extern OS_EVENT *g_QSemGPSMsgRecv;

#endif

