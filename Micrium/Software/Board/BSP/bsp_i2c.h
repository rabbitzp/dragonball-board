/*
*********************************************************************************************************
*
*                                      bsp i2c functions
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : bsp_i2c.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#define I2C_WR	0		/* Ð´¿ØÖÆbit */
#define I2C_RD	1		/* ¶Á¿ØÖÆbit */

void        i2c_InitBus(void);
bool        i2c_Start(void);
void        i2c_Stop(void);
void        i2c_SendByte(u8 _ucByte);
u8          i2c_ReadByte(void);
u8          i2c_WaitAck(void);
void        i2c_Ack(void);
void        i2c_NAck(void);

u8          i2c_CheckDevice(u8 _Address);

#if defined(__cplusplus)
  }
#endif


#endif /* __BSP_I2C_H__ */

