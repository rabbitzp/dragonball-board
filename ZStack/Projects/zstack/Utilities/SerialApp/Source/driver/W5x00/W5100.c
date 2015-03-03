/**********************************************************************************
 * �ļ���  ��W5100.c
 * ����    ��W5100 ����������         
 * ��汾  ��ST_v3.5
 * ����    ������ͨ��ģ�鿪���Ŷ�
 * ����    ��http://nirenelec.blog.163.com
 * �Ա�    ��http://nirenelec.taobao.com
 * ˵����
 * 		���������5�����֣�
 *   	1. W5100��ʼ��
 *   	2. W5100��Socket��ʼ��
 *   	3. Socket����
 *   	   ���Socket����ΪTCP������ģʽ�������Socket_Listen()����,W5100��������״̬��ֱ��Զ�̿ͻ����������ӡ�
 *   	   ���Socket����ΪTCP�ͻ���ģʽ�������Socket_Connect()������
 *   	                                  ÿ����һ��Socket_Connect(s)����������һ�����ӣ�
 *   	                                  ������Ӳ��ɹ����������ʱ�жϣ�Ȼ������ٵ��øú����������ӡ�
 *   	   ���Socket����ΪUDPģʽ,�����Socket_UDP����
 *   	4. Socket���ݽ��պͷ���
 *   	5. W5100�жϴ���

 *   ��W5100Ϊ������ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Listen(s)�����ù��̼���ɣ��ȴ��ͻ��˵����ӡ�
 *   ��W5100Ϊ�ͻ���ģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_Connect(s)�����ù��̼���ɣ�����Զ�̷��������ӡ�
 *   ��W5100ΪUDPģʽ�ĵ��ù��̣�W5100_Init()-->Socket_Init(s)-->Socket_UDP(s)�����ù��̼���ɣ�������Զ������UDPͨ�š�

 *   W5100���������ӳɹ�����ֹ���ӡ��������ݡ��������ݡ���ʱ���¼��������Դ��ж�״̬�л�á�
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

/***************----- ��������������� -----***************/
unsigned char Gateway_IP[4];//����IP��ַ 
unsigned char Sub_Mask[4];	//�������� 
unsigned char Phy_Addr[6];	//�����ַ(MAC) 
unsigned char IP_Addr[4];	//����IP��ַ 

unsigned char S0_Port[2];	//�˿�0�Ķ˿ں�
unsigned char S0_DIP[4];	//�˿�0Ŀ��IP��ַ 
unsigned char S0_DPort[2];	//�˿�0Ŀ�Ķ˿ں�

unsigned char UDP_DIPR[4];	//UDP(�㲥)ģʽ,Ŀ������IP��ַ
unsigned char UDP_DPORT[2];	//UDP(�㲥)ģʽ,Ŀ�������˿ں�

/***************----- �˿ڵ�����ģʽ -----***************/
unsigned char S0_Mode =3;	//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ

#define TCP_SERVER	0x00	//TCP������ģʽ
#define TCP_CLIENT	0x01	//TCP�ͻ���ģʽ 
#define UDP_MODE	0x02	//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
unsigned char S0_State =0;	//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������) 
#define S_INIT		0x01	//�˿���ɳ�ʼ�� 
#define S_CONN		0x02	//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
unsigned char S0_Data;		//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
#define S_RECEIVE	 0x01	//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK 0x02	//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
unsigned char Rx_Buffer[30];	//�˿ڽ������ݻ����� 
unsigned char Tx_Buffer[30];	//�˿ڷ������ݻ����� 

unsigned char W5100_Interrupt;	//W5100�жϱ�־(0:���ж�,1:���ж�)

/*-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ local functions define here -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*/
static unsigned char SPI_ReadByte(void);
static void SPI_SendByte(unsigned char dt);
static unsigned char Read_W5100(unsigned short addr);
static void Write_W5100(unsigned short addr, unsigned char dat);
static void Load_Net_Parameters(void);
static void CC2530_IO_config(void);

