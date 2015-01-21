/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_key.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include "GUI_WndDef.h"  /* valid LCD configuration */


/*----------------macros declare here---------------------*/
#define PIN_USER_KEY_HOME       GPIO_Pin_3            /* GPIOD*/
#define PIN_USER_KEY_OK         GPIO_Pin_6            /* GPIOC*/
#define PIN_USER_KEY_RETURN     GPIO_Pin_7            /* GPIOG*/
#define PIN_USER_KEY_UP         GPIO_Pin_7            /* GPIOC*/
#define PIN_USER_KEY_DOWN       GPIO_Pin_8            /* GPIOA*/

/*----------------local global vars declare here---------------------*/
static OS_STK      g_UserTaskKeySTK[DEFAULT_USER_KEY_STK_SIZE];

/*----------------local global funcs declare here---------------------*/
static void UKey_TaskProc(void *p_arg);
static void UKey_Init(void);
static void UKey_EventLoop(void);


u8 UKey_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    printf("starting user task key...\r\n");

    ucResult = OSTaskCreate(UKey_TaskProc, (void *)0, (OS_STK *)&g_UserTaskKeySTK[DEFAULT_USER_KEY_STK_SIZE - 1], DEFAULT_USER_TASK_KEY_PRIO);

    if (OS_ERR_NONE == ucResult)
    {
        return UCORE_ERR_SUCCESS;
    }

    printf("User task Key Failed value:%d.\r\n", ucResult);

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void UKey_TaskProc(void *p_arg)
{
    printf("User key Task Start.\r\n");

    /* call init */
    UKey_Init();

    /* enter event loop */
    UKey_EventLoop();

    printf("User key Task Exit.\r\n");
}

void UKey_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;                    //����GPIO_InitStructure
    
    /* HOME KEY PD3 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);    //��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = PIN_USER_KEY_HOME;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;        //����������2MHz

    GPIO_Init(GPIOD, &GPIO_InitStructure);                  //��ʼ��GPIO

    /* OK/UP KEY PC6 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    //��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = PIN_USER_KEY_OK | PIN_USER_KEY_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;        //����������2MHz

    GPIO_Init(GPIOC, &GPIO_InitStructure);                  //��ʼ��GPIO    

    /* DOWN KEY PA8 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = PIN_USER_KEY_DOWN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;        //����������2MHz

    GPIO_Init(GPIOA, &GPIO_InitStructure);                  //��ʼ��GPIO    

    /* RETURN KEY PG7 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);    //��GPIOʱ��

    GPIO_InitStructure.GPIO_Pin = PIN_USER_KEY_RETURN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;        //����������2MHz

    GPIO_Init(GPIOG, &GPIO_InitStructure);                  //��ʼ��GPIO      
}

#define DETECT_TIME 50 // ms
u32 abs_time(u32 time_p, u32 time_n)
{
	return time_n>time_p?time_n-time_p:0xFFFFFFFF-time_p+time_n;
}

void UKey_EventLoop(void)
{
    while (1)
    {
        /* HOME, PD3 */
