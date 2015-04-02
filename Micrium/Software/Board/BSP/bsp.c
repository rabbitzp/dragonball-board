/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                            (c) Copyright 2007-2008; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_MODULE
#include <bsp.h>
#include <fsmc_nand.h>
#include <bsp_i2c.h>

/* include user define headers */
#include <string.h>
#include <user_task_console.h>
#include <user_task_uart.h>
#include <user_task_gps.h>
#include "nmea_parser.h"


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/
                                                                /* -------------------- GPIOA PINS -------------------- */
#define  BSP_GPIOA_PB_WAKEUP                      DEF_BIT_00
#define  BSP_GPIOA_USART2_CTS                     DEF_BIT_00    /* USART2.                                              */
#define  BSP_GPIOA_USART2_RTS                     DEF_BIT_01    /* USART2.                                              */
#define  BSP_GPIOA_USART2_TX                      DEF_BIT_02    /* USART2.                                              */
#define  BSP_GPIOA_USART2_RX                      DEF_BIT_03    /* USART2.                                              */
#define  BSP_GPIOA_LCD_BL                         DEF_BIT_08    /* LCD.                                                 */
#define  BSP_GPIOA_USART1_TX                      DEF_BIT_09    /* USART1.                                              */
#define  BSP_GPIOA_USART1_RX                      DEF_BIT_10    /* USART1.                                              */
#define  BSP_GPIOA_USB_DM                         DEF_BIT_11    /* USB.                                                 */
#define  BSP_GPIOA_USB_DP                         DEF_BIT_12    /* USB.                                                 */

                                                                /* -------------------- GPIOB PINS -------------------- */
#define  BSP_GPIOB_SMARTCARD_PWR_SEL              DEF_BIT_00    /* Smartcard.                                           */
#define  BSP_GPIOB_SMARTCARD_IOUC                 DEF_BIT_10    /* Smartcard.                                           */
#define  BSP_GPIOB_SMARTCARD_RSTIN                DEF_BIT_11    /* Smartcard.                                           */
#define  BSP_GPIOB_SMARTCARD_XTAL1                DEF_BIT_12    /* Smartcard.                                           */
#define  BSP_GPIOB_USB_DISCONNECT                 DEF_BIT_14    /* USB.                                                 */

                                                                /* -------------------- GPIOC PINS -------------------- */
#define  BSP_GPIOC_ANALOG_CN5                     DEF_BIT_01    /* Analog Input.                                        */
#define  BSP_GPIOC_ANALOG_CN3                     DEF_BIT_02    /* Analog Input.                                        */
#define  BSP_GPIOC_ANALOG_CN2                     DEF_BIT_03    /* Analog Input.                                        */
#define  BSP_GPIOC_POT                            DEF_BIT_04    /* Potentiometer.                                       */
#define  BSP_GPIOC_SMARTCARD_CMDVCC               DEF_BIT_06    /* Smartcard.                                           */
#define  BSP_GPIOC_SMARTCARD_OFF                  DEF_BIT_07    /* Smartcard.                                           */
#define  BSP_GPIOC_MICROSD_D0                     DEF_BIT_08    /* MicroSD.                                             */
#define  BSP_GPIOC_MICROSD_D1                     DEF_BIT_09    /* MicroSD.                                             */
#define  BSP_GPIOC_MICROSD_D2                     DEF_BIT_10    /* MicroSD.                                             */
#define  BSP_GPIOC_MICROSD_D3                     DEF_BIT_11    /* MicroSD.                                             */
#define  BSP_GPIOC_MICROSD_CLK                    DEF_BIT_12    /* MicroSD.                                             */
#define  BSP_GPIOC_PB_TAMPER                      DEF_BIT_13

                                                                /* -------------------- GPIOD PINS -------------------- */
#define  BSP_GPIOD_MICROSD_CMD                    DEF_BIT_02    /* MicroSD.                                             */
#define  BSP_GPIOD_JOY_DOWN                       DEF_BIT_03    /* Joystick.                                            */

                                                                /* -------------------- GPIOF PINS -------------------- */
