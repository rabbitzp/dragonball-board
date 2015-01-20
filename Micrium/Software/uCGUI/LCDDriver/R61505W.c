/*
*********************************************************************************************************
*
*	ģ������ : TFTҺ����ʾ������ģ��
*	�ļ����� : LCD_tft_lcd.c
*	��    �� : V1.0
*	˵    �� : ����������������TFT��ʾ���ֱ���Ϊ240x400��3.0���������PWM������ڹ��ܡ�
*				֧�ֵ�LCD�ڲ�����оƬ�ͺ��У�SPFD5420A��OTM4001A��R61509V
*				����оƬ�ķ��ʵ�ַΪ:  0x60000000
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-08-21 armfly  ST�̼���汾 V3.5.0�汾��
*					a) ȡ�����ʼĴ����Ľṹ�壬ֱ�Ӷ���
*		V2.0    2011-10-16 armfly  ����R61509V������ʵ��ͼ����ʾ����
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	������ʾ��
	TFT��������һ���12864������ʾ���Ŀ����������������������˴��ڻ�ͼ�Ļ��ƣ�������ƶ��ڻ��ƾֲ�ͼ��
	�Ƿǳ���Ч�ġ�TFT������ָ��һ����ͼ���ڣ�Ȼ�����еĶ�д�Դ�Ĳ��������������֮�ڣ����������ҪCPU��
	�ڴ��б���������Ļ���������ݡ�
*/

#include <includes.h>
#include <stdio.h>
#include <string.h>
#include "R61505W.h"

/* ����LCD�������ķ��ʵ�ַ
	TFT�ӿ��е�RS��������FSMC_A0���ţ�������16bitģʽ��RS��ӦA1��ַ�ߣ����
	LCD_RAM�ĵ�ַ��+2
*/
#define LCD_BASE        ((u32)(0x60000000 | 0x08000000))
#define LCD_REG			*(volatile u16 *)(LCD_BASE)
#define LCD_RAM			*(volatile u16 *)(LCD_BASE + 2)

/* ����LCD��ʾ����ķֱ��� */
#define LCD_HEIGHT		220		/* �߶ȣ���λ������ */
#define LCD_WIDTH		220		/* ��ȣ���λ������ */

/* LCD �Ĵ�������, LR_ǰ׺��LCD Register�ļ�д */
#define LR_CTRL1		0x007	/* ��д�Դ�ļĴ�����ַ */
#define LR_GRAM			0x22	/* ��д�Դ�ļĴ�����ַ */
#define LR_GRAM_X		0x200	/* �Դ�ˮƽ��ַ������X���꣩*/
#define LR_GRAM_Y		0x201	/* �Դ洹ֱ��ַ������Y���꣩*/

/*
������������
*/
static volatile u8 s_RGBChgEn = 0;		/* RGBת��ʹ��, 4001��д�Դ������RGB��ʽ��д��Ĳ�ͬ */
static volatile u8 s_AddrAutoInc = 0;	/* ����һ�����غ��Դ��ַ�Ƿ��Զ���1 */

static void LCD_R61505W_InitHard(void);
static void LCD_R61505W_SetCursor(u16 _usX, u16 _usY);
static u16  LCD_R61505W_BGR2RGB(u16 _usRGB);
static void LCD_R61505W_FSMCConfig(void);
static void LCD_R61505W_CtrlLinesConfig(void);
static void LCD_R61505W_WriteReg(volatile u16 _usAddr, u16 _usValue);
static u16  LCD_R61505W_ReadReg(volatile u16 _usAddr);