#if 1
{
	static vu32 time_pressed;
	static vu8 pressed = 0;
	static vu8 processed = 0;
	
	if(0 == GPIO_ReadInputDataBit(GPIOD, PIN_USER_KEY_HOME))//��������
    {
		if (!processed)
		{
        	if (!pressed)
        	{
    			time_pressed = OSTime;
    			pressed = 1;
        	}
			else
			{
				if (abs_time(time_pressed, OSTime) > DETECT_TIME)
				{
					processed = 1;

				    GUI_SendKeyMsg(GUI_KEY_HOME, 1);                    
				}
					
			}

		}
	}
	else
	{
		pressed = 0;
		processed = 0;
	}
}
{
	static vu32 time_pressed;
	static vu8 pressed = 0;
	static vu8 processed = 0;
	
	if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_OK))//��������
    {
		if (!processed)
		{
        	if (!pressed)
        	{
    			time_pressed = OSTime;
    			pressed = 1;
        	}
			else
			{
				if (abs_time(time_pressed, OSTime)>DETECT_TIME)
				{
					processed = 1;

				    GUI_SendKeyMsg(GUI_KEY_ENTER, 1);                    
				}
					
			}

		}
	}
	else
	{
		pressed = 0;
		processed = 0;
	}
}
{
	static vu32 time_pressed;
	static vu8 pressed = 0;
	static vu8 processed = 0;
	
	if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_UP))//��������
    {
		if (!processed)
		{
        	if (!pressed)
        	{
    			time_pressed = OSTime;
    			pressed = 1;
        	}
			else
			{
				if (abs_time(time_pressed, OSTime)>DETECT_TIME)
				{				
					processed = 1;

                    GUI_SendKeyMsg(GUI_KEY_UP, 1);                    
				}
					
			}

		}
	}
	else
	{
		pressed = 0;
		processed = 0;
	}
}
{
	static vu32 time_pressed;
	static vu8 pressed = 0;
	static vu8 processed = 0;
	
	if(0 == GPIO_ReadInputDataBit(GPIOA, PIN_USER_KEY_DOWN))//��������
    {
		if (!processed)
		{
        	if (!pressed)
        	{
    			time_pressed = OSTime;
    			pressed = 1;
        	}
			else
			{
				if (abs_time(time_pressed, OSTime)>DETECT_TIME)
				{				
					processed = 1;

                    GUI_SendKeyMsg(GUI_KEY_DOWN, 1);                    
				}
					
			}

		}
	}
	else
	{
		pressed = 0;
		processed = 0;
	}
}
{
	static vu32 time_pressed;
	static vu8 pressed = 0;
	static vu8 processed = 0;
	
	if(0 == GPIO_ReadInputDataBit(GPIOG, PIN_USER_KEY_RETURN))//��������
    {
		if (!processed)
		{
        	if (!pressed)
        	{
    			time_pressed = OSTime;
    			pressed = 1;
        	}
			else
			{
				if (abs_time(time_pressed, OSTime)>DETECT_TIME)
				{
					processed = 1;

                    GUI_SendKeyMsg(GUI_KEY_ESCAPE, 1);                    
				}
					
			}

		}
	}
	else
	{
		pressed = 0;
		processed = 0;
	}
}



#else
        if(0 == GPIO_ReadInputDataBit(GPIOD, PIN_USER_KEY_HOME))//��������
        {
            /* �ӳ�ȥ�� */
            OSTimeDlyHMSM(0, 0, 0, 50);
            if(0 == GPIO_ReadInputDataBit(GPIOD, PIN_USER_KEY_HOME))//��������
            {
                UCore_PostMessage1(UCORE_MESSAGE_TYPE_KEY_HOME, 0, NULL); 
            }
        }

        /* OK, PC6 */
        if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_OK))//��������
        {
            /* �ӳ�ȥ�� */
            OSTimeDlyHMSM(0, 0, 0, 50);
            if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_OK))//��������
            {
                UCore_PostMessage1(UCORE_MESSAGE_TYPE_KEY_OK, 0, NULL); 
            }
        }

        /* UP, PC7 */
        if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_UP))//��������
        {
            /* �ӳ�ȥ�� */
            OSTimeDlyHMSM(0, 0, 0, 50);
            if(0 == GPIO_ReadInputDataBit(GPIOC, PIN_USER_KEY_UP))//��������
            {
                UCore_PostMessage1(UCORE_MESSAGE_TYPE_KEY_UP, 0, NULL); 
            }
        } 

        /* DOWN, PA8 */
        if(0 == GPIO_ReadInputDataBit(GPIOA, PIN_USER_KEY_DOWN))//��������
        {
            /* �ӳ�ȥ�� */
            OSTimeDlyHMSM(0, 0, 0, 50);
            if(0 == GPIO_ReadInputDataBit(GPIOA, PIN_USER_KEY_DOWN))//��������
            {
                UCore_PostMessage1(UCORE_MESSAGE_TYPE_KEY_DOWN, 0, NULL); 
            }
        } 

        /* RETURN, PG7 */
        if(0 == GPIO_ReadInputDataBit(GPIOG, PIN_USER_KEY_RETURN))//��������
        {
            /* �ӳ�ȥ�� */
            OSTimeDlyHMSM(0, 0, 0, 50);
            if(0 == GPIO_ReadInputDataBit(GPIOG, PIN_USER_KEY_RETURN))//��������
            {
                UCore_PostMessage1(UCORE_MESSAGE_TYPE_KEY_RETURN, 0, NULL); 
            }
        }        
#endif                
        OSTimeDlyHMSM(0, 0, 0, 50);	
    }
}