#define  BSP_GPIOF_LED1                           DEF_BIT_06    /* LED.                                                 */
#define  BSP_GPIOF_LED2                           DEF_BIT_07    /* LED.                                                 */
#define  BSP_GPIOF_LED3                           DEF_BIT_08    /* LED.                                                 */
#define  BSP_GPIOF_LED4                           DEF_BIT_09    /* LED.                                                 */
#define  BSP_GPIOA_LCD_CD                         DEF_BIT_10    /* LCD.                                                 */
#define  BSP_GPIOF_MICROSD_DETECT                 DEF_BIT_11    /* MicroSD.                                             */

                                                                /* -------------------- GPIOG PINS -------------------- */
#define  BSP_GPIOG_FSMC_INT2                      DEF_BIT_06    /* FSMC.                                                */
#define  BSP_GPIOG_JOY_SEL                        DEF_BIT_07    /* Joystick.                                            */
#define  BSP_GPIOG_PB_KEY                         DEF_BIT_08    /* Button.                                              */
#define  BSP_GPIOG_FSMC_NE2                       DEF_BIT_09    /* FSMC.                                                */
#define  BSP_GPIOG_FSMC_NE3                       DEF_BIT_10    /* FSMC.                                                */
#define  BSP_GPIOG_AUDIO_PDN                      DEF_BIT_11    /* Audio.                                               */
#define  BSP_GPIOG_FSMC_NE4                       DEF_BIT_12    /* FSMC.                                                */
#define  BSP_GPIOG_JOY_RIGHT                      DEF_BIT_13    /* Joystick.                                            */
#define  BSP_GPIOG_JOY_LEFT                       DEF_BIT_14    /* Joystick.                                            */
#define  BSP_GPIOG_JOY_UP                         DEF_BIT_15    /* Joystick.                                            */

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/
#define MAX_CONSOLE_MSG__BUFF_SIZE              255

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
#define MAX_GPS_RECV_MSG_BUFF_SIZE              255
#endif

/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
static vu8  g_SBUF_CONRXArray[MAX_CONSOLE_MSG__BUFF_SIZE];
static vu8  g_CONArryPos = 0;

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
static vu8  g_SBUF_GPSRXArray[MAX_GPS_RECV_MSG_BUFF_SIZE];
static vu8  g_GPSArryPos = 0;
#endif

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
#if 0
// jiaozi no need
static  void  BSP_ADC_Init     (void);

static  void  BSP_Joystick_Init(void);

static  void  BSP_LED_Init     (void);

static  void  BSP_PB_Init      (void);
#endif
/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/



#if defined(ENABLE_UART_DEBUG_CONSOLE) && (1 == ENABLE_UART_DEBUG_CONSOLE)

// jiaozi 150113
#include <stdio.h>

#pragma import(__use_no_semihosting)      
//void _ttywrch(int ch)  
//{ 
//    while((USART1->SR&0X40)==0);//ѭ������,ֱ���������    
//    USART1->DR = (u8) ch;  
//}

//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
    int handle; 
}; 

/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;

//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
    x = x; 
} 

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{
    /* send data */
    USART_SendData(USART1, (u8) ch);

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }

    return ch;
}

#if 1
// jiaozi for usart1

//����1�жϷ������
void USART1_IRQHandler(void)
{
    OS_CPU_SR   cpu_sr;
    u8          ch          = 0;
	u8          *pcBuff     = NULL;

    OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();


	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //���յ�����
	{
		ch = USART_ReceiveData(USART1);	//��ȡ���յ�������
		USART_ClearFlag(USART1, USART_IT_RXNE);      

        /* write back */
        if (ASCII_BACK == ch)
        {
            if (g_CONArryPos > 0)
            {
                g_CONArryPos--;
                printf("\b \b");
            }
        }
        else
        {
            printf("%c", ch);
        }

        /* check boundary */
        if (g_CONArryPos >= MAX_CONSOLE_MSG__BUFF_SIZE)
        {
            g_CONArryPos = 0;
        }        

        /* allow chars */
        if (((ASCII_SPACE <= ch) && (ASCII_DEL > ch)) ||
           (ASCII_CR == ch) || (ASCII_LF == ch))
        {
            /* append buffer */
            g_SBUF_CONRXArray[g_CONArryPos++] = ch;

            /* check end '\r\n' */
            if ((ASCII_LF == ch) && (ASCII_CR == g_SBUF_CONRXArray[g_CONArryPos - 2]))
            {
                /* alloc buffer */
        	    pcBuff = malloc(g_CONArryPos + 1);
                if (NULL != pcBuff)
                {
                    /* clear */
                    memset(pcBuff, 0, g_CONArryPos + 1);

                    /* copy */
                    memcpy(pcBuff, (char *) g_SBUF_CONRXArray, g_CONArryPos);
                    
                    /* pass to up level process */
                    OSQPost(g_QSemConsoleMsgRecv, (void *) pcBuff);
                }
                        
                g_CONArryPos = 0;
            }
        }       
	}

    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
    
	return;
}

