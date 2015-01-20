/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : LCD_tft_lcd.c
*	版    本 : V1.0
*	说    明 : 安富莱开发板标配的TFT显示器分辨率为240x400，3.0寸宽屏，带PWM背光调节功能。
*				支持的LCD内部驱动芯片型号有：SPFD5420A、OTM4001A、R61509V
*				驱动芯片的访问地址为:  0x60000000
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 armfly  ST固件库版本 V3.5.0版本。
*					a) 取消访问寄存器的结构体，直接定义
*		V2.0    2011-10-16 armfly  增加R61509V驱动，实现图标显示函数
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	友情提示：
	TFT控制器和一般的12864点阵显示器的控制器最大的区别在于引入了窗口绘图的机制，这个机制对于绘制局部图形
	是非常有效的。TFT可以先指定一个绘图窗口，然后所有的读写显存的操作均在这个窗口之内，因此它不需要CPU在
	内存中保存整个屏幕的像素数据。
*/

#include <includes.h>
#include <stdio.h>
#include <string.h>
#include "R61505W.h"

/* 定义LCD驱动器的访问地址
	TFT接口中的RS引脚连接FSMC_A0引脚，由于是16bit模式，RS对应A1地址线，因此
	LCD_RAM的地址是+2
*/
#define LCD_BASE        ((u32)(0x60000000 | 0x08000000))
#define LCD_REG			*(volatile u16 *)(LCD_BASE)
#define LCD_RAM			*(volatile u16 *)(LCD_BASE + 2)

/* 定义LCD显示区域的分辨率 */
#define LCD_HEIGHT		220		/* 高度，单位：像素 */
#define LCD_WIDTH		220		/* 宽度，单位：像素 */

/* LCD 寄存器定义, LR_前缀是LCD Register的简写 */
#define LR_CTRL1		0x007	/* 读写显存的寄存器地址 */
#define LR_GRAM			0x22	/* 读写显存的寄存器地址 */
#define LR_GRAM_X		0x200	/* 显存水平地址（物理X坐标）*/
#define LR_GRAM_Y		0x201	/* 显存垂直地址（物理Y坐标）*/

/*
定义驱动函数
*/
static volatile u8 s_RGBChgEn = 0;		/* RGB转换使能, 4001屏写显存后读会的RGB格式和写入的不同 */
static volatile u8 s_AddrAutoInc = 0;	/* 读回一个像素后，显存地址是否自动增1 */

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
*	函 数 名: LCD_R61505W_WriteReg
*	功能说明: 修改LCD控制器的寄存器的值。
*	形    参：
*			LCD_Reg ：寄存器地址;
*			LCD_RegValue : 寄存器值
*	返 回 值: 无
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
*	函 数 名: LCD_R61505W_ReadReg
*	功能说明: 读取LCD控制器的寄存器的值。
*	形    参：
*			LCD_Reg ：寄存器地址;
*			LCD_RegValue : 寄存器值
*	返 回 值: 无
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
*	函 数 名: LCD_R61505W_BGR2RGB
*	功能说明: RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
*	形    参：RGB颜色代码
*	返 回 值: 转化后的颜色代码
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
*	函 数 名: LCD_R61505W_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_R61505W_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/* 使能 FSMC, GPIOD, GPIOE, GPIOF, GPIOG 和 AFIO 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                     RCC_APB2Periph_AFIO, ENABLE);

	/* 设置 PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) 为复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 设置 PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) 为复用推挽输出 */
	/* PE3,PE4 用于A19, A20, STM32F103ZE-EK(REV 1.0)必须使能 */
	/* PE5,PE6 用于A19, A20, STM32F103ZE-EK(REV 2.0)必须使能 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 设置 PF.00(A0 (RS))  为复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* 设置 PG.12(NE4 (LCD/CS)) 为复用推挽输出 - CE3(LCD /CS) */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	//GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* 设置 PG.10(NE3 (LCD/CS)) 为复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	函 数 名: LCD_R61505W_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参：无
*	返 回 值: 无
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
//	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* 注意旧库无这个成员 */
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
*	函 数 名: LCD_R61505W_GetID
*	功能说明: 读取LCD的器件ID
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: LCD_R61505W_Init
*	功能说明: 初始化LCD
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_R61505W_Init(void)
{
	u16 id;

	/* 配置LCD控制口线GPIO */
	LCD_R61505W_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_R61505W_FSMCConfig();

	/* FSMC重置后必须加延迟才能访问总线设备  */
	LCD_R61505W_Delayms(5);

	id = LCD_R61505W_GetID();  	/* 读取LCD驱动芯片ID */

	printf("芯片ID：0X%04X\r\n", id);

	printf("初始化LCD \r\n");
	LCD_R61505W_InitHard();
}  

/*
*********************************************************************************************************
*	函 数 名: LCD_R61505W_SetCursor
*	功能说明: 设置光标位置
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_R61505W_SetCursor(u16 _usX, u16 _usY)
{
	/*
		px，py 是物理坐标， x，y是虚拟坐标
		转换公式:
		py = 399 - x;
		px = y;
	*/
    _usX += 10;
	LCD_R61505W_WriteReg(0x0020, _usX);        //列首址
	LCD_R61505W_WriteReg(0x0021, _usY);        //行首址
} 

/*
*********************************************************************************************************
*	函 数 名: LCD_GetPixel
*	功能说明: 获取(x,y)坐标的像素点 
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
u16 LCD_R61505W_GetPoint(u16 _usX,u16 _usY)
{
	u16 usRGB;

	LCD_R61505W_SetCursor(_usX, _usY);	/* 设置光标位置 */

	/* 准备写显存 */
	LCD_REG = LR_GRAM;

	usRGB = LCD_RAM;

	/* 读 16-bit GRAM Reg */
	if (s_RGBChgEn == 1)
	{
		usRGB = LCD_R61505W_BGR2RGB(usRGB);
	}

	return usRGB;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_PutPixel
*	功能说明: 把像素点写入(x,y)坐标点
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_R61505W_SetPoint(u16 _usX, u16 _usY, u16 _usColor)
{
	LCD_R61505W_SetCursor(_usX, _usY);	/* 设置光标位置 */

	/* 写显存 */
	LCD_REG = LR_GRAM;

	/* Write 16-bit GRAM Reg */
	LCD_RAM = _usColor;
}       


