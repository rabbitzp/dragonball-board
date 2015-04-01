/*
*********************************************************************************************************
*
*                                      EEPROM R/W functions
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : driver_24Cxx.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __DRIVER_24CXX_H_
#define __DRIVER_24CXX_H_

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767 

#define EE_DEV_ADDR			0xA0		    /* 24C128的设备地址 */
#define EE_PAGE_SIZE		64			    /* 24C128的页面大小 */

#define EE_SIZE				16384   	    /* 24C128总容量 */

u8 Driver_AT24CXXProbe(void);

u8 AT24CXX_ReadBytes(u8 *_pReadBuf, u16 _usAddress, u16 _usSize);
u8 AT24CXX_WriteBytes(u8 *_pWriteBuf, u16 _usAddress, u16 _usSize);

#endif /* __DRIVER_24CXX_H_ */