//=========================================================

void db_uart1_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
		
	USART_DeInit(USART1);  //��λ����1
	
	//USART1_TX   PA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
	//USART1_RX	  PA.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10
  
	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;	//���ò����ʣ�һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART1, &USART_InitStructure); //��ʼ������
		
	USART1->CR1 |= 1 << 8;	//PE�ж�ʹ��
	USART1->CR1 |= 1 << 5;	//���ջ������ǿ��ж�ʹ��

	BSP_IntVectSet(BSP_INT_ID_USART1, USART1_IRQHandler);
	BSP_IntPrioSet(BSP_INT_ID_USART1, 2);
	BSP_IntEn(BSP_INT_ID_USART1);
	
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���

	printf("\n%s\n", __FUNCTION__);
	
}

#endif
#if 1
//����2�жϷ������, jiaozi for stm 2 cc2530
void USART2_IRQHandler(void)
{
    OS_CPU_SR   cpu_sr;
	u8          *pcBuff = NULL;

    OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //���յ�����
	{
        USART_ClearFlag(USART2, USART_IT_RXNE);
        
	    pcBuff = malloc(1);        
		pcBuff[0] = USART_ReceiveData(USART2);	//��ȡ���յ�������	

        //printf("%02X ", pcBuff[0]);
		
        /* pass to up level process */
        OSQPost(g_QSemUart2MsgRecv, (void *) pcBuff);
	}

    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
    
	return;
}

//=========================================================