/*******************************************************************************
* ������  : SPI_ReadByte
* ����    : ��ȡһ���ֽ�SPI���ص�����
* ����    : ��
* ���    : ��
* ����ֵ  : ��ȡ���ļĴ�������
* ˵��    : ��
*******************************************************************************/
unsigned char SPI_ReadByte(void)
{
unsigned char rByte=0;

#if defined(W5100_USING_CC2530_SPI)
    /* ��������ָ����ʱ���ź� */
    U1DBUF = 0X00;
    /* �ȴ�������� */
    while(!U1TX_BYTE);
    /* ���־λ */
    U1TX_BYTE = 0;

    /* ��ȡ���ߵ����� */
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
* ������  : SPI_SendByte
* ����    : ����һ���ֽ�����
* ����    : dt:�����͵�����
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void SPI_SendByte(unsigned char dt)
{
#if defined(W5100_USING_CC2530_SPI)
    U1DBUF = dt; 
    /* �ȴ�������� */
    while(!U1TX_BYTE);
    /* ���־λ */
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
* ������  : Read_W5100
* ����    : ��W5100ָ����ַ�ļĴ�������
* ����    : addr:(��ַ)
* ���    : ��
* ����ֵ  : ��ȡ���ļĴ�������
* ˵��    : ��W5100ָ���ĵ�ַ��ȡһ���ֽ�
*******************************************************************************/
unsigned char Read_W5100(unsigned short addr)
{
	unsigned char i;

	W5100_SCS=0;			            //��W5100��CSΪ�͵�ƽ
	SPI_SendByte(0x0f);		            //���Ͷ�����
	SPI_SendByte(HI_UINT16(addr));	    //���͵�ַ��λ
	SPI_SendByte(LO_UINT16(addr));		//���͵�ַ��λ
	i=SPI_ReadByte();		            //��ȡ���� 
	W5100_SCS=1;			            //��W5100��CSΪ�ߵ�ƽ

	return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Write_W5100
* ����    : ��W5100ָ����ַ�Ĵ���дһ���ֽ�����
* ����    : addr:(��ַ) dat:(��д������)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��һ���ֽ�д��W5100ָ���ĵ�ַ
*******************************************************************************/
void Write_W5100(unsigned short addr, unsigned char dat)
{
	W5100_SCS=0;			        //��W5100��CSΪ�͵�ƽ
    //Delay(1);				        //��ʱһ��
	SPI_SendByte(0xf0);		        //����д����
	SPI_SendByte(HI_UINT16(addr));	//���͵�ַ��λ
	SPI_SendByte(LO_UINT16(addr));	//���͵�ַ��λ 
	SPI_SendByte(dat);		        //д������
    //Delay(1);				        //��ʱһ��
 	W5100_SCS=1;			        //��W5100��CSΪ�ߵ�ƽ 
}

/*******************************************************************************
* ������  : W5100_Hardware_Reset
* ����    : Ӳ����λW5100
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
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
* ������  : W5100_Init
* ����    : ��ʼ��W5100�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5100֮ǰ���ȶ�W5100��ʼ��
*******************************************************************************/
void  W5100_Init(void)
{
	unsigned char i;

	W5100_SCLK =0;
	W5100_SCS =0;
	W5100_MOSI =0;
        
	Write_W5100(W5100_MODE,MODE_RST);//�����λW5100,��1��Ч,��λ���Զ���0
	Delay(100);//��ʱ100ms,�Լ�����ú���

	//��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ����� 
	//ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
	for(i=0;i<4;i++)
		Write_W5100(W5100_GAR+i,Gateway_IP[i]);	
			
	//������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
	//��������������������
	for(i=0;i<4;i++)
		Write_W5100(W5100_SUBR+i,Sub_Mask[i]);		
	
	//���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
	//�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	//����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
	for(i=0;i<6;i++)
		Write_W5100(W5100_SHAR+i,Phy_Addr[i]);		

	//���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
	//ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
	for(i=0;i<4;i++)
		Write_W5100(W5100_SIPR+i,IP_Addr[i]);		
	
	//���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5100�����ֲ�
	//��ʼ��ֵΪ0x55,4���˿�(0~3)�ֱ����2K�Ĵ洢�ռ�
	Write_W5100(W5100_RMSR,0x55);	//Socket Rx memory size=2k
	Write_W5100(W5100_TMSR,0x55);	//Socket Tx mempry size=2k

	//��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	Write_W5100(W5100_RTR,0x07);
	Write_W5100(W5100_RTR+1,0xd0);

	//�������Դ�����Ĭ��Ϊ8�� 
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	Write_W5100(W5100_RCR,8);

	//�����жϣ��ο�W5100�����ֲ�ȷ���Լ���Ҫ���ж�����
	//IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�,IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	//������Socket�¼��жϣ�������Ҫ���
	Write_W5100(W5100_IMR,(IMR_CONFLICT|IMR_UNREACH|IMR_S0_INT|IMR_S1_INT|IMR_S2_INT|IMR_S3_INT));
}

/*******************************************************************************
* ������  : Detect_Gateway
* ����    : ������ط�����
* ����    : ��
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ��
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char i;

	Write_W5100((W5100_S0_MR),S_MR_TCP + S_MR_MC);//����socket0ΪTCPģʽ
	Write_W5100((W5100_S0_CR),S_CR_OPEN);//��socket0
	//���socket0��ʧ��
	if(Read_W5100(W5100_S0_SSR)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR),S_CR_CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	//������ؼ���ȡ���ص������ַ
	for(i=0;i<4;i++)
		Write_W5100((W5100_S0_DIPR+i),IP_Addr[i]+1);//��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ
	//��socket0��TCP����
	Write_W5100((W5100_S0_CR),S_CR_CONNECT);
	//��ʱ20ms 
	Delay(50);						
	//��ȡĿ�������������ַ���õ�ַ�������ص�ַ
	i=Read_W5100(W5100_S0_DHAR);	
	//�ر�socket0
	Write_W5100((W5100_S0_CR),S_CR_CLOSE);	

	if(i==0xff)
	{
		/**********û���ҵ����ط���������û�������ط������ɹ�����***********/
		/**********              �Լ���Ӵ������                ***********/
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~3)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	unsigned int i;

	//���÷�Ƭ���ȣ��ο�W5100�����ֲᣬ��ֵ���Բ��޸�
	//����Ƭ�ֽ���=30(0x1e)
	Write_W5100((W5100_S0_MSS+s*0x100),0x00);		
	Write_W5100((W5100_S0_MSS+s*0x100+1),0x1e);
	//����ָ���˿�
	switch(s)
	{
		case 0:
			//���ö˿�0�Ķ˿ں�
			Write_W5100(W5100_S0_PORT,S0_Port[0]);
			Write_W5100(W5100_S0_PORT+1,S0_Port[1]);
			//���ö˿�0Ŀ��(Զ��)�˿ں�
			Write_W5100(W5100_S0_DPORT,S0_DPort[0]);	
			Write_W5100(W5100_S0_DPORT+1,S0_DPort[1]);
			//���ö˿�0Ŀ��(Զ��)IP��ַ
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
* ������  : Socket_Connect
* ����    : ����ָ��Socket(0~3)Ϊ�ͻ�����Զ�̷���������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڿͻ���ģʽʱ,���øó���,��Զ�̷�������������
*			����������Ӻ���ֳ�ʱ�жϣ��������������ʧ��,��Ҫ���µ��øó�������
*			�ó���ÿ����һ��,�������������һ������
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);//����socketΪTCPģʽ
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//��Socket
	//���socket��ʧ��
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		Write_W5100(W5100_S0_CR+s*0x100,S_CR_CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	//����SocketΪConnectģʽ
	Write_W5100((W5100_S0_CR+s*0x100),S_CR_CONNECT);

	return TRUE;

	//���������Socket�Ĵ����ӹ���,�������Ƿ���Զ�̷�������������,����Ҫ�ȴ�Socket�жϣ�
	//���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬
}

/*******************************************************************************
* ������  : Socket_Listen
* ����    : ����ָ��Socket(0~3)��Ϊ�������ȴ�Զ������������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڷ�����ģʽʱ,���øó���,�ȵ�Զ������������
*			�ó���ֻ����һ��,��ʹW5100����Ϊ������ģʽ
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_TCP);//����socketΪTCPģʽ 
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//��Socket
	//���socket��ʧ��
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_INIT)
	{
		Write_W5100((W5100_S0_CR+s*0x100),S_CR_CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	//����SocketΪ����ģʽ
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_LISTEN);
	//���socket����ʧ��
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_LISTEN)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);//���ò��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}

	return TRUE;

	//���������Socket�Ĵ򿪺�������������,����Զ�̿ͻ����Ƿ�������������,����Ҫ�ȴ�Socket�жϣ�
	//���ж�Socket�������Ƿ�ɹ����ο�W5100�����ֲ��Socket�ж�״̬
	//�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�
}

/*******************************************************************************
* ������  : Socket_UDP
* ����    : ����ָ��Socket(0~3)ΪUDPģʽ
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ���Socket������UDPģʽ,���øó���,��UDPģʽ��,Socketͨ�Ų���Ҫ��������
*			�ó���ֻ����һ�Σ���ʹW5100����ΪUDPģʽ
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5100((W5100_S0_MR+s*0x100), S_MR_UDP);//����SocketΪUDPģʽ*/
	Write_W5100((W5100_S0_CR+s*0x100), S_CR_OPEN);//��Socket*/
	//���Socket��ʧ��
	if(Read_W5100(W5100_S0_SSR+s*0x100)!=S_SSR_UDP)
	{
		Write_W5100((W5100_S0_CR+s*0x100), S_CR_CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	else
		return TRUE;

	//���������Socket�Ĵ򿪺�UDPģʽ����,������ģʽ��������Ҫ��Զ��������������
	//��ΪSocket����Ҫ��������,�����ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	//���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶���,�����й�����û�иı�,��ôҲ��������������
}

/*******************************************************************************
* ������  : S_rx_process
* ����    : ָ��Socket(0~3)�������ݴ���
* ����    : s:�˿�
* ���    : ��
* ����ֵ  : ���ؽ��յ����ݵĳ���
* ˵��    : ���Socket�����������ݵ��ж�,�����øó�����д���
*			�ó���Socket�Ľ��յ������ݻ��浽Rx_buffer������,�����ؽ��յ������ֽ���
*******************************************************************************/
unsigned short S_rx_process(SOCKET s)
{
    
	unsigned short i,j;
	unsigned short rx_size,rx_offset,rx_offset1;

	//��ȡ�������ݵ��ֽ���
	rx_size=Read_W5100(W5100_S0_RX_RSR+s*0x100);
	rx_size*=256;
	rx_size+=Read_W5100(W5100_S0_RX_RSR+s*0x100+1);
	//��ȡ���ջ�������ƫ����
	rx_offset=Read_W5100(W5100_S0_RX_RR+s*0x100);
	rx_offset*=256;
	rx_offset+=Read_W5100(W5100_S0_RX_RR+s*0x100+1);
	rx_offset1=rx_offset;

	i=rx_offset/S_RX_SIZE; //����ʵ�ʵ�����ƫ������S0_RX_SIZE��Ҫ��ǰ��#define�ж���
						   //ע��S_RX_SIZE��ֵ��W5100_Init()������W5100_RMSR��ȷ��
	rx_offset=rx_offset-i*S_RX_SIZE;

	j=W5100_RX+s*S_RX_SIZE+rx_offset; //ʵ�������ַΪW5100_RX+rx_offset
	for(i=0;i<rx_size;i++)
	{
		if(rx_offset>=S_RX_SIZE)
		{
			j=W5100_RX+s*S_RX_SIZE;
			rx_offset=0;
		}
		Rx_Buffer[i]=Read_W5100(j);	//�����ݻ��浽Rx_buffer������
		j++;
		rx_offset++;
	}
	//������һ��ƫ����
	rx_offset1+=rx_size;
	Write_W5100((W5100_S0_RX_RR+s*0x100), (rx_offset1/256));
	Write_W5100((W5100_S0_RX_RR+s*0x100+1), rx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_RECV);//����RECV����ȵ���һ�ν���

	return rx_size;//���ؽ��յ������ֽ���	
}

/*******************************************************************************
* ������  : S_tx_process
* ����    : ָ��Socket(0~3)�������ݴ���
* ����    : s:�˿�,size(�������ݵĳ���)
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : Ҫ���͵����ݻ�����Tx_buffer��
*******************************************************************************/
unsigned char S_tx_process(SOCKET s, unsigned int size)
{
	unsigned short i,j;
	unsigned short tx_offset,tx_offset1;

	//�����UDPģʽ,�����ڴ�����Ŀ��������IP�Ͷ˿ں�
	if((Read_W5100(W5100_S0_MR+s*0x100)&0x0f)==0x02)
	{
		//����Ŀ������IP
		for(i=0;i<4;i++)			
			Write_W5100((W5100_S0_DIPR+s*0x100+i), UDP_DIPR[i]);
  		//����Ŀ�������˿ں�
		Write_W5100((W5100_S0_DPORT+s*0x100), UDP_DPORT[0]);
		Write_W5100((W5100_S0_DPORT+s*0x100+1), UDP_DPORT[1]);
	}

	//��ȡ������ʣ��ĳ���
	i=Read_W5100(W5100_S0_TX_FSR+s*0x100);
	i*=256;
	i+=Read_W5100(W5100_S0_TX_FSR+s*0x100+1);
	if(i<size) return FALSE; //���ʣ����ֽڳ���С�ڷ����ֽڳ���,�򷵻�*/
		
	//��ȡ���ͻ�������ƫ����
	tx_offset=Read_W5100(W5100_S0_TX_WR+s*0x100);
	tx_offset*=256;
	tx_offset+=Read_W5100(W5100_S0_TX_WR+s*0x100+1);
	tx_offset1=tx_offset;

	i=tx_offset/S_TX_SIZE; //����ʵ�ʵ�����ƫ������S0_TX_SIZE��Ҫ��ǰ��#define�ж���
						   //ע��S0_TX_SIZE��ֵ��W5100_Init()������W5100_TMSR��ȷ��
	tx_offset=tx_offset-i*S_TX_SIZE;
	j=W5100_TX+s*S_TX_SIZE+tx_offset;//ʵ�������ַΪW5100_TX+tx_offset
	//������������д��W5100���ͻ�����
	for(i=0;i<size;i++)
	{
		if(tx_offset>=S_TX_SIZE)//�����ַ����
		{
			j=W5100_TX+s*S_TX_SIZE;
			tx_offset=0;
		}
		Write_W5100(j,Tx_Buffer[i]);//��Tx_buffer�������е�����д�뵽���ͻ�����
		j++;
		tx_offset++;
	}
	//������һ�ε�ƫ����
	tx_offset1+=size;
	Write_W5100((W5100_S0_TX_WR+s*0x100),(tx_offset1/256));
	Write_W5100((W5100_S0_TX_WR+s*0x100+1),tx_offset1);

	Write_W5100((W5100_S0_CR+s*0x100), S_CR_SEND);//����SEND����,��������

	return TRUE;//���سɹ�
}

/*******************************************************************************
* ������  : W5100_Interrupt_Process
* ����    : W5100�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
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
  
	W5100_Interrupt=0;//�����жϱ�־	
	i=Read_W5100(W5100_IR);//��ȡ�жϱ�־�Ĵ���
	Write_W5100(W5100_IR, (i&0xf0));//��д����жϱ�־

	if((i & IR_CONFLICT) == IR_CONFLICT)//IP��ַ��ͻ�쳣����
	{
		 //�Լ���Ӵ���
	}

	if((i & IR_UNREACH) == IR_UNREACH)//UDPģʽ�µ�ַ�޷������쳣����
	{
		//�Լ���Ӵ���
	}
	
	if((i & IR_S0_INT) == IR_S0_INT)//Socket0�¼����� 
	{
		j=Read_W5100(W5100_S0_IR);//��ȡSocket0�жϱ�־�Ĵ���
		Write_W5100(W5100_S0_IR, j);//��д���жϱ�־ 

		if(j&S_IR_CON)//��TCPģʽ��,Socket0�ɹ����� 
		{
			S0_State|=S_CONN;//��������״̬0x02,�˿�������ӣ�����������������

            /* ��ʼ��S0_Data�������װ��ı�־λ�ж� */
            S0_Data|=S_TRANSMITOK;

            JMIDebug("conn success.\r\n");
		}
		if(j&S_IR_DISCON)//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);// �رն˿�,�ȴ����´����� 
			S0_State=0;//��������״̬0x00,�˿�����ʧ��

            /* ����һ��������ʱ��(5s) */            
            osal_start_timerEx( SerialApp_TaskID, SERIALAPP_TCP_CONN_EVT, 5000 ); 

            JMIDebug("conn broke.\r\n");
		}
		if(j&S_IR_SENDOK)//Socket0���ݷ������,�����ٴ�����S_tx_process()������������ 
		{
			S0_Data|=S_TRANSMITOK;//�˿ڷ���һ�����ݰ���� 
		}
		if(j&S_IR_RECV)//Socket���յ�����,��������S_rx_process()���� 
		{
			S0_Data|=S_RECEIVE;//�˿ڽ��յ�һ�����ݰ�
		}
		if(j&S_IR_TIMEOUT)//Socket���ӻ����ݴ��䳬ʱ���� 
		{
			Write_W5100(W5100_S0_CR, S_CR_CLOSE);//�رն˿�,�ȴ����´�����
			S0_State=0;//��������״̬0x00,�˿�����ʧ��

            /* ����һ��������ʱ��(5s) */            
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
* ������  : Delay
* ����    : ��ʱ����(ms)
* ����    : d:��ʱϵ������λΪ����
* ���    : ��
* ����    : �� 
* ˵��    : ��ʱ������Timer2��ʱ��������1����ļ�����ʵ�ֵ�
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
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢�����ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void Load_Net_Parameters()
{
	Gateway_IP[0] = 172;//�������ز���
	Gateway_IP[1] = 200;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//������������
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//���������ַ
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=172;//���ر���IP��ַ
	IP_Addr[1]=200;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	S0_Port[0] = 0x13;//���ض˿�0�Ķ˿ں�5000 
	S0_Port[1] = 0x88;

	S0_DIP[0]=172;//���ض˿�0��Ŀ��IP��ַ
	S0_DIP[1]=200;
	S0_DIP[2]=1;
	S0_DIP[3]=100;
	
	S0_DPort[0] = 0x17;//���ض˿�0��Ŀ�Ķ˿ں�6000
	S0_DPort[1] = 0x70;

	S0_Mode=TCP_CLIENT;//���ض˿�0�Ĺ���ģʽ,TCP�ͻ���ģʽ
}

/*******************************************************************************
* ������  : CC2530_IO_config
* ����    : CC2530 SPI��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
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

    /* �����ⲿ�ж� */
    P0IEN |= 0x20;          // P0_5 ����Ϊ�жϷ�ʽ 1���ж�ʹ��
    PICTL |= 0x1;           //�½��ش���   
    P0IE = 1;               //����P0���ж�; 
    P0IFG = 0x00;           //��ʼ���жϱ�־λ
    EA = 1;                 //�����ж�
}

/*******************************************************************************
* ������  : W5100_Initialization
* ����    : W5100��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5100_Initialization()
{
    /* ��������IO�����ģʽ */
    CC2530_IO_config();

    /* װ��������� */
    Load_Net_Parameters();	

    /* Ӳ����λW5100 */
    W5100_Hardware_Reset();

    /* ��ʼ��W5100�Ĵ������� */
	W5100_Init();

    /* ������ط����� */
    Detect_Gateway(); 

    /* ָ��Socket(0~3)��ʼ��,��ʼ���˿�0 */
	Socket_Init(0);
}

/*******************************************************************************
* ������  : W5100_Socket_Start
* ����    : W5100�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void W5100_Socket_Start(void)
{
	if(S0_State==0)//�˿�0��ʼ������
	{
		if(S0_Mode==TCP_SERVER)//TCP������ģʽ 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP�ͻ���ģʽ 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDPģʽ 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/* ����W5100�ķ��ͽӿ� */
uint16 W5100_SocketSend(SOCKET s, unsigned char *pcBuff, unsigned int size)
{
    uint8 ucRet = FALSE;
    
    if ((NULL == pcBuff) || (30 < size))
    {
        return 0;
    }

    /* ���״̬ */   
    if (!(S0_State & S_CONN))
    {
        return 0;
    }

    /* �ȴ�ԭ���ݷ������ */
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

/* ����W5100�Ľ��սӿ� */
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

    /* ָ����ջ��� */
    *ppcBuff = Rx_Buffer;
    
    /* ���ؽ��յ������ݴ�С */
    return ucRet;
}


