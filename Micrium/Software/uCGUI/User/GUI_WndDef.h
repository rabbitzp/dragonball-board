/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_WndDef.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __GUI_WND_DEF_H__
#define __GUI_WND_DEF_H__

#include "LCD_ConfDefaults.h"  /* valid LCD configuration */
#include "WM.h"
#include "GUI.h"


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*      Macro defines
*
*********************************************************************
*/

#define GUI_USER_LARGE ((LCD_XSIZE >= 240) && (LCD_YSIZE >= 200))
#define GUI_USER_TINY  ((LCD_XSIZE <  140) || (LCD_YSIZE <  100))

/* here define user message */
#define GUI_USER_MSG_GETFOCUS           WM_USER + 0
#define GUI_USER_MSG_LOSFOCUSED         WM_USER + 1
#define GUI_USER_MSG_GPS_UPDATE         WM_USER + 2

#if defined(__cplusplus)
  }
#endif

#endif

