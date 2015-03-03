/**********************************************************************************
 * 文件名  ：W5100.c
 * 描述    ：W5100 驱动函数库         
 * 库版本  ：ST_v3.5
 * 作者    ：泥人通信模块开发团队
 * 博客    ：http://nirenelec.blog.163.com
 * 淘宝    ：http://nirenelec.taobao.com
 * 说明：
 * 		本软件包括5个部分：
 *   	1. W5100初始化
 *   	2. W5100的Socket初始化
 *   	3. Socket连接
 *   	   如果Socket设置为TCP服务器模式，则调用Socket_Listen()函数,W5100处于侦听状态，直到远程客户端与它连接。
 *   	   如果Socket设置为TCP客户端模式，则调用Socket_Connect()函数，
 *   	                                  每调用一次Socket_Connect(s)函数，产生一次连接，
 *   	                                  如果连接不成功，则产生超时中断，然后可以再调用该函数进行连接。
 *   	   如果Socket设置为UDP模式,则调用Socket_UDP函数
 *   	4. Socket数据接收和发送
 *   	5. W5100中断处理

 *   置W5100为服务器模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)，设置过程即完成，等待客户端的连接。
 *   置W5100为客户端模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)，设置过程即完成，并与远程服务器连接。
 *   置W5100为UDP模式的调用过程：W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)，设置过程即完成，可以与远程主机UDP通信。

 *   W5100产生的连接成功、终止连接、接收数据、发送数据、超时等事件，都可以从中断状态中获得。
**********************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ioCC2530.h>
#include "OnBoard.h"
#include "hal_defs.h"
#include "W5100.h"
#include "SerialApp.h"
/*include jstack inc */
#include "JStack\jstack_defs.h"
#include "JStack\jstack_func.h"

/***************----- 网络参数变量定义 -----***************/
unsigned char Gateway_IP[4];//网关IP地址 
unsigned char Sub_Mask[4];	//子网掩码 
unsigned char Phy_Addr[6];	//物理地址(MAC) 
unsigned char IP_Addr[4];	//本机IP地址 

unsigned char S0_Port[2];	//端口0的端口号
unsigned char S0_DIP[4];	//端口0目的IP地址 
unsigned char S0_DPort[2];	//端口0目的端口号

unsigned char UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
unsigned char UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
unsigned char S0_Mode =3;	//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式

#define TCP_SERVER	0x00	//TCP服务器模式
#define TCP_CLIENT	0x01	//TCP客户端模式 
#define UDP_MODE	0x02	//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
unsigned char S0_State =0;	//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据) 
#define S_INIT		0x01	//端口完成初始化 
#define S_CONN		0x02	//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
unsigned char S0_Data;		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE	 0x01	//端口接收到一个数据包 
#define S_TRANSMITOK 0x02	//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
unsigned char Rx_Buffer[30];	//端口接收数据缓冲区 
unsigned char Tx_Buffer[30];	//端口发送数据缓冲区 

unsigned char W5100_Interrupt;	//W5100中断标志(0:无中断,1:有中断)

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ local functions define here -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static unsigned char SPI_ReadByte(void);
static void SPI_SendByte(unsigned char dt);
static unsigned char Read_W5100(unsigned short addr);
static void Write_W5100(unsigned short addr, unsigned char dat);
static void Load_Net_Parameters(void);
static void CC2530_IO_config(void);

/*******************************************************************************
* 函数名  : SPI_ReadByte
* 描述    : 读取一个字节SPI返回的数据
* 输入    : 无
* 输出    : 无
* 返回值  : 读取到的寄存器数据
* 说明    : 无
*******************************************************************************/
unsigned char SPI_ReadByte(void)
{
unsigned char rByte=0;

#if defined(W5100_USING_CC2530_SPI)
    /* 发送无用指令，获得时钟信号 */
    U1DBUF = 0X00;
    /* 等待传输完成 */
    while(!U1TX_BYTE);
    /* 清标志位 */
    U1TX_BYTE = 0;

    /* 读取总线的数据 */
    rByte = U1DBUF;
#else
	unsigned char i;
	
	for(i=0;i<8;i++)
	{
		rByte<<=1;
		rByte|=W5100_MISO;
		W5100_SCLK=0;
		Delay(10);
		W5100_SCLK=1;
		W5100_SCLK=0;
	}	
#endif

    return rByte;
}