void db_uart2_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

    /* ��ʼ���ܽ�ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /* ��ʼ��ͨѶ���� */	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 ,ENABLE);
	USART_DeInit(USART2);  //��λ����1
	
	//USART1_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	//USART1_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;	//���ò����ʣ�һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART2, &USART_InitStructure); //��ʼ������
		
	USART2->CR1 |= 1 << 8;	//PE�ж�ʹ��
	USART2->CR1 |= 1 << 5;	//���ջ������ǿ��ж�ʹ��

	BSP_IntVectSet(BSP_INT_ID_USART2, USART2_IRQHandler);
	BSP_IntPrioSet(BSP_INT_ID_USART2, 2);
	BSP_IntEn(BSP_INT_ID_USART2);
	
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���

	printf("\n%s\n", __FUNCTION__);
	
}

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
void USART3_IRQHandler(void)
{
    OS_CPU_SR   cpu_sr;
    u8          ch;
    nmeaGPRMC   *pInfo      = NULL;    
    nmeaINFO    *pnmeaInfo  = NULL;

    OS_ENTER_CRITICAL();                         /* Tell uC/OS-II that we are starting an ISR          */
    OSIntNesting++;
    OS_EXIT_CRITICAL();
    
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //���յ�����
	{
        USART_ClearFlag(USART3, USART_IT_RXNE);

        ch = USART_ReceiveData(USART3);	//��ȡ���յ�������

        //printf("%c", ch);
        
        if (g_GPSArryPos >= MAX_GPS_RECV_MSG_BUFF_SIZE)
        {
            g_GPSArryPos = 0;
        }

        /* check gps '$' */
        if ('$' == ch)
        {
            g_GPSArryPos = 0;
        }
        else
        {
            g_GPSArryPos++;
        }

        /* append buffer */
        g_SBUF_GPSRXArray[g_GPSArryPos] = ch;

        /* check end '\r' */
        if ('\n' == ch)
        {
            g_SBUF_GPSRXArray[++g_GPSArryPos] = 0;
            
            /* prepare buff */
            pInfo = (nmeaGPRMC *) malloc(sizeof(nmeaGPRMC));
            memset(pInfo, 0, sizeof(nmeaGPRMC));
            
            /* parse info */
        	if (0 == nmea_parse((char *) g_SBUF_GPSRXArray, g_GPSArryPos, pInfo))
        	{
        	    /* ignore invalid gps data */
                if ('A' == pInfo->status)
                {
                    pnmeaInfo = (nmeaINFO *) malloc(sizeof(nmeaINFO));
                    memset(pnmeaInfo, 0, sizeof(nmeaINFO));

                    /* asign values */
                    memcpy(&pnmeaInfo->utc, &pInfo->utc, sizeof(nmeaTIME));                 /**< UTC time */
                	pnmeaInfo->status = pInfo->status;			                            /**< Status (A = active or V = void) */
                	pnmeaInfo->ns = pInfo->ns;				                                /**< [N]orth or [S]outh */
                	pnmeaInfo->ew = pInfo->ew;				                                /**< [E]ast or [W]est */
                	pnmeaInfo->lat = atof((char *) pInfo->lat);	                            /**< Latitude in NDEG - [degree][min].[sec/60] */
                	pnmeaInfo->lon = atof((char *) pInfo->lon);	                            /**< Longitude in NDEG - [degree][min].[sec/60] */
                	pnmeaInfo->speed = atof((char *) pInfo->speed) * NMEA_TUD_KNOTS;	    /**< Speed over the ground in knots */    
                    pnmeaInfo->course = atof((char *) pInfo->course);                       /**< direction, 000.0~359.9, base N */

            		/* trans to degree */
        	    	pnmeaInfo->lat = (double)((int)pnmeaInfo->lat / 100) + (double)(pnmeaInfo->lat - ((int)pnmeaInfo->lat / 100) * 100) / 60.00;
    		        pnmeaInfo->lon = (double)((int)pnmeaInfo->lon / 100) + (double)(pnmeaInfo->lon - ((int)pnmeaInfo->lon / 100) * 100) / 60.00;

                    /* pass to up level process */
                    OSQPost(g_QSemGPSMsgRecv, (void *) pnmeaInfo);
                }                
        	}

            /* clean buff */
            free(pInfo);            
        }        		
	}

    OSIntExit();                                 /* Tell uC/OS-II that we are leaving the ISR          */
    
	return;
}

void db_uart3_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 ,ENABLE);
    
	USART_DeInit(USART3);  //��λ����3
	
	//USART3_TX   PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
   
	//USART3_RX	  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);
  
	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;	//���ò����ʣ�һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;	//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	
	USART_Init(USART3, &USART_InitStructure); //��ʼ������
		
	USART3->CR1 |= 1 << 8;	//PE�ж�ʹ��
	USART3->CR1 |= 1 << 5;	//���ջ������ǿ��ж�ʹ��

	BSP_IntVectSet(BSP_INT_ID_USART3, USART3_IRQHandler);
	BSP_IntPrioSet(BSP_INT_ID_USART3, 2);
	BSP_IntEn(BSP_INT_ID_USART3);
	
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���

	printf("\n%s\n", __FUNCTION__);
	
}
#endif /* #if (DEF_ENABLED == OS_USER_GPS_SUPPORT) */
#endif

void  BSP_Init (void)
{
	BSP_IntInit();

    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    RCC_WaitForHSEStartUp();


    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div2);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    FLASH_SetLatency(FLASH_Latency_2);
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {
        ;
    }

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    while (RCC_GetSYSCLKSource() != 0x08) {
        ;
    }
    return;
}

#endif


/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/

void  BSP_Init_post (void)
{
    /* init debug console uart1 */
    db_uart1_init(115200);

    /* init uart2 for cc2530 */
    db_uart2_init(9600);

#if (DEF_ENABLED == OS_USER_GPS_SUPPORT)
    /* init uart3 for gps */
    db_uart3_init(9600);
#endif

    /* init nand flash */
    db_nand_init();

    /* init i2c bus */
    i2c_InitBus();

	// ������Ҫ
    //BSP_ADC_Init();                                             /* Initialize the I/Os for the ADC      controls.       */
    //BSP_LED_Init();                                             /* Initialize the I/Os for the LED      controls.       */
    //BSP_PB_Init();                                              /* Initialize the I/Os for the PB       control.        */
    //BSP_Joystick_Init();                                        /* Initialize the I/Os for the Joystick control.        */    
}


