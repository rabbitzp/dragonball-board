/*
*********************************************************************************************************
*
*                                      GUI for gps radar
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : GUI_WndRadar.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <includes.h>
#include "GUI.h"
#if GUI_WINSUPPORT
#include "PROGBAR.h"
#include "LISTBOX.h"
#include "FRAMEWIN.h"
#include "BUTTON.h"
#endif
#include "nmea_parser.h"
#include "GUI_WndRadar.h"
#include <user_ep_manage.h>

#define SCREEN_X_PIXEL 220
#define SCREEN_Y_PIXEL 220

#define SCREEN_ORIGIN_X_POS (SCREEN_X_PIXEL / 2)
#define SCREEN_ORIGIN_Y_POS (SCREEN_Y_PIXEL / 2)

#define RADAR_MAX_SCAN_RANGE        500 /* for meter */
#define RADAR_SCAN_ACCURACY_RANGE   50  /* for meter */
#define RADAR_TOTAL_CIRCLES         (RADAR_MAX_SCAN_RANGE / RADAR_SCAN_ACCURACY_RANGE)  /* for now, 10 circles */
#define RADAR_CIRCLE_RADIUS         ((SCREEN_X_PIXEL / 2) / RADAR_TOTAL_CIRCLES)
#define RADAR_METER_PIXS            (0.22)  /* 110 / 500 */

#define RADAR_MAX_SUPPORT_EP_NUM    32

extern EP_INFO_S g_stMyEpInfo;

static WM_HWIN      g_hWndRadar;

static nmeaINFO     g_stArryGPSInfo[RADAR_MAX_SUPPORT_EP_NUM]   = {0};
static OS_TMR       *g_drawtimer = NULL;

/*******************************************************************/
/*-+-+-+-+-+-+-+-+-+-+-+ local functions -+-+-+-+-+-+-+-+-+-+-+-+-+*/
/*******************************************************************/
double rad(double d)
{
	const double PI = 3.1415926535898;
	return d * PI / 180.0;
}

/*******************************************************************
*
*       RaderDrawTimerCallback
*/
void RaderDrawTimerCallback (OS_TMR *ptmr, void *p_arg)
{
    /* redraw */
    WM_InvalidateWindow(g_hWndRadar);    
}