/*******************************************************************************
* 函数名  : SPI_SendByte
* 描述    : 发送一个字节数据
* 输入    : dt:待发送的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI_SendByte(unsigned char dt)
{
#if defined(W5100_USING_CC2530_SPI)
    U1DBUF = dt; 
    /* 等待传输完成 */
    while(!U1TX_BYTE);
    /* 清标志位 */
    U1TX_BYTE = 0;
#else
	unsigned char i;
	
	for(i=0;i<8;i++)
	{
		if((dt<<i)&0x80)
		{
			W5100_MOSI=1;
		}
		else
		{
			W5100_MOSI=0;
		}
		W5100_SCLK=0;
		Delay(10);
		W5100_SCLK=1;
		W5100_SCLK=0;
	}
#endif
}

/*******************************************************************************
* 函数名  : Read_W5100
* 描述    : 读W5100指定地址的寄存器数据
* 输入    : addr:(地址)
* 输出    : 无
* 返回值  : 读取到的寄存器数据
* 说明    : 从W5100指定的地址读取一个字节
*******************************************************************************/
unsigned char Read_W5100(unsigned short addr)
{
	unsigned char i;

	W5100_SCS=0;			            //置W5100的CS为低电平
	SPI_SendByte(0x0f);		            //发送读命令
	SPI_SendByte(HI_UINT16(addr));	    //发送地址高位
	SPI_SendByte(LO_UINT16(addr));		//发送地址低位
	i=SPI_ReadByte();		            //读取数据 
	W5100_SCS=1;			            //置W5100的CS为高电平

	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Write_W5100
* 描述    : 向W5100指定地址寄存器写一个字节数据
* 输入    : addr:(地址) dat:(待写入数据)
* 输出    : 无
* 返回值  : 无
* 说明    : 将一个字节写入W5100指定的地址
*******************************************************************************/
void Write_W5100(unsigned short addr, unsigned char dat)
{
	W5100_SCS=0;			        //置W5100的CS为低电平
    //Delay(1);				        //延时一会
	SPI_SendByte(0xf0);		        //发送写命令
	SPI_SendByte(HI_UINT16(addr));	//发送地址高位
	SPI_SendByte(LO_UINT16(addr));	//发送地址低位 
	SPI_SendByte(dat);		        //写入数据
    //Delay(1);				        //延时一会
 	W5100_SCS=1;			        //置W5100的CS为高电平 
}

/*******************************************************************************
* 函数名  : W5100_Hardware_Reset
* 描述    : 硬件复位W5100
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5100_Hardware_Reset(void)
{
	W5100_RST = 0;
	//P2=0;
	Delay(100);
	W5100_RST = 1;
	Delay(100);
	//P2=0xff;
}

/*******************************************************************************
* 函数名  : W5100_Init
* 描述    : 初始化W5100寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5100之前，先对W5100初始化
*******************************************************************************/
void  W5100_Init(void)
{
	unsigned char i;

	W5100_SCLK =0;
	W5100_SCS =0;
	W5100_MOSI =0;
        
	Write_W5100(W5100_MODE,MODE_RST);//软件复位W5100,置1有效,复位后自动清0
	Delay(100);//延时100ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义 
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	for(i=0;i<4;i++)
		Write_W5100(W5100_GAR+i,Gateway_IP[i]);	
			
	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	for(i=0;i<4;i++)
		Write_W5100(W5100_SUBR+i,Sub_Mask[i]);		
	
	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	for(i=0;i<6;i++)
		Write_W5100(W5100_SHAR+i,Phy_Addr[i]);		

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	for(i=0;i<4;i++)
		Write_W5100(W5100_SIPR+i,IP_Addr[i]);		
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5100数据手册
	//初始化值为0x55,4个端口(0~3)分别分配2K的存储空间
	Write_W5100(W5100_RMSR,0x55);	//Socket Rx memory size=2k
	Write_W5100(W5100_TMSR,0x55);	//Socket Tx mempry size=2k

	//设置重试时间，默认为2000(200ms) 
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	Write_W5100(W5100_RTR,0x07);
	Write_W5100(W5100_RTR+1,0xd0);

	//设置重试次数，默认为8次 
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	Write_W5100(W5100_RCR,8);

	//启动中断，参考W5100数据手册确定自己需要的中断类型
	//IMR_CONFLICT是IP地址冲突异常中断,IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	//其它是Socket事件中断，根据需要添加
	Write_W5100(W5100_IMR,(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT|IMR_S1_INT|IMR_S2_INT|IMR_S3_INT));
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器
* 输入    : 无
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 无
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char i;

	Write_W5100((W5100_S0_MR),S_MR_TCP + S_MR_MC);//设置socket0为TCP模式
	Write_W5100((W5100_S0_CR),S_CR_OPEN);//打开socket0
	//如果socket0打开失败
	if(Read_W5100(W5100_S0_SSR)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR),S_CR_CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	//检查网关及获取网关的物理地址
	for(i=0;i<4;i++)
		Write_W5100((W5100_S0_DIPR+i),IP_Addr[i]+1);//向目的地址寄存器写入与本机IP不同的IP值
	//打开socket0的TCP连接
	Write_W5100((W5100_S0_CR),S_CR_CONNECT);
	//延时20ms 
	Delay(50);						
	//读取目的主机的物理地址，该地址就是网关地址
	i=Read_W5100(W5100_S0_DHAR);	
	//关闭socket0
	Write_W5100((W5100_S0_CR),S_CR_CLOSE);	

	if(i==0xff)
	{
		/**********没有找到网关服务器，或没有与网关服务器成功连接***********/
		/**********              自己添加处理代码                ***********/
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~3)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	unsigned int i;

	//设置分片长度，参考W5100数据手册，该值可以不修改
	//最大分片字节数=30(0x1e)
	Write_W5100((W5100_S0_MSS+s*0x100),0x00);		
	Write_W5100((W5100_S0_MSS+s*0x100+1),0x1e);
	//设置指定端口
	switch(s)
	{
		case 0:
			//设置端口0的端口号
			Write_W5100(W5100_S0_PORT,S0_Port[0]);
			Write_W5100(W5100_S0_PORT+1,S0_Port[1]);
			//设置端口0目的(远程)端口号
			Write_W5100(W5100_S0_DPORT,S0_DPort[0]);	
			Write_W5100(W5100_S0_DPORT+1,S0_DPort[1]);
			//设置端口0目的(远程)IP地址
			for(i=0;i<4;i++)
				Write_W5100(W5100_S0_DIPR+i,S0_DIP[i]);	
			break;

		case 1:
			break;

		case 2:
			break;

		case 3:
			break;

		default:
			break;
	}
}

/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : 设置指定Socket(0~3)为客户端与远程服务器连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在客户端模式时,引用该程序,与远程服务器建立连接
*			如果启动连接后出现超时中断，则与服务器连接失败,需要重新调用该程序连接
*			该程序每调用一次,就与服务器产生一次连接
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);//设置socket为TCP模式
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//打开Socket
	//如果socket打开失败
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		Write_W5100(W5100_S0_CR+s*0x100,S_CR_CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	//设置Socket为Connect模式
	Write_W5100((W5100_S0_CR+s*0x100),S_CR_CONNECT);

	return TRUE;

	//至此完成了Socket的打开连接工作,至于它是否与远程服务器建立连接,则需要等待Socket中断，
	//以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定Socket(0~3)作为服务器等待远程主机的连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*			该程序只调用一次,就使W5100设置为服务器模式
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);//设置socket为TCP模式 
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//打开Socket
	//如果socket打开失败
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR+s*0x100),S_CR_CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	//设置Socket为侦听模式
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_LISTEN);
	//如果socket设置失败
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_LISTEN)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);//设置不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}

	return TRUE;

	//至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
	//以判断Socket的连接是否成功。参考W5100数据手册的Socket中断状态
	//在服务器侦听模式不需要设置目的IP和目的端口号
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定Socket(0~3)为UDP模式
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需要建立连接
*			该程序只调用一次，就使W5100设置为UDP模式
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_UDP);//设置Socket为UDP模式*/
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//打开Socket*/
	//如果Socket打开失败
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_UDP)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	else
		return TRUE;

	//至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
	//因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	//如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}