/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         OS CORTEX-M3 FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         OS_CPU_SysTickClkFreq()
*
* Description : Get system tick clock frequency.
*
* Argument(s) : none.
*
* Return(s)   : Clock frequency (of system tick).
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

INT32U  OS_CPU_SysTickClkFreq (void)
{
    INT32U  freq;


    freq = BSP_CPU_ClkFreq();
    return (freq);
}

#if 0
// jiaozi noneed
/*
*********************************************************************************************************
*********************************************************************************************************
*                                              ADC FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_ADC_Init()
*
* Description : Initialize the board's ADC
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_ADC_Init (void)
{
    ADC_InitTypeDef   adc_init;
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin  = GPIO_Pin_4;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &gpio_init);

    adc_init.ADC_Mode               = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode       = DISABLE;
    adc_init.ADC_ContinuousConvMode = ENABLE;
    adc_init.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign          = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC1, &adc_init);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_13Cycles5);
    ADC_Cmd(ADC1, ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/*
*********************************************************************************************************
*                                           BSP_ADC_GetStatus()
*
* Description : This function initializes the board's ADC
*
* Argument(s) : adc             ID of the ADC to probe.  For this board, the only legitimate value is 1.
*
* Return(s)   : The numerator of the binary fraction representing the result of the latest ADC conversion.
*               This value will be a 12-bit value between 0x0000 and 0x0FFF, inclusive.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16U  BSP_ADC_GetStatus (CPU_INT08U  adc)
{
    CPU_INT16U  result;


    result = 0;

    if (adc == 1) {
        result = ADC_GetConversionValue(ADC1);
    }

    return (result);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               PB FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              BSP_PB_Init()
*
* Description : Initialize the board's PB.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_PB_Init (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    gpio_init.GPIO_Pin  = BSP_GPIOA_PB_WAKEUP;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_init);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio_init.GPIO_Pin  = BSP_GPIOC_PB_TAMPER;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio_init);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    gpio_init.GPIO_Pin  = BSP_GPIOG_PB_KEY;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOG, &gpio_init);
}

/*
*********************************************************************************************************
*                                           BSP_PB_GetStatus()
*
* Description : Get the status of a push button on the board.
*
* Argument(s) : pb      The ID of the push button to probe
*
*                       1    probe the user push button
*
* Return(s)   : DEF_FALSE   if the push button is pressed.
*               DEF_TRUE    if the push button is not pressed.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_PB_GetStatus (CPU_INT08U pb)
{
    CPU_BOOLEAN  status;
    CPU_INT32U   pin;


    status = DEF_FALSE;

    switch (pb) {
        case BSP_PB_ID_KEY:
             pin = GPIO_ReadInputDataBit(GPIOG, BSP_GPIOG_PB_KEY);
             if (pin == 0) {
                 status = DEF_TRUE;
             }
             break;

        case BSP_PB_ID_WAKEUP:
             pin = GPIO_ReadInputDataBit(GPIOA, BSP_GPIOA_PB_WAKEUP);
             if (pin == 0) {
                 status = DEF_TRUE;
             }
             break;

        case BSP_PB_ID_TAMPER:
             pin = GPIO_ReadInputDataBit(GPIOC, BSP_GPIOC_PB_TAMPER);
             if (pin == 0) {
                 status = DEF_TRUE;
             }
             break;

        default:
             break;
    }

    return (status);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                           JOYSTICK FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           BSP_Joystick_Init()
*
* Description : Initialize the board's joystick.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_Joystick_Init (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);

                                                                /* Configure JOY_UP, JOY_SEL, JOY_LEFT, JOY_RIGHT       */
    gpio_init.GPIO_Pin  = BSP_GPIOG_JOY_UP | BSP_GPIOG_JOY_SEL | BSP_GPIOG_JOY_LEFT | BSP_GPIOG_JOY_RIGHT;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOG, &gpio_init);

                                                                /* Configure JOY_DOWN.                                  */
    gpio_init.GPIO_Pin  = BSP_GPIOD_JOY_DOWN;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &gpio_init);
}

