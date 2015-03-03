/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_UserApp.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <stddef.h>
#include <string.h>
#include "GUI.h"
#include "GUI_UserApp.h"
#include "GUI_WndMain.h"


/*********************************************************************
*
*       GUIDEMO_Intro
*
**********************************************************************
*/
void GUI_UserDispIntro(void) {
  int xCenter = LCD_GET_XSIZE() / 2;
  int y;
  char acText[50] = "Version of DragonBall:1.00 ";
  GUI_SetBkColor(GUI_BLUE);
  GUI_SetColor(GUI_LIGHTRED);
  GUI_Clear();
  GUI_SetFont(&GUI_Font24B_1);
  GUI_DispStringHCenterAt("DragonBall", xCenter, y= 15);
  GUI_SetFont(&GUI_Font10S_ASCII);
  GUI_DispStringHCenterAt("Compiled " __DATE__ " "__TIME__, xCenter, y += 25);
  GUI_SetFont(&GUI_Font13HB_1);
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringHCenterAt(acText, xCenter, y += 21);
  GUI_SetFont(&GUI_FontComic18B_1);
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringHCenterAt("www.JBS.com", LCD_GET_XSIZE() / 2, LCD_GET_YSIZE() - 40);
  GUI_SetFont(&GUI_Font10S_ASCII);
  GUI_DispStringAt("GUI_OS: ", 0,210); GUI_DispDecMin(GUI_OS);
  GUI_DispStringAt("GUI_ALLOC_SIZE: ",0, 220); GUI_DispDecMin(GUI_ALLOC_SIZE);
  GUI_DispStringAt("Compiler: "
  #ifdef _MSC_VER
    "Microsoft"
  #elif defined (NC308)
    "Mitsubishi NC308"
  #elif defined (NC30)
    "Mitsubishi NC30"
  #elif defined (__TID__)
    #if (((__TID__ >>8) &0x7f) == 48)            /* IAR MC80 */
      "IAR M32C"
    #elif (((__TID__ >>8) &0x7f) == 85)          /* IAR V850 */
      "IAR V850"
    #else                                        /* IAR MC16 */
      "IAR M32C"
    #endif
  #else
    "Unknown"
  #endif
    ,0, 230);
  GUI_Delay(5000);
}

void GUI_UserAppStart(void)
{
#if GUI_WINSUPPORT
    /* Automatically use memory devices on all windows */
    WM_SetCreateFlags(WM_CF_MEMDEV);
    WM_EnableMemdev(WM_HBKWIN);
    
    /* display version info */
    GUI_UserDispIntro();

    /* set default desktop back color */
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    
    /* create main window */
    GUI_WndMainCreate();
#endif    
}

