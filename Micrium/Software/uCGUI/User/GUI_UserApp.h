/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_UserApp.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __GUI_USER_APP_H__
#define __GUI_USER_APP_H__

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
void GUI_UserAppStart(void);

void MainTask(void);

#if defined(__cplusplus)
  }
#endif

#endif