/*
*********************************************************************************************************
*                                        BSP_Joystick_GetStatus()
*
* Description : Get the status of the joystick on the board.
*
* Argument(s) : none.
*
* Return(s)   : Bit-mapped status of joystick :
*
*                   BSP_JOYSTICK_CENTER     if the joystick is being pressed.
*                   BSP_JOYSTICK_LEFT       if the joystick is toggled left.
*                   BSP_JOYSTICK_RIGHT      if the joystick is toggled right.
*                   BSP_JOYSTICK_UP         if the joystick is toggled up.
*                   BSP_JOYSTICK_DOWN       if the joystick is toggled down.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_Joystick_GetStatus (void)
{
    CPU_BOOLEAN  pin;
    CPU_BOOLEAN  status;

    status = 0;

    pin    = GPIO_ReadInputDataBit(GPIOG, BSP_GPIOG_JOY_RIGHT);
    if (pin == 0) {
        status |= BSP_JOYSTICK_LEFT;
    }

    pin    = GPIO_ReadInputDataBit(GPIOG, BSP_GPIOG_JOY_LEFT);
    if (pin == 0) {
        status |= BSP_JOYSTICK_RIGHT;
    }

    pin    = GPIO_ReadInputDataBit(GPIOG, BSP_GPIOG_JOY_UP);
    if (pin == 0) {
        status |= BSP_JOYSTICK_UP;
    }

    pin    = GPIO_ReadInputDataBit(GPIOD, BSP_GPIOD_JOY_DOWN);
    if (pin == 0) {
        status |= BSP_JOYSTICK_DOWN;
    }

    pin    = GPIO_ReadInputDataBit(GPIOG, BSP_GPIOG_JOY_SEL);
    if (pin == 0) {
        status |= BSP_JOYSTICK_CENTER;
    }

    return (status);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LED FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_LED_Init()
*
* Description : Initialize the I/O for the LEDs
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_LED_Init (void)
{
    GPIO_InitTypeDef  gpio_init;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

    gpio_init.GPIO_Pin   = BSP_GPIOF_LED1 | BSP_GPIOF_LED2 | BSP_GPIOF_LED3 | BSP_GPIOF_LED4;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOF, &gpio_init);
}

/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turn ON all LEDs on the board
*                       1    turn ON LED 1
*                       2    turn ON LED 2
*                       3    turn ON LED 3
*                       4    turn ON LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_On (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_SetBits(GPIOF, BSP_GPIOF_LED1 | BSP_GPIOF_LED2 | BSP_GPIOF_LED3 | BSP_GPIOF_LED4);
             break;

        case 1:
             GPIO_SetBits(GPIOF, BSP_GPIOF_LED1);
             break;

        case 2:
             GPIO_SetBits(GPIOF, BSP_GPIOF_LED2);
             break;

        case 3:
             GPIO_SetBits(GPIOF, BSP_GPIOF_LED3);
             break;

        case 4:
             GPIO_SetBits(GPIOF, BSP_GPIOF_LED4);
             break;

        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                              BSP_LED_Off()
*
* Description : Turn OFF any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turn OFF all LEDs on the board
*                       1    turn OFF LED 1
*                       2    turn OFF LED 2
*                       3    turn OFF LED 3
*                       4    turn OFF LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off (CPU_INT08U led)
{
    switch (led) {
        case 0:
             GPIO_ResetBits(GPIOF, BSP_GPIOF_LED1 | BSP_GPIOF_LED2 | BSP_GPIOF_LED3 | BSP_GPIOF_LED4);
             break;

        case 1:
             GPIO_ResetBits(GPIOF, BSP_GPIOF_LED1);
             break;

        case 2:
             GPIO_ResetBits(GPIOF, BSP_GPIOF_LED2);
             break;

        case 3:
             GPIO_ResetBits(GPIOF, BSP_GPIOF_LED3);
             break;

        case 4:
             GPIO_ResetBits(GPIOF, BSP_GPIOF_LED4);
             break;

        default:
             break;
    }
}

/*
*********************************************************************************************************
*                                            BSP_LED_Toggle()
*
* Description : TOGGLE any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    TOGGLE all LEDs on the board
*                       1    TOGGLE LED 1
*                       2    TOGGLE LED 2
*                       3    TOGGLE LED 3
*                       4    TOGGLE LED 4
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Toggle (CPU_INT08U led)
{
    CPU_INT32U  pins;


    pins = GPIO_ReadOutputData(GPIOC);

    switch (led) {
        case 0:
             BSP_LED_Toggle(1);
             BSP_LED_Toggle(2);
             BSP_LED_Toggle(3);
             BSP_LED_Toggle(4);
             break;

        case 1:
             if ((pins & BSP_GPIOF_LED1) == 0) {
                 GPIO_SetBits(  GPIOF, BSP_GPIOF_LED1);
             } else {
                 GPIO_ResetBits(GPIOF, BSP_GPIOF_LED1);
             }
            break;

        case 2:
             if ((pins & BSP_GPIOF_LED2) == 0) {
                 GPIO_SetBits(  GPIOF, BSP_GPIOF_LED2);
             } else {
                 GPIO_ResetBits(GPIOF, BSP_GPIOF_LED2);
             }
            break;

        case 3:
             if ((pins & BSP_GPIOF_LED3) == 0) {
                 GPIO_SetBits(  GPIOF, BSP_GPIOF_LED3);
             } else {
                 GPIO_ResetBits(GPIOF, BSP_GPIOF_LED3);
             }
            break;

        case 4:
             if ((pins & BSP_GPIOF_LED4) == 0) {
                 GPIO_SetBits(  GPIOF, BSP_GPIOF_LED4);
             } else {
                 GPIO_ResetBits(GPIOF, BSP_GPIOF_LED4);
             }
            break;

        default:
             break;
    }
}
#endif
/*
*********************************************************************************************************
*********************************************************************************************************
*                                            MEMORY FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_NAND_Init()
*
* Description : Initialize FSMC for NAND flash.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_NAND_Init (void)
{
    GPIO_InitTypeDef                   gpio_init;
    FSMC_NANDInitTypeDef               nand_init;
    FSMC_NAND_PCCARDTimingInitTypeDef  p;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);

                                                                /* ---------------------- CFG GPIO -------------------- */
                                                                /* CLE, ALE, D0..3, NOW, NWE & NCE2 NAND pin cfg.       */
    gpio_init.GPIO_Pin   =  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15 |
                            GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_7;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOD, &gpio_init);

                                                                /* D4..7 NAND pin configuration.                        */
    gpio_init.GPIO_Pin   = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;

    GPIO_Init(GPIOE, &gpio_init);


                                                                /* NWAIT NAND pin configuration.                        */
    gpio_init.GPIO_Pin   = GPIO_Pin_6;   							
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_IPU;

    GPIO_Init(GPIOD, &gpio_init);

                                                                /* INT2 NAND pin configuration.                         */
    gpio_init.GPIO_Pin   = GPIO_Pin_6;   							
    GPIO_Init(GPIOG, &gpio_init);

                                                                /* ---------------------- CFG FSMC -------------------- */
    p.FSMC_SetupTime                          = 0x1;
    p.FSMC_WaitSetupTime                      = 0x3;
    p.FSMC_HoldSetupTime                      = 0x2;
    p.FSMC_HiZSetupTime                       = 0x1;

    nand_init.FSMC_Bank                       = FSMC_Bank2_NAND;
    nand_init.FSMC_Waitfeature                = FSMC_Waitfeature_Enable;
    nand_init.FSMC_MemoryDataWidth            = FSMC_MemoryDataWidth_8b;
    nand_init.FSMC_ECC                        = FSMC_ECC_Enable;
    nand_init.FSMC_ECCPageSize                = FSMC_ECCPageSize_512Bytes;
    nand_init.FSMC_AddressLowMapping          = FSMC_AddressLowMapping_Direct;
    nand_init.FSMC_TCLRSetupTime              = 0x00;
    nand_init.FSMC_TARSetupTime               = 0x00;
    nand_init.FSMC_CommonSpaceTimingStruct    = &p;
    nand_init.FSMC_AttributeSpaceTimingStruct = &p;

    FSMC_NANDInit(&nand_init);

    FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);                      /* FSMC NAND bank cmd test.                             */
}