void LCD_R61505W_Delayms(u16 ms)
{
	vu32 start_time = OSTime;
	vu32 tick = 0;

	while (tick < 1+ms)
	{
		if (start_time  != OSTime)
		{
			tick++;
			start_time = OSTime;
		}
	}

	return;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_WriteReg
*	����˵��: �޸�LCD�������ļĴ�����ֵ��
*	��    �Σ�
*			LCD_Reg ���Ĵ�����ַ;
*			LCD_RegValue : �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_R61505W_WriteReg(volatile u16 _usAddr, u16 _usValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD_REG = _usAddr;
	/* Write 16-bit Reg */
	LCD_RAM = _usValue;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_ReadReg
*	����˵��: ��ȡLCD�������ļĴ�����ֵ��
*	��    �Σ�
*			LCD_Reg ���Ĵ�����ַ;
*			LCD_RegValue : �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static u16 LCD_R61505W_ReadReg(volatile u16 _usAddr)
{
	/* Write 16-bit Index (then Read Reg) */
	LCD_REG = _usAddr;
	/* Read 16-bit Reg */
	return (LCD_RAM);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_BGR2RGB
*	����˵��: RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
*	��    �Σ�RGB��ɫ����
*	�� �� ֵ: ת�������ɫ����
*********************************************************************************************************
*/
static u16 LCD_R61505W_BGR2RGB(u16 _usRGB)
{
	u16  r, g, b, rgb;

	b = (_usRGB >> 0)  & 0x1F;
	g = (_usRGB >> 5)  & 0x3F;
	r = (_usRGB >> 11) & 0x1F;

	rgb = (b<<11) + (g<<5) + (r<<0);

	return( rgb );
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_CtrlLinesConfig
*	����˵��: ����LCD���ƿ��ߣ�FSMC�ܽ�����Ϊ���ù���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_R61505W_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/* ʹ�� FSMC, GPIOD, GPIOE, GPIOF, GPIOG �� AFIO ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                     RCC_APB2Periph_AFIO, ENABLE);

	/* ���� PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* ���� PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) Ϊ����������� */
	/* PE3,PE4 ����A19, A20, STM32F103ZE-EK(REV 1.0)����ʹ�� */
	/* PE5,PE6 ����A19, A20, STM32F103ZE-EK(REV 2.0)����ʹ�� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* ���� PF.00(A0 (RS))  Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* ���� PG.12(NE4 (LCD/CS)) Ϊ����������� - CE3(LCD /CS) */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* ���� PG.10(NE3 (LCD/CS)) Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_R61505W_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  init;
	FSMC_NORSRAMTimingInitTypeDef  timing;

	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 3 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM3 configuration */
	timing.FSMC_AddressSetupTime = 1;
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 2;
	timing.FSMC_BusTurnAroundDuration = 0;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM3;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
//	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* ע��ɿ��������Ա */
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FSMC_ReadWriteTimingStruct = &timing;
	init.FSMC_WriteTimingStruct = &timing;

	FSMC_NORSRAMInit(&init);

	/* - BANK 3 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_GetID
*	����˵��: ��ȡLCD������ID
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
u16 LCD_R61505W_GetID(void)
{
	return LCD_R61505W_ReadReg(0x0000);
}

void LCD_R61505W_InitHard(void)
{
	LCD_R61505W_Delayms(5);	//DelayNS(1000);
	
	LCD_R61505W_WriteReg( 0x00, 0x0000);
	LCD_R61505W_WriteReg( 0x00, 0x0000 );
	LCD_R61505W_WriteReg( 0x00, 0x0000 );
	LCD_R61505W_WriteReg( 0x00, 0x0000 );
	LCD_R61505W_WriteReg( 0x00, 0x0000 );
	
	LCD_R61505W_WriteReg( 0xa4, 0x0001 );
	LCD_R61505W_Delayms(1);//DelayNS(20);

	LCD_R61505W_WriteReg( 0x60, 0x2700 );
	LCD_R61505W_WriteReg( 0x08, 0x0808 );

	LCD_R61505W_WriteReg( 0x30, 0x0b00 );
	LCD_R61505W_WriteReg( 0x31, 0x7d17 );
	LCD_R61505W_WriteReg( 0x32, 0x00a01 );
	LCD_R61505W_WriteReg( 0x33, 0xa03);
	LCD_R61505W_WriteReg( 0x34, 0x0000 );
	LCD_R61505W_WriteReg( 0x35, 0x0030a );
	LCD_R61505W_WriteReg( 0x36, 0x710a );
	LCD_R61505W_WriteReg( 0x37, 0x170d);
	LCD_R61505W_WriteReg( 0x38, 0x000b );
	LCD_R61505W_WriteReg( 0x39, 0x3333 );
	
	LCD_R61505W_WriteReg( 0x90, 0x0017 );
	
	LCD_R61505W_WriteReg( 0x0a, 0x0008);
	LCD_R61505W_Delayms(1);//DelayNS(20);
	LCD_R61505W_WriteReg( 0x10, 0x0730 );
	LCD_R61505W_Delayms(1);//	DelayNS(20);
	LCD_R61505W_WriteReg( 0x11, 0x0247 );
	LCD_R61505W_Delayms(1);//DelayNS(150);
	LCD_R61505W_WriteReg( 0x0e, 0x0020 );

	LCD_R61505W_WriteReg( 0x13, 0x1e00 );
	
	LCD_R61505W_WriteReg( 0x2a, 0x0056 );
	LCD_R61505W_WriteReg( 0x12, 0x0199 );
	LCD_R61505W_WriteReg( 0x12, 0x01b9 );


	LCD_R61505W_WriteReg( 0x50, 0x0000 );
	LCD_R61505W_WriteReg( 0x51, 0x00ef );
	LCD_R61505W_WriteReg( 0x52, 0x0000 );
	LCD_R61505W_WriteReg( 0x53, 0x013f );
	
	LCD_R61505W_WriteReg( 0x61, 0x0001 );
	LCD_R61505W_WriteReg( 0x6a, 0x0000 );
	
	LCD_R61505W_WriteReg( 0x80, 0x0000 );
	LCD_R61505W_WriteReg( 0x81, 0x0000 );
	LCD_R61505W_WriteReg( 0x82, 0x0000 );
	

	LCD_R61505W_WriteReg( 0x92, 0x0300 );
	LCD_R61505W_WriteReg( 0x93, 0x0005 );
	LCD_R61505W_WriteReg( 0x01, 0x0100 );
	LCD_R61505W_WriteReg( 0x02, 0x0200 );
	LCD_R61505W_WriteReg( 0x03, 0x1030 );
	LCD_R61505W_WriteReg( 0x0c, 0x0000 );
	LCD_R61505W_WriteReg( 0x0f, 0x0000 );

	
	LCD_R61505W_WriteReg( 0x20, 0x0000 );
	LCD_R61505W_WriteReg( 0x21, 0x0000 );

	LCD_R61505W_WriteReg( 0x07, 0x0100 );
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_Init
*	����˵��: ��ʼ��LCD
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_R61505W_Init(void)
{
	u16 id;

	/* ����LCD���ƿ���GPIO */
	LCD_R61505W_CtrlLinesConfig();

	/* ����FSMC�ӿڣ��������� */
	LCD_R61505W_FSMCConfig();

	/* FSMC���ú������ӳٲ��ܷ��������豸  */
	LCD_R61505W_Delayms(5);

	id = LCD_R61505W_GetID();  	/* ��ȡLCD����оƬID */

	printf("оƬID��0X%04X\r\n", id);

	printf("��ʼ��LCD \r\n");
	LCD_R61505W_InitHard();
}  

/*
*********************************************************************************************************
*	�� �� ��: LCD_R61505W_SetCursor
*	����˵��: ���ù��λ��
*	��    �Σ�_usX : X����; _usY: Y����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_R61505W_SetCursor(u16 _usX, u16 _usY)
{
	/*
		px��py ���������꣬ x��y����������
		ת����ʽ:
		py = 399 - x;
		px = y;
	*/
    _usX += 10;
	LCD_R61505W_WriteReg(0x0020, _usX);        //����ַ
	LCD_R61505W_WriteReg(0x0021, _usY);        //����ַ
} 

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetPixel
*	����˵��: ��ȡ(x,y)��������ص� 
*	��    �Σ�
*			_usX,_usY : ��������
*			_usColor  ��������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
u16 LCD_R61505W_GetPoint(u16 _usX,u16 _usY)
{
	u16 usRGB;

	LCD_R61505W_SetCursor(_usX, _usY);	/* ���ù��λ�� */

	/* ׼��д�Դ� */
	LCD_REG = LR_GRAM;

	usRGB = LCD_RAM;

	/* �� 16-bit GRAM Reg */
	if (s_RGBChgEn == 1)
	{
		usRGB = LCD_R61505W_BGR2RGB(usRGB);
	}

	return usRGB;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_PutPixel
*	����˵��: �����ص�д��(x,y)�����
*	��    �Σ�
*			_usX,_usY : ��������
*			_usColor  ��������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_R61505W_SetPoint(u16 _usX, u16 _usY, u16 _usColor)
{
	LCD_R61505W_SetCursor(_usX, _usY);	/* ���ù��λ�� */

	/* д�Դ� */
	LCD_REG = LR_GRAM;

	/* Write 16-bit GRAM Reg */
	LCD_RAM = _usColor;
}       


