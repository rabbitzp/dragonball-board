/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_lcd.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>


/*----------------local global vars declare here---------------------*/
static OS_STK       g_UserTaskLcdSTK[DEFAULT_USER_LCD_STK_SIZE];

OS_EVENT            *g_QMboxUIMsg = NULL;
/*----------------local global funcs declare here---------------------*/
static void ULCD_TaskProc(void *p_arg);
static void ULCD_Init(void);
static void ULCD_EventLoop(void);


u8 ULCD_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    printf("starting user task lcd...\r\n");

    ucResult = OSTaskCreate(ULCD_TaskProc, (void *)0, (OS_STK *)&g_UserTaskLcdSTK[DEFAULT_USER_LCD_STK_SIZE - 1], DEFAULT_USER_TASK_LCD_PRIO);

    if (OS_ERR_NONE == ucResult)
    {
        return UCORE_ERR_SUCCESS;
    }

    printf("User task lcd Failed value:%d.\r\n", ucResult);

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void ULCD_TaskProc(void *p_arg)
{
    printf("User Task lcd Start.\r\n");

    /* call init */
    ULCD_Init();

    /* enter event loop */
    ULCD_EventLoop();

    printf("User Task lcd Exit.\r\n");
}

void ULCD_Init(void)
{
    g_QMboxUIMsg = OSMboxCreate((void *)0);

    printf("User task lcd init finished.\r\n");
}

void ULCD_EventLoop(void)
{
    INT8U           err     = 0;    
    UCORE_MSG_S     *pMsg   = NULL;   	
    
    while (1)
    {
        pMsg = OSMboxPend(g_QMboxUIMsg, 0, &err); 
        if ((OS_NO_ERR == err) && (NULL != pMsg))
        {
            switch (pMsg->usMsgType)
            {
                case UCORE_MESSAGE_TYPE_KEY_HOME:
                    {
                        printf("Key press:HOME.\r\n");
                        //LCD_ClrScr(CL_BLACK);
                    	//LCD_TextOut(110, 120, "HOME", TRANSP);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_KEY_OK:
                    {
                        printf("Key press:OK.\r\n");
                        //LCD_ClrScr(CL_BLACK);
                        //LCD_TextOut(110, 120, "OK", TRANSP);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_KEY_UP:
                    {
                        printf("Key press:UP.\r\n");
                        //LCD_ClrScr(CL_BLACK);
                        //LCD_TextOut(110, 120, "UP", TRANSP);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_KEY_DOWN:
                    {
                        printf("Key press:DOWN.\r\n");
                        //LCD_ClrScr(CL_BLACK);
                        //LCD_TextOut(110, 120, "DOWN", TRANSP);
                    }
                    break;
                case UCORE_MESSAGE_TYPE_KEY_RETURN:
                    {
                        printf("Key press:RETURN.\r\n");
                        //LCD_ClrScr(CL_BLACK);
                        //LCD_TextOut(110, 120, "RETURN", TRANSP);
                    }
                    break;                                      
                default:
                    printf("Unknown message type:%d\r\n", pMsg->usMsgType);
                    break;
            }

            /* release message buff */
            UCORE_DEL_MESSAGE(pMsg);
        }
    }
}