/*
*********************************************************************************************************
*                                             BSP_NOR_Init()
*
* Description : Initialize FSMC for NOR flash.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_NOR_Init (void)
{
    GPIO_InitTypeDef               gpio_init;
    FSMC_NORSRAMInitTypeDef        nor_init;
    FSMC_NORSRAMTimingInitTypeDef  p;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);

                                                                /* ---------------------- CFG GPIO -------------------- */
                                                                /* NOR data lines configuration.                        */
    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                           GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &gpio_init);

                                                                /* NOR address lines configuration.                     */
    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_4 |
                           GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOF, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOG, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOD, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_3  | GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_6;
    GPIO_Init(GPIOE, &gpio_init);

                                                                /* NOE and NWE configuration.                           */
    gpio_init.GPIO_Pin   = GPIO_Pin_4  | GPIO_Pin_5;
    GPIO_Init(GPIOD, &gpio_init);

                                                                /* NE2 configuration.                                   */
    gpio_init.GPIO_Pin   = GPIO_Pin_9;
    GPIO_Init(GPIOG, &gpio_init);

                                                                /* ---------------------- CFG FSMC -------------------- */
    p.FSMC_AddressSetupTime             = 0x05;
    p.FSMC_AddressHoldTime              = 0x00;
    p.FSMC_DataSetupTime                = 0x07;
    p.FSMC_BusTurnAroundDuration        = 0x00;
    p.FSMC_CLKDivision                  = 0x00;
    p.FSMC_DataLatency                  = 0x00;
    p.FSMC_AccessMode                   = FSMC_AccessMode_B;

    nor_init.FSMC_Bank                  = FSMC_Bank1_NORSRAM2;
    nor_init.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
    nor_init.FSMC_MemoryType            = FSMC_MemoryType_NOR;
    nor_init.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;
    nor_init.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
    nor_init.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
    nor_init.FSMC_WrapMode              = FSMC_WrapMode_Disable;
    nor_init.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
    nor_init.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
    nor_init.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
    nor_init.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
    nor_init.FSMC_AsyncWait             = FSMC_AsyncWait_Disable;
    nor_init.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
    nor_init.FSMC_ReadWriteTimingStruct = &p;
    nor_init.FSMC_WriteTimingStruct     = &p;

    FSMC_NORSRAMInit(&nor_init);

                                                                /* Enable FSMC Bank1_NOR Bank.                          */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                           OS PROBE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           OSProbe_TmrInit()