/*******************************************************************************
* 函数名  : S_rx_process
* 描述    : 指定Socket(0~3)接收数据处理
* 输入    : s:端口
* 输出    : 无
* 返回值  : 返回接收到数据的长度
* 说明    : 如果Socket产生接收数据的中断,则引用该程序进行处理
*			该程序将Socket的接收到的数据缓存到Rx_buffer数组中,并返回接收的数据字节数
*******************************************************************************/
unsigned short S_rx_process(SOCKET s)
{
    
	unsigned short i,j;
	unsigned short rx_size,rx_offset,rx_offset1;

	//读取接收数据的字节数
	rx_size=Read_W5100(W5100_S0_RX_RSR+s*0x100);
	rx_size*=256;
	rx_size+=Read_W5100(W5100_S0_RX_RSR+s*0x100+1);
	//读取接收缓冲区的偏移量
	rx_offset=Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset*=256;
	rx_offset+=Read_W5100(W5100_S0_RX_RR+s*0x100+1);
	rx_offset1=rx_offset;

	i=rx_offset/S_RX_SIZE; //计算实际的物理偏移量，S0_RX_SIZE需要在前面#define中定义
						   //注意S_RX_SIZE的值在W5100_Init()函数的W5100_RMSR中确定
	rx_offset=rx_offset-i*S_RX_SIZE;

	j=W5100_RX+s*S_RX_SIZE+rx_offset; //实际物理地址为W5100_RX+rx_offset
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			j=W5100_RX+s*S_RX_SIZE;
			rx_offset=0;
		}
		Rx_Buffer[i]=Read_W5100(j);	//将数据缓存到Rx_buffer数组中
		j++;
		rx_offset++;
	}
	//计算下一次偏移量
	rx_offset1+=rx_size;
	Write_W5100((W5100_S0_RX_RR+s*0x100), (rx_offset1/256));
	Write_W5100((W5100_S0_RX_RR+s*0x100+1), rx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_RECV);//设置RECV命令，等等下一次接收

	return rx_size;//返回接收的数据字节数	
}

