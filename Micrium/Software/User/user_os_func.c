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

/*******************************************************************************
* Function Name  : SystemReset
* Description    : Only valid under ARM-COTEX M3
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
__asm void SystemReset(void)
{
 MOV R0, #1           //; 
 MSR FAULTMASK, R0    //; 清除FAULTMASK 禁止一切中断产生
 LDR R0, =0xE000ED0C  //;
 LDR R1, =0x05FA0004  //; 
 STR R1, [R0]         //; 系统软件复位   
 
deadloop
    B deadloop        //; 死循环使程序运行不到下面的代码
}

