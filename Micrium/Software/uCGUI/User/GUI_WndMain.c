/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_WndMain.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <includes.h>
#include "GUI.h"
#if GUI_WINSUPPORT
#include "PROGBAR.h"
#include "LISTBOX.h"
#include "FRAMEWIN.h"
#include "BUTTON.h"
#endif
#include "GUI_WndMain.h"
#include "GUI_WndRadar.h"

/* include resources */
#include "res_bmp_main_backgroud2.h"
#include "res_bmp_radar.h"
#include "res_bmp_comp.h"
#include "res_bmp_config.h"
#include "res_bmp_help.h"

/*******************************************************************
*
*       macro variables
*
********************************************************************
*/
#define GUI_WND_MAIN_MAX_CHILD_CONTROLS         (4)


/*******************************************************************
*
*       static variables
*
********************************************************************
*/
static WM_CALLBACK* g_cbBkWindowOld;
static WM_HWIN      g_hWndMenu;
static WM_HWIN      g_hWndChildMenu[GUI_WND_MAIN_MAX_CHILD_CONTROLS];
static WM_HWIN      g_hWndCurrent;

static GUI_COLOR    g_WndMainColor              = GUI_DARKBLUE;
static GUI_COLOR    g_WndChildNormalColor       = GUI_GREEN;
static GUI_COLOR    g_WndChildSelColor          = GUI_YELLOW;
static GUI_COLOR    g_WndChildbkColor[GUI_WND_MAIN_MAX_CHILD_CONTROLS] = {GUI_YELLOW, GUI_GREEN, GUI_GREEN, GUI_GREEN};

static u8           g_ucCurrentSelChild          = 0;

/*******************************************************************
*
*       static functions
*
********************************************************************
*/
static void GUI_WndMenuCreate(WM_HWIN *pCurrent);

/*******************************************************************
*
*       static code, callbacks for windows
*
********************************************************************
*/

/*******************************************************************
*
*       _cbDeskTop
*/
static void _cbDeskTop(WM_MESSAGE* pMsg) 
{
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetBkColor(GUI_BLACK);
            GUI_Clear();
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ENTER:
                    /* call menu */
                    GUI_WndMenuCreate(&g_hWndCurrent);
                    break;
                default:
                    break;
            }
          break;
        default:
            WM_DefaultProc(pMsg);
    }
}

/*******************************************************************
*
*       _cbWndMenu
*/
static void _cbWndMenu(WM_MESSAGE* pMsg) {
    int i;
    WM_MESSAGE Message;

    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* draw bmp */
            GUI_DrawBitmap(&bmmain_backgroud2, 0, 0);

            /* draw circle */
            GUI_SetColor(GUI_WHITE);
            
            GUI_DrawCircle(90, 80, 5);
            GUI_DrawCircle(110, 80, 4);
            GUI_DrawCircle(130, 80, 4);

            /* fill circle */
            GUI_FillCircle(90, 80, 5);
            
            break;
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:                    
                    GUI_EndDialog(g_hWndMenu, 1);
                    if (0 != WM_SetFocus(WM_HBKWIN))
                    {
                        printf("[GUI] Set focus window to desktop failed.\r\n");
                    }
                    break;
                case GUI_KEY_ENTER:                    
                    break;
                case GUI_KEY_DOWN:
                    if (0 == g_ucCurrentSelChild)
                    {
                        g_ucCurrentSelChild = GUI_WND_MAIN_MAX_CHILD_CONTROLS - 1;                        
                    }
                    else
                    {
                        g_ucCurrentSelChild--;
                    }

                    /* send message to next control */
                    Message.MsgId  = GUI_USER_MSG_GETFOCUS;                    
                    WM_SendMessage(g_hWndChildMenu[g_ucCurrentSelChild], &Message);

                    /* unsel other controls */
                    for (i=0; i<GUI_WND_MAIN_MAX_CHILD_CONTROLS; i++)
                    {
                        if (i != g_ucCurrentSelChild)
                        {
                            /* send message to next control */
                            Message.MsgId  = GUI_USER_MSG_LOSFOCUSED;                            
                            WM_SendMessage(g_hWndChildMenu[i], &Message);
                        }
                    }
                   
                    break;
                case GUI_KEY_UP:
                    if (GUI_WND_MAIN_MAX_CHILD_CONTROLS <= ++g_ucCurrentSelChild)
                    {
                        g_ucCurrentSelChild = 0;                        
                    }

                    /* send message to next control */
                    Message.MsgId  = GUI_USER_MSG_GETFOCUS;                    
                    WM_SendMessage(g_hWndChildMenu[g_ucCurrentSelChild], &Message);

                    /* unsel other controls */
                    for (i=0; i<GUI_WND_MAIN_MAX_CHILD_CONTROLS; i++)
                    {
                        if (i != g_ucCurrentSelChild)
                        {
                            /* send message to next control */
                            Message.MsgId  = GUI_USER_MSG_LOSFOCUSED;                            
                            WM_SendMessage(g_hWndChildMenu[i], &Message);
                        }
                    }
                    
                    break;
                default:
                    break;
            }
          break;            
        default:
            WM_DefaultProc(pMsg);
    }
}

