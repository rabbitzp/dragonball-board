/*
*********************************************************************************************************
*
*                                      GUI for gps radar
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_WndRadar.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __GUI_WND_RADAR_H__
#define __GUI_WND_RADAR_H__

#include "GUI_WndDef.h"  /* valid LCD configuration */


#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*      Macro defines
*
*********************************************************************
*/

/*********************************************************************
*
*      Function defines
*
*********************************************************************
*/
void GUI_WndRadarCreate(WM_HWIN *pCurrent);
void GUI_WndRadarDestroy(void);

#if defined(__cplusplus)
  }
#endif

#endif