/*******************************************************************
*
*       _cbWndRadar
*/
static void _cbWndRadar(WM_MESSAGE* pMsg) {
    int         i;

    switch (pMsg->MsgId) 
    {
        case WM_PAINT:
        {
            float fDistance, fdisx, fdisy, fangle;
            u16   uDisPixx, uDisPixY;
            u8    ucMyEpId = 0;
            
            GUI_SetDrawMode(GUI_DRAWMODE_NORMAL);
            GUI_SetTextMode(GUI_TM_TRANS);    
            
            /* clear control */
            GUI_Clear();

            /* set backgroud color */
            GUI_SetBkColor(GUI_BLACK);

            /* set forgroud color */
            GUI_SetColor(GUI_DARKGREEN);

            /* set pen size */
            GUI_SetPenSize(2);

            /* draw circles */
            for (i=1; i<=RADAR_TOTAL_CIRCLES; i++)
            {
                GUI_DrawCircle(SCREEN_ORIGIN_X_POS, SCREEN_ORIGIN_Y_POS, RADAR_CIRCLE_RADIUS * i);
            } 

            /* draw lines */
            GUI_DrawLine(0, SCREEN_ORIGIN_Y_POS, SCREEN_X_PIXEL, SCREEN_ORIGIN_Y_POS);
            GUI_DrawLine(SCREEN_ORIGIN_X_POS, 0, SCREEN_ORIGIN_X_POS, SCREEN_Y_PIXEL);  
            GUI_DrawLine(0, 0, SCREEN_X_PIXEL, SCREEN_Y_PIXEL);
            GUI_DrawLine(SCREEN_X_PIXEL, 0, 0, SCREEN_Y_PIXEL);    

            ucMyEpId = g_stMyEpInfo.ucEpId;
            
            /* my gps must be valid */
            if ('A' != g_stArryGPSInfo[ucMyEpId].status)
            {
                return;
            }

            /* 我的位置以红点表示在雷点原点，其它终端的位置以我为参考 */
            GUI_SetColor(GUI_RED);
            GUI_FillCircle(SCREEN_ORIGIN_X_POS, SCREEN_ORIGIN_Y_POS, 3);
            
            for (i = 0; i < RADAR_MAX_SUPPORT_EP_NUM; i++)
            {
                /* 跳过自身，画出其它终端与自身的参考位置 */
                if (('A' == g_stArryGPSInfo[i].status) && (i != ucMyEpId))
                {
                    /* cal distance */
                    fDistance = nmea_caldistance(g_stArryGPSInfo[ucMyEpId].lat, g_stArryGPSInfo[ucMyEpId].lon, g_stArryGPSInfo[i].lat, g_stArryGPSInfo[i].lon);
                    fangle = g_stArryGPSInfo[i].course;

                    if (fangle >= 0 && fangle <= 90)
                    {
                        fdisx = fDistance * sin(rad(fangle));
                        fdisy = fDistance * cos(rad(fangle));

                        uDisPixx = SCREEN_ORIGIN_X_POS + fdisx * RADAR_METER_PIXS;
                        uDisPixY = SCREEN_ORIGIN_Y_POS - fdisy * RADAR_METER_PIXS;
                    }
                    else if (fangle > 90 && fangle <= 180)
                    {
                        fangle -= 90;
                        
                        fdisx = fDistance * cos(rad(fangle));
                        fdisy = fDistance * sin(rad(fangle));  

                        uDisPixx = SCREEN_ORIGIN_X_POS + fdisx * RADAR_METER_PIXS;
                        uDisPixY = SCREEN_ORIGIN_Y_POS + fdisy * RADAR_METER_PIXS;                        
                    }
                    else if (fangle > 180 && fangle <= 270)
                    {
                        fangle -= 180;
                        
                        fdisx = fDistance * sin(rad(fangle));
                        fdisy = fDistance * cos(rad(fangle));     

                        uDisPixx = SCREEN_ORIGIN_X_POS - fdisx * RADAR_METER_PIXS;
                        uDisPixY = SCREEN_ORIGIN_Y_POS + fdisy * RADAR_METER_PIXS;                        
                    }
                    else
                    {
                        fangle -= 270;
                        
                        fdisx = fDistance * cos(rad(fangle));
                        fdisy = fDistance * sin(rad(fangle));

                        uDisPixx = SCREEN_ORIGIN_X_POS - fdisx * RADAR_METER_PIXS;
                        uDisPixY = SCREEN_ORIGIN_Y_POS - fdisy * RADAR_METER_PIXS;                        
                    }

                    /* dump info */
                    //printf("ep[%d] to orgin's distance:%.5f, uDisPixx:%d uDisPixY:%d\r\n", g_stArryGPSInfo[i].id, fDistance, uDisPixx, uDisPixY);

                    /* 如果是协调器，则用黄点表示，默认其它EP是白点 */
                    if (i == 0)
                    {
                        GUI_SetColor(GUI_YELLOW);
                    }
                    else
                    {
                        GUI_SetColor(GUI_WHITE);
                    }

                    /* draw this point */
                    GUI_FillCircle(uDisPixx, uDisPixY, 3);
                }
            }
        }   
            break;
        case GUI_USER_MSG_GPS_UPDATE:
            {
                nmeaINFO *pInfo = NULL;

                pInfo = (nmeaINFO *) pMsg->Data.p;
                /* copy info */
                if (NULL != pInfo)
                {
                    if (pInfo->id >= 32)
                        return;                                       

                    memcpy(&g_stArryGPSInfo[pInfo->id], pInfo, sizeof(nmeaINFO));
                }
            }
        case WM_KEY:
            switch (((WM_KEY_INFO*)(pMsg->Data.p))->Key) 
            {
                default:
                    break;
            }
          break;            
        default:
            WM_DefaultProc(pMsg);
    }
}


void GUI_WndRadarCreate(WM_HWIN *pCurrent)
{
    INT8U err = 0;
    
    /* Create radar wnd */
    g_hWndRadar = WM_CreateWindow( 0, 0, 220, 220, WM_CF_SHOW | WM_CF_MEMDEV, _cbWndRadar, 0);

    if (0 != WM_SetFocus(g_hWndRadar))
    {
        printf("[GUI] Set focus window failed.\r\n");
        return;
    }

    *pCurrent = g_hWndRadar;

    /* start timer */
    g_drawtimer = OSTmrCreate(10, 10, OS_TMR_OPT_PERIODIC, (OS_TMR_CALLBACK) RaderDrawTimerCallback, 0, "tmr1", &err);
    if (NULL == g_drawtimer)
    {
        printf("[GUI] create radar draw timer failed.\r\n");
        return;    
    }

    if (OS_TRUE != OSTmrStart(g_drawtimer, &err))
    {
        printf("[GUI] start radar draw timer failed.\r\n");
        return;     
    }
}

void GUI_WndRadarDestroy(void)
{
    INT8U err = 0;

    if (OS_TRUE != OSTmrStop(g_drawtimer, OS_TMR_OPT_PERIODIC, 0, &err))
    {
        printf("[GUI] stop radar draw timer failed.\r\n");
    }

    if (OS_TRUE != OSTmrDel(g_drawtimer, &err))
    {
        printf("[GUI] del radar draw timer failed.\r\n");
    }    
}