/*******************************************************************
*
*       _cbWndChildMenu1
*/
static void _cbWndChildMenu1(WM_MESSAGE* pMsg) {
    int              x, y;
    
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* draw bmp */
            GUI_DrawBitmap(&bmradar, 0, 0);

            /*  draw active/unactive rect */
            GUI_SetColor(g_WndChildbkColor[0]);

            x = WM_GetWindowSizeX(pMsg->hWin);
            y = WM_GetWindowSizeY(pMsg->hWin);

            GUI_SetPenSize(5);
            GUI_DrawRect(0, 0, x - 1, y - 1);

            break;
        case GUI_USER_MSG_GETFOCUS:
            {
                g_WndChildbkColor[0] = g_WndChildSelColor;
                WM_InvalidateWindow(g_hWndChildMenu[0]);
            }
            break;
        case GUI_USER_MSG_LOSFOCUSED:
            {
                g_WndChildbkColor[0] = g_WndChildNormalColor;
                WM_InvalidateWindow(g_hWndChildMenu[0]);
            }
            break;            
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(g_hWndChildMenu[0], 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(g_hWndChildMenu[0], 0);
                    break;
                case GUI_KEY_DOWN:
                    break;
                case GUI_KEY_UP:
                    break;
                default:
                    break;
            }
          break;            
        default:
            WM_DefaultProc(pMsg);
    }
}

static void _cbWndChildMenu2(WM_MESSAGE* pMsg) {
    int              x, y;
    
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* draw bmp */
            GUI_DrawBitmap(&bmcomp, 0, 0);

            /*  draw active/unactive rect */
            GUI_SetColor(g_WndChildbkColor[1]);

            x = WM_GetWindowSizeX(pMsg->hWin);
            y = WM_GetWindowSizeY(pMsg->hWin);

            GUI_SetPenSize(5);
            GUI_DrawRect(0, 0, x - 1, y - 1);
            break;
        case GUI_USER_MSG_GETFOCUS:
            {
                g_WndChildbkColor[1] = g_WndChildSelColor;
                WM_InvalidateWindow(g_hWndChildMenu[1]);
            }
            break;
        case GUI_USER_MSG_LOSFOCUSED:
            {
                g_WndChildbkColor[1] = g_WndChildNormalColor;
                WM_InvalidateWindow(g_hWndChildMenu[1]);
            }
            break;             
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(g_hWndChildMenu[1], 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(g_hWndChildMenu[1], 0);
                    break;
                case GUI_KEY_DOWN:
                    break;
                case GUI_KEY_UP:
                    break;
                default:
                    break;
            }
          break;             
        default:
            WM_DefaultProc(pMsg);
    }
}