/*******************************************************************************
* 函数名  : S_tx_process
* 描述    : 指定Socket(0~3)发送数据处理
* 输入    : s:端口,size(发送数据的长度)
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 要发送的数据缓存在Tx_buffer中
*******************************************************************************/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i,j;
	unsigned short tx_offset,tx_offset1;

	//如果是UDP模式,可以在此设置目的主机的IP和端口号
	if((Read_W5100(W5100_S0_MR+s*0x100)&0x0f)==0x02)
	{
		//设置目的主机IP
		for(i=0;i<4;i++)			
			Write_W5100((W5100_S0_DIPR+s*0x100+i), UDP_DIPR[i]);
  		//设置目的主机端口号
		Write_W5100((W5100_S0_DPORT+s*0x100), UDP_DPORT[0]);
		Write_W5100((W5100_S0_DPORT+s*0x100+1), UDP_DPORT[1]);
	}

	//读取缓冲区剩余的长度
	i=Read_W5100(W5100_S0_TX_FSR+s*0x100);
	i*=256;
	i+=Read_W5100(W5100_S0_TX_FSR+s*0x100+1);
	if(i<size) return FALSE; //如果剩余的字节长度小于发送字节长度,则返回*/
		
	//读取发送缓冲区的偏移量
	tx_offset=Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset*=256;
	tx_offset+=Read_W5100(W5100_S0_TX_WR+s*0x100+1);
	tx_offset1=tx_offset;

	i=tx_offset/S_TX_SIZE; //计算实际的物理偏移量，S0_TX_SIZE需要在前面#define中定义
						   //注意S0_TX_SIZE的值在W5100_Init()函数的W5100_TMSR中确定
	tx_offset=tx_offset-i*S_TX_SIZE;
	j=W5100_TX+s*S_TX_SIZE+tx_offset;//实际物理地址为W5100_TX+tx_offset
	//将待发送数据写入W5100发送缓冲区
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)//物理地址超出
		{
			j=W5100_TX+s*S_TX_SIZE;
			tx_offset=0;
		}
		Write_W5100(j,Tx_Buffer[i]);//将Tx_buffer缓冲区中的数据写入到发送缓冲区
		j++;
		tx_offset++;
	}
	//计算下一次的偏移量
	tx_offset1+=size;
	Write_W5100((W5100_S0_TX_WR+s*0x100),(tx_offset1/256));
	Write_W5100((W5100_S0_TX_WR+s*0x100+1),tx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_SEND);//设置SEND命令,启动发送

	return TRUE;//返回成功
}

