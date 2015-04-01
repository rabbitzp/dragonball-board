/*
*********************************************************************************************************
*
*                                      User core processor
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_task_hmc.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <math.h>
#include <user_core.h>
#include <user_task_hmc.h>
#include "bsp_i2c.h"


#define	HMC5883L_Addr   0x3C

/*----------------local global vars declare here---------------------*/
static OS_STK       g_UserTaskHmcSTK[DEFAULT_USER_HMC_STK_SIZE];

OS_EVENT            *g_QMboxHmcMsg = NULL;

u8                  test = 0; 
u16                 angle;
/*----------------local global funcs declare here---------------------*/
static void UHMC_TaskProc(void *p_arg);
static void UHMC_Init(void);
static void UHMC_EventLoop(void);

void conversion(u16 temp_data)  
{   
	UCORE_DEBUG_PRINT("现在的角度为:=%d°\n",temp_data);
    
    OSTimeDlyHMSM(0, 0, 0, 20);		
}

unsigned char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{   unsigned char REG_data;     	
	if(!i2c_Start())return FALSE;
    i2c_SendByte(SlaveAddress); //i2c_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(!i2c_WaitAck()){i2c_Stop();test=1; return FALSE;}
    i2c_SendByte((u8) REG_Address);   //设置低起始地址      
    i2c_WaitAck();
    i2c_Start();
    i2c_SendByte(SlaveAddress+1);
    i2c_WaitAck();

	REG_data= i2c_ReadByte();
    i2c_NAck();
    i2c_Stop();
    //return TRUE;
	return REG_data;

}

bool Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)		     //void
{
  	if(!i2c_Start())return FALSE;
    i2c_SendByte(SlaveAddress);   //发送设备地址+写信号//i2c_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址 
    if(!i2c_WaitAck()){i2c_Stop(); return FALSE;}
    i2c_SendByte(REG_Address );   //设置低起始地址      
    i2c_WaitAck();	
    i2c_SendByte(REG_data);
    i2c_WaitAck();   
    i2c_Stop(); 

    OSTimeDlyHMSM(0, 0, 0, 5);
    
    return TRUE;
}

void read_hmc5883l()
{
    unsigned char BUF[8];                         //接收数据缓存区
    int   x,y;

    Single_Write(HMC5883L_Addr,0x00,0x14);    
    Single_Write(HMC5883L_Addr,0x02,0x00);   
    
    OSTimeDlyHMSM(0, 0, 0, 10);

    BUF[1]=Single_Read(HMC5883L_Addr,0x03); 
    BUF[2]=Single_Read(HMC5883L_Addr,0x04); 

    BUF[3]=Single_Read(HMC5883L_Addr,0x07); 
    BUF[4]=Single_Read(HMC5883L_Addr,0x08); 

    x=(BUF[1] << 8) | BUF[2];  
    y=(BUF[3] << 8) | BUF[4];  

    if(x>0x7fff)x-=0xffff;	  
    if(y>0x7fff)y-=0xffff;	 
    
    angle= atan2(y,x) * (180 / 3.14159265) + 180;  

    /* conversion(angle); */
}

u8 UHMC_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    UCORE_DEBUG_PRINT("starting user task hmc...\r\n");

    ucResult = OSTaskCreate(UHMC_TaskProc, (void *)0, (OS_STK *)&g_UserTaskHmcSTK[DEFAULT_USER_HMC_STK_SIZE - 1], DEFAULT_USER_TASK_HMC_PRIO);

    if (OS_ERR_NONE == ucResult)
    {
        return UCORE_ERR_SUCCESS;
    }

    UCORE_DEBUG_PRINT("User task hmc Failed value:%d.\r\n", ucResult);

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void UHMC_TaskProc(void *p_arg)
{
    UCORE_DEBUG_PRINT("User Task hmc Start.\r\n");

    /* call init */
    UHMC_Init();

    /* enter event loop */
    UHMC_EventLoop();

    UCORE_DEBUG_PRINT("User Task hmc Exit.\r\n");
}

void UHMC_Init(void)
{
    g_QMboxHmcMsg = OSMboxCreate((void *)0);

    //初始化HMC5883L
    if (TRUE != Single_Write(HMC5883L_Addr,0x00,0x14))
    {
        UCORE_DEBUG_PRINT("初始化失败!\r\n");
        return;
    }
    
    Single_Write(HMC5883L_Addr,0x02,0x00);   
  
    UCORE_DEBUG_PRINT("User task hmc init finished.\r\n");
}

void UHMC_EventLoop(void)
{    
    while (1)
    {
        read_hmc5883l();

        OSTimeDlyHMSM(0, 0, 0, 1000);
    }
}