static void _cbWndChildMenu3(WM_MESSAGE* pMsg) {
    int              x, y;
    
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* draw bmp */
            GUI_DrawBitmap(&bmconfig, 0, 0);

            /*  draw active/unactive rect */
            GUI_SetColor(g_WndChildbkColor[2]);

            x = WM_GetWindowSizeX(pMsg->hWin);
            y = WM_GetWindowSizeY(pMsg->hWin);

            GUI_SetPenSize(5);
            GUI_DrawRect(0, 0, x - 1, y - 1);
            break;
        case GUI_USER_MSG_GETFOCUS:
            {
                g_WndChildbkColor[2] = g_WndChildSelColor;
                WM_InvalidateWindow(g_hWndChildMenu[2]);
            }
            break;
        case GUI_USER_MSG_LOSFOCUSED:
            {
                g_WndChildbkColor[2] = g_WndChildNormalColor;
                WM_InvalidateWindow(g_hWndChildMenu[2]);
            }
            break;             
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(g_hWndChildMenu[2], 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(g_hWndChildMenu[2], 0);
                    break;
                case GUI_KEY_DOWN:
                    break;
                case GUI_KEY_UP:
                    break;
                default:
                    break;
            }
          break;             
        default:
            WM_DefaultProc(pMsg);
    }
}

static void _cbWndChildMenu4(WM_MESSAGE* pMsg) {
    int              x, y;
    
    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* draw bmp */
            GUI_DrawBitmap(&bmhelp, 0, 0);

            /*  draw active/unactive rect */
            GUI_SetColor(g_WndChildbkColor[3]);

            x = WM_GetWindowSizeX(pMsg->hWin);
            y = WM_GetWindowSizeY(pMsg->hWin);

            GUI_SetPenSize(5);
            GUI_DrawRect(0, 0, x - 1, y - 1);
            break;
        case GUI_USER_MSG_GETFOCUS:
            {
                g_WndChildbkColor[3] = g_WndChildSelColor;
                WM_InvalidateWindow(g_hWndChildMenu[3]);
            }
            break;
        case GUI_USER_MSG_LOSFOCUSED:
            {
                g_WndChildbkColor[3] = g_WndChildNormalColor;
                WM_InvalidateWindow(g_hWndChildMenu[3]);
            }
            break;             
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                case GUI_KEY_ESCAPE:
                    GUI_EndDialog(g_hWndChildMenu[3], 1);
                    break;
                case GUI_KEY_ENTER:
                    GUI_EndDialog(g_hWndChildMenu[3], 0);
                    break;
                case GUI_KEY_DOWN:
                    break;
                case GUI_KEY_UP:
                    break;
                default:
                    break;
            }
          break;             
        default:
            WM_DefaultProc(pMsg);
    }
}

WM_HWIN GUI_GetCurrentWnd(void)
{
    return g_hWndCurrent;
}

void GUI_WndMainCreate(void)
{
    /* Set callback for background window */
    g_cbBkWindowOld = WM_SetCallback(WM_HBKWIN, _cbDeskTop);

    /* create default window, radar */
    GUI_WndRadarCreate(&g_hWndCurrent);
}

void GUI_WndMenuCreate(WM_HWIN *pCurrent)
{
    /* Create main wnd */
    g_hWndMenu = WM_CreateWindow( 0, 60, 220, 100, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndMenu, 0);

    /* create controls */
    g_hWndChildMenu[0] = WM_CreateWindowAsChild(15, 15, 40, 40, g_hWndMenu, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndChildMenu1, 0);
    g_hWndChildMenu[1] = WM_CreateWindowAsChild(65, 15, 40, 40, g_hWndMenu, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndChildMenu2, 0);
    g_hWndChildMenu[2] = WM_CreateWindowAsChild(115, 15, 40, 40, g_hWndMenu, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndChildMenu3, 0);
    g_hWndChildMenu[3] = WM_CreateWindowAsChild(165, 15, 40, 40, g_hWndMenu, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndChildMenu4, 0);

    if (0 != WM_SetFocus(g_hWndMenu))
    {
        printf("[GUI] Set focus window failed.\r\n");
        return;
    }

    *pCurrent = g_hWndMenu;
}

