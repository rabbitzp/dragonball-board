/*
*********************************************************************************************************
*	                                  
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : LCD_tft_lcd.h
*	版    本 : V2.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 armfly  ST固件库版本 V3.5.0版本。
*		v2.0    2011-10-16 armfly  ST固件库版本 V3.5.0版本。规范函数接口，优化内部结构
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _BSP_TFT_LCD_H
#define _BSP_TFT_LCD_H

#define LCD_BKL_ON()	GPIO_SetBits(GPIOB, GPIO_Pin_9)    	 // LCD背光点亮--1
#define LCD_BKL_OFF()	GPIO_ResetBits(GPIOB, GPIO_Pin_9)    // LCD背光熄灭--0

void    LCD_R61505W_Init(void);                             //LCD硬件初始化
u16     LCD_R61505W_GetPoint(u16 x,u16 y);                  //获取(x,y)坐标的像素点
void    LCD_R61505W_SetPoint(u16 x,u16 y,u16 point);        //把像素点写入(x,y)坐标点

#endif