*
* Description : Select & initialize a timer for use with the uC/Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : OSProbe_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
void  OSProbe_TmrInit (void)
{
    TIM_TimeBaseInitTypeDef  tim_init;


    tim_init.TIM_Period        = 0xFFFF;
    tim_init.TIM_Prescaler     = 256;
    tim_init.TIM_ClockDivision = TIM_CKD_DIV4;
    tim_init.TIM_CounterMode   = TIM_CounterMode_Up;

#if (OS_PROBE_TIMER_SEL == 2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInit(TIM2, &tim_init);
    TIM_SetCounter(TIM2, 0);
    TIM_PrescalerConfig(TIM2, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM2, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInit(TIM3, &tim_init);
    TIM_SetCounter(TIM3, 0);
    TIM_PrescalerConfig(TIM3, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM3, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 4)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInit(TIM4, &tim_init);
    TIM_SetCounter(TIM4, 0);
    TIM_PrescalerConfig(TIM4, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM4, ENABLE);
#endif
}
#endif

/*
*********************************************************************************************************
*                                            OSProbe_TmrRd()
*
* Description : Read the current counts of a 16-bit free running timer.
*
* Argument(s) : none.
*
* Return(s)   : The 16-bit counts of the timer in a 32-bit variable.
*
* Caller(s)   : OSProbe_TimeGetCycles().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
CPU_INT32U  OSProbe_TmrRd (void)
{
#if (OS_PROBE_TIMER_SEL == 2)

    return ((CPU_INT32U)TIM_GetCounter(TIM2));

#elif (OS_PROBE_TIMER_SEL == 3)

    return ((CPU_INT32U)TIM_GetCounter(TIM3));

#elif (OS_PROBE_TIMER_SEL == 4)

    return ((CPU_INT32U)TIM_GetCounter(TIM4));

#endif
}
#endif