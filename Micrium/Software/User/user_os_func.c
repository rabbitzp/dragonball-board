/*
*********************************************************************************************************
*
*                                      User os functions
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_os_func.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <user_os_func.h>

u16 USART_Send(USART_TypeDef* USARTx, u16 usLen, void *pData)
{
    u16 i       = 0;
    u8  *pcBuff = NULL;
    
    if ((NULL == USARTx) || (NULL == pData) || (0 == usLen))
    {
        return 0;
    }

    /* asign buffer ptr */
    pcBuff = (u8 *)pData;
    
    for (i = 0; i < usLen; i++)
    {
        /* send to uart */
        USART_SendData(USARTx, pcBuff[i]);

        /* Loop until the end of transmission */
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    }

    return usLen;
}