/*******************************************************************************
* 函数名  : W5100_Interrupt_Process
* 描述    : W5100中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
#pragma vector = P0INT_VECTOR    
__interrupt void W5100_Interrupt_Process(void) 
{
	unsigned char i,j;

    if (!(P0IFG & BV(5)))
    {
        /*
         Clear the CPU interrupt flag for Port_0
         PxIFG has to be cleared before PxIF
        */
        P0IFG = 0;
        P0IF = 0;
        
        return;
    }
  
	W5100_Interrupt=0;//清零中断标志	
	i=Read_W5100(W5100_IR);//读取中断标志寄存器
	Write_W5100(W5100_IR, (i&0xf0));//回写清除中断标志

	if((i & IR_CONFLICT) == IR_CONFLICT)//IP地址冲突异常处理
	{
		 //自己添加代码
	}

	if((i & IR_UNREACH) == IR_UNREACH)//UDP模式下地址无法到达异常处理
	{
		//自己添加代码
	}
	
	if((i & IR_S0_INT) == IR_S0_INT)//Socket0事件处理 
	{
		j=Read_W5100(W5100_S0_IR);//读取Socket0中断标志寄存器
		Write_W5100(W5100_S0_IR, j);//回写清中断标志 

		if(j&S_IR_CON)//在TCP模式下,Socket0成功连接 
		{
			S0_State|=S_CONN;//网络连接状态0x02,端口完成连接，可以正常传输数据

            /* 初始化S0_Data，用于首包的标志位判断 */
            S0_Data|=S_TRANSMITOK;

            JMIDebug("conn success.\r\n");
		}
		if(j&S_IR_DISCON)//在TCP模式下Socket断开连接处理
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);// 关闭端口,等待重新打开连接 
			S0_State=0;//网络连接状态0x00,端口连接失败

            /* 设置一个重连定时器(5s) */            
            osal_start_timerEx( SerialApp_TaskID, SERIALAPP_TCP_CONN_EVT, 5000 ); 

            JMIDebug("conn broke.\r\n");
		}
		if(j&S_IR_SENDOK)//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据 
		{
			S0_Data|=S_TRANSMITOK;//端口发送一个数据包完成 
		}
		if(j&S_IR_RECV)//Socket接收到数据,可以启动S_rx_process()函数 
		{
			S0_Data|=S_RECEIVE;//端口接收到一个数据包
		}
		if(j&S_IR_TIMEOUT)//Socket连接或数据传输超时处理 
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);//关闭端口,等待重新打开连接
			S0_State=0;//网络连接状态0x00,端口连接失败

            /* 设置一个重连定时器(5s) */            
            osal_start_timerEx( SerialApp_TaskID, SERIALAPP_TCP_CONN_EVT, 5000 ); 

            JMIDebug("conn timeout.\r\n");
		}
	}

    /*
     Clear the CPU interrupt flag for Port_0
     PxIFG has to be cleared before PxIF
    */
    P0IFG = 0;
    P0IF = 0;    
}

