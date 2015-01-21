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
#include <user_core.h>
#include <user_task_hmc.h>


#define	HMC5883L_Addr   0x3C

#define SCL_H         GPIOB->BSRR = GPIO_Pin_6
#define SCL_L         GPIOB->BRR  = GPIO_Pin_6 
   
#define SDA_H         GPIOB->BSRR = GPIO_Pin_7
#define SDA_L         GPIOB->BRR  = GPIO_Pin_7

#define SCL_read      GPIOB->IDR  & GPIO_Pin_6
#define SDA_read      GPIOB->IDR  & GPIO_Pin_7

/*----------------local global vars declare here---------------------*/
static OS_STK       g_UserTaskHmcSTK[DEFAULT_USER_LCD_STK_SIZE];

OS_EVENT            *g_QMboxHmcMsg = NULL;

u8                  test = 0; 
u16                 angle;
/*----------------local global funcs declare here---------------------*/
static void UHMC_TaskProc(void *p_arg);
static void UHMC_Init(void);
static void UHMC_EventLoop(void);

void I2C_GPIO_Config(void);
bool I2C_Start(void);
void I2C_Stop(void);
void I2C_Ack(void);
void I2C_NoAck(void);
bool I2C_WaitAck(void); 	 //返回为:=1有ACK,=0无ACK
void I2C_SendByte(u8 SendByte); //数据从高位到低位//
unsigned char I2C_RadeByte(void);  //数据从高位到低位//

/*
********************************************************************************
** 函数名称 ： void Delayms(vu32 m)
** 函数功能 ： 长延时函数	 m=1,延时1ms
** 输    入	： 无
** 输    出	： 无
** 返    回	： 无
********************************************************************************
*/
 void Delayms(int m)
{
  int i;
  
  for(; m != 0; m--)	
       for (i=0; i<50000; i++);
}

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : Simulation IIC Timing series delay
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_delay(void)
{
		
   char i=30; //这里可以优化速度	，经测试最低到5还能写入
   while(i) 
   { 
     i--; 
   }  
}

void delay5ms(void)
{
		
   int i=5000;  
   while(i) 
   { 
     i--; 
   }  
}

void I2C_GPIO_Config(void)
{
 	RCC->APB2ENR|=1<<3;//先使能外设IO PORTC时钟 							 
	GPIOB->CRL&=0X00FFFFFF;//PC11/12 推挽输出
	GPIOB->CRL|=0X33000000;	   
	GPIOB->ODR|=3<<6;     //PC11,12 输出高
}

bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)return FALSE;	//SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) return FALSE;	//SDA线为高电平则总线出错,退出
	SDA_L;
	I2C_delay();
	return TRUE;
}
void I2C_Stop(void)
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;
	I2C_delay();
} 
void I2C_Ack(void)
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}   
void I2C_NoAck(void)
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
} 
bool I2C_WaitAck(void) 	 //返回为:=1有ACK,=0无ACK
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
	  I2C_delay();
      return FALSE;
	}
	SCL_L;
	I2C_delay();
	return TRUE;
}
void I2C_SendByte(u8 SendByte) //数据从高位到低位//
{
    u8 i=8;
    while(i--)
    {
        SCL_L;
        I2C_delay();
      if(SendByte&0x80)
        SDA_H;  
      else 
        SDA_L;   
        SendByte<<=1;
        I2C_delay();
		SCL_H;
        I2C_delay();
    }
    SCL_L;
}  
unsigned char I2C_RadeByte(void)  //数据从高位到低位//
{ 
    u8 i=8;
    u8 ReceiveByte=0;

    SDA_H;				
    while(i--)
    {
      ReceiveByte<<=1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)
      {
        ReceiveByte|=0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
} 

bool Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)		     //void
{
  	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress);   //发送设备地址+写信号//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop(); return FALSE;}
    I2C_SendByte(REG_Address );   //设置低起始地址      
    I2C_WaitAck();	
    I2C_SendByte(REG_data);
    I2C_WaitAck();   
    I2C_Stop(); 
    delay5ms();
    return TRUE;
}

unsigned char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{   unsigned char REG_data;     	
	if(!I2C_Start())return FALSE;
    I2C_SendByte(SlaveAddress); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(!I2C_WaitAck()){I2C_Stop();test=1; return FALSE;}
    I2C_SendByte((u8) REG_Address);   //设置低起始地址      
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(SlaveAddress+1);
    I2C_WaitAck();

	REG_data= I2C_RadeByte();
    I2C_NoAck();
    I2C_Stop();
    //return TRUE;
	return REG_data;

}

void conversion(u16 temp_data)  
{   
	printf("现在的角度为:=%d°\n",temp_data);
    Delayms(20);		
}

void read_hmc5883l()
{
    unsigned char BUF[8];                         //接收数据缓存区
    int   x,y;

    Single_Write(HMC5883L_Addr,0x00,0x14);    
    Single_Write(HMC5883L_Addr,0x02,0x00);   
    Delayms(10);

    BUF[1]=Single_Read(HMC5883L_Addr,0x03); 
    BUF[2]=Single_Read(HMC5883L_Addr,0x04); 

    BUF[3]=Single_Read(HMC5883L_Addr,0x07); 
    BUF[4]=Single_Read(HMC5883L_Addr,0x08); 

    x=(BUF[1] << 8) | BUF[2];  
    y=(BUF[3] << 8) | BUF[4];  

    if(x>0x7fff)x-=0xffff;	  
    if(y>0x7fff)y-=0xffff;	  
    angle= atan2(y,x) * (180 / 3.14159265) + 180;  
    conversion(angle);
}

u8 UHMC_Start(void)
{
    u8          ucResult    = UCORE_ERR_COMMON_FAILED;

    printf("starting user task hmc...\r\n");

    ucResult = OSTaskCreate(UHMC_TaskProc, (void *)0, (OS_STK *)&g_UserTaskHmcSTK[DEFAULT_USER_HMC_STK_SIZE - 1], DEFAULT_USER_TASK_HMC_PRIO);

    if (OS_ERR_NONE == ucResult)
    {
        return UCORE_ERR_SUCCESS;
    }

    printf("User task hmc Failed value:%d.\r\n", ucResult);

    return UCORE_ERR_CREATE_TASK_FAILED;
}

void UHMC_TaskProc(void *p_arg)
{
    printf("User Task hmc Start.\r\n");

    /* call init */
    UHMC_Init();

    /* enter event loop */
    UHMC_EventLoop();

    printf("User Task hmc Exit.\r\n");
}

void UHMC_Init(void)
{
    g_QMboxHmcMsg = OSMboxCreate((void *)0);

    I2C_GPIO_Config();
    //初始化HMC5883L
    if (TRUE != Single_Write(HMC5883L_Addr,0x00,0x14))
    {
        printf("初始化失败!\r\n");
        return;
    }
    Single_Write(HMC5883L_Addr,0x02,0x00);   
  
    printf("User task hmc init finished.\r\n");
}

void UHMC_EventLoop(void)
{
    INT8U           err     = 0;    
    UCORE_MSG_S     *pMsg   = NULL;   	
    
    while (1)
    {
        read_hmc5883l();

        OSTimeDlyHMSM(0, 0, 0, 1000);
    }
}
