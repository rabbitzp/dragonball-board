/*
*********************************************************************************************************
*
*                                      User os functions
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_os_func.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_OS_FUNC_H__
#define  __USER_OS_FUNC_H__
/*--------------macros declare here-----------------------------*/

/*--------------enums declare here------------------------------*/

/*--------------struct declare here-----------------------------*/

/*--------------functions declare here-----------------------------*/

u16     USART_Send(USART_TypeDef* USARTx, u16 usLen, void *pData);

void    SystemReset(void);

/*---------------extern vars & functinos here-----------------------*/
#endif