/*******************************************************************************
* 函数名  : Delay
* 描述    : 延时函数(ms)
* 输入    : d:延时系数，单位为毫秒
* 输出    : 无
* 返回    : 无 
* 说明    : 延时是利用Timer2定时器产生的1毫秒的计数来实现的
*******************************************************************************/
void Delay(unsigned int  x)
{
  unsigned char i;
  while(x--)
  {
    for(i=0;i<100;i++)
     MicroWait(10);
  }
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
void Load_Net_Parameters()
{
	Gateway_IP[0] = 172;//加载网关参数
	Gateway_IP[1] = 200;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//加载子网掩码
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//加载物理地址
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=172;//加载本机IP地址
	IP_Addr[1]=200;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	S0_Port[0] = 0x13;//加载端口0的端口号5000 
	S0_Port[1] = 0x88;

	S0_DIP[0]=172;//加载端口0的目的IP地址
	S0_DIP[1]=200;
	S0_DIP[2]=1;
	S0_DIP[3]=100;
	
	S0_DPort[0] = 0x17;//加载端口0的目的端口号6000
	S0_DPort[1] = 0x70;

	S0_Mode=TCP_CLIENT;//加载端口0的工作模式,TCP客户端模式
}

/*******************************************************************************
* 函数名  : CC2530_IO_config
* 描述    : CC2530 SPI初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void CC2530_IO_config(void)
{
#if defined(W5100_USING_CC2530_SPI)
	// SPI Master Mode
	PERCFG |= 0x02;         // map USART1 to its alternative 2 location. P1_4: SSN, P1_5: SCK, P1_6: MOSI, P1_7: MISO
	P1SEL |= 0xE0;          // P1_5, P1_6, and P1_7 are peripherals
	P1SEL &= ~0x10;         // P1_4 is GPIO (SSN)
	P1DIR |= 0x10;          // SSN is set as output

	P0SEL &= ~0x10;         // P0_4 is GPIO (RESET)
	P0DIR |= 0x10;          // P0_4 is set as output
	
	U1BAUD = 0x00; U1GCR |= 0x11;        // Set baud rate to max (system clock frequency / 8)
	U1CSR &= ~0xA0;         // SPI Master Mode
	U1CSR |= 0x40;
	U1GCR &= ~0xC0; 
    U1GCR |= 0x20;          // MSB
#else
    P0SEL &= ~0x10;         // P0_4 is GPIO (RESET)
    P0DIR |= 0x10;          // RESET is set as output

    P1SEL &= ~0xF0;         // P1_4, P1_5, P1_6, and P1_7 are GPIO
    P1DIR = 0x70;           // SSN(P1_4), P1_5, P1_6 are output, and P1_7 is input 
#endif

    /* 定义外部中断 */
    P0IEN |= 0x20;          // P0_5 设置为中断方式 1：中断使能
    PICTL |= 0x1;           //下降沿触发   
    P0IE = 1;               //允许P0口中断; 
    P0IFG = 0x00;           //初始化中断标志位
    EA = 1;                 //打开总中断
}

/*******************************************************************************
* 函数名  : W5100_Initialization
* 描述    : W5100初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5100_Initialization()
{
    /* 首先配置IO口相关模式 */
    CC2530_IO_config();

    /* 装载网络参数 */
    Load_Net_Parameters();	

    /* 硬件复位W5100 */
    W5100_Hardware_Reset();

    /* 初始化W5100寄存器函数 */
	W5100_Init();

    /* 检查网关服务器 */
    Detect_Gateway(); 

    /* 指定Socket(0~3)初始化,初始化端口0 */
	Socket_Init(0);
}

/*******************************************************************************
* 函数名  : W5100_Socket_Start
* 描述    : W5100端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5100_Socket_Start(void)
{
	if(S0_State==0)//端口0初始化配置
	{
		if(S0_Mode==TCP_SERVER)//TCP服务器模式 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP客户端模式 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDP模式 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/* 定义W5100的发送接口 */
uint16 W5100_SocketSend(SOCKET s, unsigned char *pcBuff, unsigned int size)
{
    uint8 ucRet = FALSE;
    
    if ((NULL == pcBuff) || (30 < size))
    {
        return 0;
    }

    /* 检查状态 */   
    if (!(S0_State & S_CONN))
    {
        return 0;
    }

    /* 等待原数据发送完成 */
    while (!(S0_Data & S_TRANSMITOK));

    /* clear flag */
	S0_Data&=~S_TRANSMITOK;

    /* copy data */
	memcpy(Tx_Buffer, pcBuff, size);	

    /* send */
	ucRet = S_tx_process(0, size);
    if (TRUE != ucRet)
    {
        return 0;
    }

    return size;    
}

/* 定义W5100的接收接口 */
uint16 W5100_SocketRecv(SOCKET s, unsigned char **ppcBuff)
{
    uint8 ucRet = FALSE;

    /* recv from socket */
    ucRet = S_rx_process(0);
    if (0 >= ucRet)
    {
        *ppcBuff = NULL;
        return 0;
    }

    /* 指向接收缓存 */
    *ppcBuff = Rx_Buffer;
    
    /* 返回接收到的数据大小 */
    return ucRet;
}


