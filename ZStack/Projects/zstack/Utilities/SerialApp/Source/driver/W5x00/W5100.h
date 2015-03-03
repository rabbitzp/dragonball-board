/**************************************************************************************************
  Filename:       W5100.h
  Revised:         
  Revision:       

  Description:    This file contains w5100 driver header file.
**************************************************************************************************/
#ifndef __W5100_H_
#define __W5100_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define S_RX_SIZE	2048	/*����Socket���ջ������Ĵ�С�����Ը���W5100_RMSR�������޸� */
#define S_TX_SIZE	2048  	/*����Socket���ͻ������Ĵ�С�����Ը���W5100_TMSR�������޸� */


/*-----------------------------------------------------------------------------*/
/*----------------- W5100 Register definition(W5100�Ĵ�������)-----------------*/
/*-----------------------------------------------------------------------------*/

#define	COMMON_BASE	0x0000		//Base address(������Ҫ�޸Ļ���ַ)

/*-----------------------------------------------------------------------------*/
/*-------------------- Common register(W5100����Ĵ�������)--------------------*/
/*-----------------------------------------------------------------------------*/
#define W5100_MODE	COMMON_BASE	//Mode register,R/W,Default=0x00(ģʽ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ������������λ��Ping�ر�ģʽ��PPPoEģʽ�Լ�������߽ӿ�)
	#define MODE_RST	0x80	//Software reset(�����λ,��1��Ч,��λ���Զ���0,оƬ�ڲ��Ĵ���������ʼ��Ϊȱʡֵ)
	#define MODE_MT		0x20	//Memory test
	#define MODE_PB		0x10	//Ping block mode(Ping��ֹģʽ,0�ر�Ping��ֹ,1����Ping��ֹ)
	#define MODE_PPPOE	0x08	//PPOE mode(PPPoEģʽ,0:�ر�PPPoEģʽ,1:��PPPoEģʽ,���������·����ֱ�����ӵ�ADSL,��λ�����á�1���Ա���ADSL���������ӡ���֪����,����ա�How to connect ADSL��Ӧ�ñʼ�)
	#define MODE_LB		0x04	//Little_endian/Big_endian ordering in indirect bus I/F
	#define MODE_AI		0x02	//Address auto increment in indirect bus I/F(������߽ӿ�ģʽ�µ�ַ�Զ�����,0����ֹ��ַ�Զ�����,1:������ַ�Զ�����,�ڼ�����߽ӿ�ģʽ��:����λ�á�1��ʱ:ÿ�ζ�/ д�������ַ�Զ�����1������ɲο���6.2������߿�ģʽ��)
	#define MODE_IND	0x01	//Indirect bus I/F mode(������߽ӿ�ģʽ,0:��ֹ������߽ӿ�ģʽ,1:����������߽ӿ�ģʽ,����λ�á�1��ʱ,���ü�����߽ӿ�ģʽ������ɲ��ա�6.Ӧ����Ϣ��,��6.2������߽ӿ�ģʽ��)

#define	W5100_GAR	COMMON_BASE+0x01	//Gateway address register,R/W,default=0x00(����IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������Ĭ�ϵ����ص�ַ,��:192.168.1.1)
#define	W5100_SUBR	COMMON_BASE+0x05	//Subnet mask address, R/W, default=0x00(��������Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������������������(Subnet Mask)ֵ,��:255.255.255.0)
#define	W5100_SHAR	COMMON_BASE+0x09	//Source hardware address, R/W, default=0x00(���������ַ(MAC)�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ñ��������ַ,��:00.08.DC.01.02.03->0x00,0x08,0xdc,0x01,0x02,0x03)
#define	W5100_SIPR	COMMON_BASE+0x0f	//Source IP address, R/W, default=0x00(����IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ñ���IP��ַ,��:192.168.0.3->0xc0,0xa8,0x00,0x03)

#define	W5100_IR	COMMON_BASE+0x15	//Interrupt and interrupt mask register, RO, Default=0x00(�жϼĴ���,ֻ��,ȱʡֵ0x00,CPUͨ�����ʸüĴ�����ò����жϵ���Դ,�κ��ж�Դ�����Ա��ж����μĴ�����IMR ���е�λ���Ρ����κ�һ��δ���ε��ж�λΪ��1��,/INT���źŽ����ֵ͵�ƽ��ֻ�е�����δ���ε��ж�λΪ0,/INT�Żָ��ߵ�ƽ��)
	#define IR_CONFLICT	0x80			//IP conflict(IP��ַ��ͻ,��һ���뱾��IP��ַ��ͬIP��ַ��ARP����ʱ,��λ���á�1��,�Ը�λд��1������0)
	#define IR_UNREACH	0x40			//Destination unreachable(�޷������ַ,��UDP���ݰ����͹�����,���Ŀ��IP��ַ������ʱ,W5100�����յ�һICMP(Ŀ���޷�����)���ݰ�,(����5.2.2.UDP ),�ڸ�״̬��,IP��ַ���˿ںŽ����浽UIPR��UPORT �Ĵ���,ͬʱ��λ�á�1��,�Ը�λд��1������0 )
	#define IR_PPPOE	0x20			//PPOE close(PPPoE���ӹر�,��PPPoEģʽ,���PPPoE���ӱ��ر�,��λ�á�1��,�Ը�λд��1������0)
	#define IR_S3_INT	0x08			//Occurrence of socket 3 socket interrupt(�˿�3�ж�,�˿�3�жϲ���ʱ,��λ���á�1����(��������ն˿��жϼĴ���S3_IR),��S3_IR��0,��λ�Զ���0)
	#define IR_S2_INT	0x04			//Occurrence of socket 2 socket interrupt(�˿�2�ж�,�˿�2�жϲ���ʱ,��λ���á�1����(��������ն˿��жϼĴ���S2_IR),��S2_IR��0,��λ�Զ���0)
	#define IR_S1_INT	0x02			//Occurrence of socket 1 socket interrupt(�˿�1�ж�,�˿�1�жϲ���ʱ,��λ���á�1����(��������ն˿��жϼĴ���S1_IR),��S1_IR��0,��λ�Զ���0)
	#define IR_S0_INT	0x01			//Occurrence of socket 0 socket interrupt(�˿�0�ж�,�˿�0�жϲ���ʱ,��λ���á�1����(��������ն˿��жϼĴ���S0_IR),��S0_IR��0,��λ�Զ���0)

#define	W5100_IMR	COMMON_BASE+0x16	//R/W, Default=0x00(�ж����μĴ���,�ɶ���д,ȱʡֵ0x00,�ж����μĴ���(IMR)���������ж�Դ,ÿ���ж�����λ��Ӧ�жϼĴ���(IR)�е�һ��λ������ж�����λ���á�1��ʱ,�κ�ʱ��ֻҪIR��Ӧ��λҲ�á�1��,�жϽ������������IMR������λ���塰0��,��ʹ��Ӧ��IR�ж�λ���á�1��,�ж�Ҳ�������)
	#define IMR_CONFLICT 0x80			//IP conflict(����IP��ͻ�����ж�)
	#define IMR_UNREACH	0x40			//Destination unreachable(�����ַ�޷���������ж�)
	#define IMR_PPPOE	0x20			//PPOE close(����PPPoE�رղ����ж�)
	#define IMR_S3_INT	0x08			//Occurrence of socket 3 socket interrupt(����˿�3�����ж�)
	#define IMR_S2_INT	0x04			//Occurrence of socket 2 socket interrupt(����˿�2�����ж�)
	#define IMR_S1_INT	0x02			//Occurrence of socket 1 socket interrupt(����˿�1�����ж�)
	#define IMR_S0_INT	0x01			//Occurrence of socket 0 socket interrupt(����˿�0�����ж�)

#define W5100_RTR	COMMON_BASE+0x17	//Retry time value. Value 1 means 100us, R/W, default=0x07D0(�ط�ʱ��Ĵ���,�üĴ����������������ʱ��ֵ,ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����,������CONNECT��DISCON��CLOSE��SEND��SEND_MAC��SEND_KEEP�������û���յ�Զ�̶Զ˵���Ӧ������Ӧ��ʱ,���ᵼ���ط�����)
#define W5100_RCR	COMMON_BASE+0X19	//Retry count, R/W, Default=0x08(�ط������Ĵ���,�üĴ����ڵ���ֵ�趨���ط��Ĵ���������ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��))

//--------------------------------------------------------    S1      S0      Memory size
//  Socket3    |  Socket2    |  Socket1    |  Socket0    |    0       0           1KB
//-------------|-------------|-------------|-------------|    0       1           2KB
//S1       S0  |S1       S0  |S1       S0  |S1       S0  |    1       0           4KB
//--------------------------------------------------------    1       1           8KB
#define W5100_RMSR	COMMON_BASE+0x1a	//RX memory size register, R/W, default=0x55(���մ洢�����üĴ���,�ɶ���д,ȱʡֵ0x55,�˼Ĵ�������ȫ��8K��RX�洢�ռ䵽��ָ���˿�,��8K��Χ��,�Ӷ˿�0��ʼ,��S0��S1ȷ��4���˿ڵĽ��ջ������Ĵ�С,����ж˿ڷ��䲻���洢�ռ�,�ö˿ھͲ���ʹ��,��ʼ��ֵΪ0x55,4���˿ڷֱ����2K�Ĵ洢�ռ�)
#define W5100_TMSR	COMMON_BASE+0x1b	//TX memory size register, R/W, default=0x55(���ʹ洢���üĴ���,�ɶ���д,ȱʡֵ0x55,�üĴ���������8K�ķ��ʹ洢�������ÿ���˿ڡ����ʹ洢�������÷�������մ洢����������ȫһ������ʼ��ֵΪ0x55����ÿ�˿ڷֱ����2KB �Ŀռ�)
#define W5100_PATR	COMMON_BASE+0x1c	//Authentication type in PPPOE mode, R, default=0x0000(PPPoEģʽ�µ���֤����,ֻ��,ȱʡֵ0x0000,����PPPoE����������ʱ,�üĴ���ָʾ�Ѿ���ͨ���İ�ȫ��֤������W5100ֻ֧�����ְ�ȫ��֤����:PAP(0xC023)��CHAP(0xC223))
#define W5100_PTIMER	COMMON_BASE+0x28//PPP LCP request timer register, R/W, default=0x28, Value 1 is about 25ms(PPP LCP�����ʱ�Ĵ���,�ɶ���д,ȱʡֵ0x28,�üĴ�����ʾ����LCP Echo(��Ӧ����)����Ҫ��ʱ����,ÿ1��λ��Լ25����) 
#define W5100_PMAGIC	COMMON_BASE+0x29//PPP LCP magic number register, R/W, default=0x00(PPP LCPħ���Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������LCP����ʱ���õ�ħ��ѡ����ա�How to connect ADSL��Ӧ�ñʼ�)
#define W5100_UIPR	COMMON_BASE+0x2a	//Unreachable IP address, RO, default=0x00(�޷������IP ��ַ�Ĵ���,ֻ��,ȱʡֵ0x00,��UDP���ݴ���ʱ(����5.2.2. UDP),���Ŀ��IP��ַ������,�����յ�һ��ICMP(��ַ�޷�����)���ݰ��������������,�޷������IP��ַ���˿ںŽ��ֱ�洢��UIPR��UPORT��)
#define W5100_UPORT	COMMON_BASE+0x2e	//Unreachable port register, RO, default=0x0000(�޷�����Ķ˿ںżĴ���,ֻ��,ȱʡֵ0x0000)

/*-----------------------------------------------------------------------------*/
/*-------------------- Socket register(W5100�˿ڼĴ�������)--------------------*/
/*-----------------------------------------------------------------------------*/
#define W5100_S0_MR	COMMON_BASE+0x0400	//Socket 0 mode register, R/W, default=0x00(�˿�0ģʽ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ���������Ӧ�˿ڵ�ѡ���Э������)
#define W5100_S1_MR	COMMON_BASE+0x0500	//Socket 1 mode register, R/W, default=0x00(�˿�1ģʽ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ���������Ӧ�˿ڵ�ѡ���Э������)
#define W5100_S2_MR	COMMON_BASE+0x0600	//Socket 2 mode register, R/W, default=0x00(�˿�2ģʽ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ���������Ӧ�˿ڵ�ѡ���Э������)
#define W5100_S3_MR	COMMON_BASE+0x0700	//Socket 3 mode register, R/W, default=0x00(�˿�3ģʽ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ���������Ӧ�˿ڵ�ѡ���Э������)
	#define S_MR_MULTI	0x80	//Multcasting(�㲥����,0:�رչ㲥����,1:������㲥����,�㲥����ֻ����UDPģʽ��ʹ�á�ʹ�ù㲥ʱ,������OPEN����ǰ,���㲥���ַ�Ͷ˿ں�д�뵽�˿�n��Ŀ��IP��ַ�Ĵ�����Ŀ�Ķ˿ںżĴ���(nΪ0~3))
	#define S_MR_MC		0x20	//Multcast(ʹ������ʱ��Ӧ,0:��ֹ����ʱ��Ӧѡ��,1:��������ʱ��Ӧѡ��,�ù���ֻ����TCPģʽ����Ч��Ϊ��1��ʱ������ʲôʱ��ӶԶ��յ����ݣ����ᷢ��һ��ACK��Ӧ��Ϊ��0��ʱ���������ڲ���ʱ���Ʒ���ACK��Ӧ)
								//  P3   P2   P1   P0     Meaning
	#define S_MR_CLOSED	0x00	//  0    0    0    0      Closed(�ر�)
	#define S_MR_TCP	0x01	//  0    0    0    1      TCP(TCP)
	#define S_MR_UDP	0x02	//  0    0    1    0      UDP(UDP)
	#define S_MR_IPRAW	0x03	//  0    0    1    1      IPRAW(IPRAW)
	#define S_MR_MACRAW	0x04	//  0    1    0    0      MACRAW(�˿�0��Э������������MACRAW)
	#define S_MR_PPPOE	0x05	//  0    1    0    1      PPPOE(�˿�0��Э������������PPPOE)

#define W5100_S0_CR	COMMON_BASE+0x0401	//Socket 0 command register, R/W, default=0x00(�˿�0����Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ö˿ڵĳ�ʼ�����رա��������ӡ��Ͽ����ӡ����ݴ����Լ�������յ�,����ִ�к�,�Ĵ�����ֵ�Զ���0x00)
#define W5100_S1_CR	COMMON_BASE+0x0501	//Socket 1 command register, R/W, default=0x00(�˿�1����Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ö˿ڵĳ�ʼ�����رա��������ӡ��Ͽ����ӡ����ݴ����Լ�������յ�,����ִ�к�,�Ĵ�����ֵ�Զ���0x00)
#define W5100_S2_CR	COMMON_BASE+0x0601	//Socket 2 command register, R/W, default=0x00(�˿�2����Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ö˿ڵĳ�ʼ�����رա��������ӡ��Ͽ����ӡ����ݴ����Լ�������յ�,����ִ�к�,�Ĵ�����ֵ�Զ���0x00)
#define W5100_S3_CR	COMMON_BASE+0x0701	//Socket 3 command register, R/W, default=0x00(�˿�3����Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ����������ö˿ڵĳ�ʼ�����رա��������ӡ��Ͽ����ӡ����ݴ����Լ�������յ�,����ִ�к�,�Ĵ�����ֵ�Զ���0x00)
	#define S_CR_OPEN	0x01	//It is used to initialize the socket(���ڳ�ʼ���˿�,���ݶ˿�nģʽ�Ĵ���(Sn_MR) ������,�˿�n��״̬�Ĵ���(Sn_SSR)��ֵ���ı�ΪSOCK_INIT��SOCK_UDP��SOCK_IPRAW ��SOCK_MACRAW ,������ա�5.����������)
	#define S_CR_LISTEN	0x02	//In TCP mode, it waits for a connection request from any remote peer(Client)(ֻ��TCPģʽ��Ч,�����˿�n��״̬�Ĵ���(Sn_SSR)�ı�ΪSOCK_LISTEN,�Ա�ȴ�Զ�̶Զ˷��͵���������������ա�5.2.1.1 ������ģʽ��)
	#define S_CR_CONNECT 0x04	//In TCP mode, it sends a connection request to remote peer(SERVER)(ֻ��TCPģʽ��Ч,������һ����������Զ�̶Զ˷�������������ʧ��,��������ʱ�жϡ�������ա�5.2.1.2�ͻ���ģʽ)
	#define S_CR_DISCON	0x08	//In TCP mode, it sends a connection termination request(ֻ��TCPģʽʱ��Ч,��������ֹ����������������ֹʧ��,������ʱ����ʱ�жϡ�������ա�5.2.1.1 ������ģʽ��,�����CLOSE�������DISCON����,ֻ�ж˿�n��״̬�Ĵ���(Sn_SSR)�ı��SOCK_CLOSED,����������ֹ���ӹ���)
	#define S_CR_CLOSE	0x10	//Used to close the socket(���������ڹرն˿ڡ����ı�˿�n��״̬�Ĵ���(Sn_SSR)ΪSOCK_CLOSED)
	#define S_CR_SEND	0x20	//It transmit the data as much as the increase size of write pointer(���ն˿�n��TXдָ�����ӵĴ�С�������ݡ�����ο��˿�n����ʣ��ռ�Ĵ���(Sn_TX_FSR)���˿�n����дָ��Ĵ���(Sn_TX_WR)���˿�n���Ͷ�ָ��Ĵ���(Sn_TX_RR),��5.2.1.1 ������ģʽ��)
	#define S_CR_SEND_MAC  0x21	//In UDP mode, same as SEND(����UDPģʽ��Ч,����������SEND��ͬ��ͨ��SEND����������Ҫ�õ���ARP������Ŀ��Ӳ����ַ����SEND_MAC����ʱʹ���û����õ�Ŀ�������ַ(Sn_DHAR),��û��ARP����)
	#define S_CR_SEND_KEEP 0x22	//In TCP mode(ֻ��TCPģʽ��Ч,��ͨ������1���ֽڵ����ݼ��˿ڵ�����״̬����������Ѿ���ֹ��Զ�û����Ӧ����������ʱ�ж�)
	#define S_CR_RECV	0x40	//Receiving is processed including the value of socket RX read pointer register(���ն˿�n�Ķ�ָ��Ĵ���(Sn_RX_RR)�е����ݽ��մ�����ɡ���������ն˿�n�������ݴ�С�Ĵ���(Sn_RX_RSR)���˿�n����дָ��Ĵ���(Sn_RX_WR)���˿�n���ն�ָ��Ĵ���(Sn_RX_RR))

	/* Definition for PPPOE */
	#define S_CR_PCON	0x23	//Start of ADSL connection
	#define S_CR_PDISCON 0x24	//End of ADSL connection
	#define S_CR_PCR	0x25	//Send REQ message ineach phase
	#define S_CR_PCN	0x26	//Send NAK message in each phase
	#define S_CR_PCJ	0x27	//Senf REJECT message in each phase

#define W5100_S0_IR	COMMON_BASE+0x0402	//Socket 0 interrup register, RO, default=0x00(�˿�0�жϼĴ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ��������ֹ���ӡ��������ݡ���������Լ�ʱ���������Ϣ���Ĵ�������Ӧ��λ���á�1�������д�롱1������0)
#define W5100_S1_IR	COMMON_BASE+0x0502	//Socket 1 interrup register, RO, default=0x00(�˿�1�жϼĴ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ��������ֹ���ӡ��������ݡ���������Լ�ʱ���������Ϣ���Ĵ�������Ӧ��λ���á�1�������д�롱1������0)
#define W5100_S2_IR	COMMON_BASE+0x0602	//Socket 2 interrup register, RO, default=0x00(�˿�2�жϼĴ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ��������ֹ���ӡ��������ݡ���������Լ�ʱ���������Ϣ���Ĵ�������Ӧ��λ���á�1�������д�롱1������0)
#define W5100_S3_IR	COMMON_BASE+0x0702	//Socket 3 interrup register, RO, default=0x00(�˿�3�жϼĴ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ��������ֹ���ӡ��������ݡ���������Լ�ʱ���������Ϣ���Ĵ�������Ӧ��λ���á�1�������д�롱1������0)
	#define S_IR_SENDOK		0x10	//Send data complete(���ݷ��Ͳ�����ɸ�λ���á�1��)
	#define S_IR_TIMEOUT	0x08	//Set if timeout occurs during connection or termination or data transmission(�����ӻ���ֹ�����ݷ��͵ȹ����г�ʱ����λ���á�1��)
	#define S_IR_RECV		0x04	//Set if data is received(���˿ڽ��յ�����ʱ�á�1������ִ��CMD_RECV �����������Ȼ��������λҲΪ��1��)
	#define S_IR_DISCON		0x02	//Set if receiv connection termination request(���յ���ֹ�����������ֹ���ӹ����ѽ���ʱ����λ���á�1��)
	#define S_IR_CON		0x01	//Set if connetion is established(�����ӳɹ�ʱ,��λ���á�1��)

	/* Definition for PPPOE */
	#define S_IR_PRECV	0x80		//Indicate receiving no support option data
	#define S_IR_PFAIL	0x40		//Indicate PAP Authentication fail
	#define S_IR_PNEXT	0x20		//Go next phase

#define W5100_S0_SSR	COMMON_BASE+0x0403	//Socket 0 status register, RO, default=0x00(�˿�0״̬�Ĵ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ�˿�0��״̬��ֵ)
#define W5100_S1_SSR	COMMON_BASE+0x0503	//Socket 1 status register, RO, default=0x00(�˿�1״̬�Ĵ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ�˿�1��״̬��ֵ)
#define W5100_S2_SSR	COMMON_BASE+0x0603	//Socket 2 status register, RO, default=0x00(�˿�2״̬�Ĵ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ�˿�2��״̬��ֵ)
#define W5100_S3_SSR	COMMON_BASE+0x0703	//Socket 3 status register, RO, default=0x00(�˿�3״̬�Ĵ���,ֻ��,ȱʡֵ0x00,�üĴ���ָʾ�˿�3��״̬��ֵ)
	#define S_SSR_CLOSED	0x00	//In case that OPEN command are given to Sn_CR, Timeout interrupt is asserted or connection is terminated(����Sn_CRд��CLOSE���������ʱ�жϻ����ӱ���ֹ,������SOCK_CLOSED״̬����SOCK_CLOSED ״̬�������κβ���,�ͷ�����������Դ)
	#define S_SSR_INIT		0x13	//In case that Sn_MR is set as TCP and OPEN commands are given to Sn_CR(��Sn_MR��ΪTCPģʽʱ,��Sn_CR��д��OPEN����ʱ������SOCK_INIT״̬�����Ƕ˿ڽ���TCP���ӵĿ�ʼ����SOCK_INIT״̬,Sn_CR��������;����˲������͡�TCP������ģʽ��TCP�ͻ���ģʽ)
	#define S_SSR_LISTEN	0x14	//In case that under the SOCK_INIT status, LISTEN commands are given to Sn_CR(���˿ڴ���SOCK_INIT״̬ʱ,��Sn_CRд��LISTEN����ʱ������SOCK_LISTEN״̬����Ӧ�Ķ˿�����ΪTCP������ģʽ,����յ��������󽫸ı�ΪESTABLISHED״̬)
	#define S_SSR_ESTABLISHED 0x17	//In case that connection is established(���˿ڽ�������ʱ������SOCK_ESTABLISHED״̬,������״̬�¿��Է��ͺͽ���TCP����)
	#define S_SSR_CLOSE_WAIT  0x1c	//In case that connection temination request is received(���յ����ԶԶ˵���ֹ��������ʱ,������SOCK_CLOSE_WAIT״̬��������״̬,�ӶԶ��յ���Ӧ��Ϣ,��û�жϿ������յ�DICON��CLOSE����ʱ,���ӶϿ�)
	#define S_SSR_UDP		0x22	//In case that OPEN command is given to Sn_CR when Sn_MR is set as UDP(��Sn_MR��ΪUDPģʽ,��Sn_CRд��OPEN����ʱ������SOCK_UDP״̬,������״̬��ͨ�Ų���Ҫ��Զ˽�������,���ݿ���ֱ�ӷ��ͺͽ���)
	#define S_SSR_IPRAW		0x32	//In case that OPEN command is given to Sn_CR when Sn_MR is set as IPRAW(��Sn_MR��ΪIPRAWģʽ,��Sn_CRд��OPEN����ʱ������SOCK_IPRAW״̬,��IP RAW״̬,IP�����ϵ�Э�齫������������ο���IPRAW��)
	#define S_SSR_MACRAW	0x42	//In case that OPEN command is given to S0_CR when S0_MR is set as MACRAW(��Sn_MR��ΪMACRAWģʽ,��S0_CRд��OPEN����ʱ������SOCK_MACRAW״̬����MAC RAW״̬,����Э�����ݰ���������,������ο���MAC RAW��)
	#define S_SSR_PPPOE		0x5f	//In case that OPEN command is given to S0_CR when S0_MR is set as PPPOE(��Sn_MR��ΪPPPoEģʽ,��S0_CRд��OPEN����ʱ������SOCK_PPPOE״̬)
	//Below is shown in the status change, and does not need much attention
	#define S_SSR_SYNSEND	0x15	//(�˿�n��SOCK_INIT״̬,��Sn_CRд��CONNECT����ʱ������SOCK_SYNSENT״̬��������ӳɹ�,���Զ��ı�ΪSOCK_ESTABLISH)
	#define S_SSR_SYNRECV	0x16	//(�����յ�Զ�̶Զ�(�ͻ���)���͵���������ʱ,������SOCK_SYNRECV״̬����Ӧ�����,״̬�ı�ΪSOCK_ESTABLISH)
	#define S_SSR_FIN_WAIT	0x18	//(0x18,0x1a,0x1b,0x1b:��������ֹ�����н�������Щ״̬�����������ֹ�����,�������ʱ������ж�,״̬���Զ����ı�ΪSOCK_CLOSED)
	#define S_SSR_CLOSING	0x1a
	#define S_SSR_TIME_WAIT	0x1b
	#define S_SSR_LAST_ACK	0x1d
	#define S_SSR_ARP0		0x11	//(0x11,0x21,0x31:��TCPģʽ�·��������������UDPģʽ�·�������ʱ,������ARP�����Ի��Զ�̶Զ˵������ַʱ,������SOCK_ARP״̬������յ�ARP��Ӧ,������SOCK_SYNSENT��SOCK_UDP ��SOCK_ICMP״̬,�Ա�����Ĳ���)
	#define S_SSR_ARP1		0x21
	#define S_SSR_ARP2		0x31

#define W5100_S0_PORT	COMMON_BASE+0x0404		//Socket 0 Source port register, R/W, default=0x00(�˿�0�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����TCP��UDPģʽ���趨��Ӧ�˿ڵĶ˿ںš���Щ�˿ںű����ڽ���OPENָ��֮ǰ���)
#define W5100_S1_PORT	COMMON_BASE+0x0504		//Socket 1 Source port register, R/W, default=0x00(�˿�1�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����TCP��UDPģʽ���趨��Ӧ�˿ڵĶ˿ںš���Щ�˿ںű����ڽ���OPENָ��֮ǰ���)
#define W5100_S2_PORT	COMMON_BASE+0x0604		//Socket 2 Source port register, R/W, default=0x00(�˿�2�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����TCP��UDPģʽ���趨��Ӧ�˿ڵĶ˿ںš���Щ�˿ںű����ڽ���OPENָ��֮ǰ���)
#define W5100_S3_PORT	COMMON_BASE+0x0704		//Socket 3 Source port register, R/W, default=0x00(�˿�3�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����TCP��UDPģʽ���趨��Ӧ�˿ڵĶ˿ںš���Щ�˿ںű����ڽ���OPENָ��֮ǰ���)
 
#define W5100_S0_DHAR	COMMON_BASE+0x0406		//Socket 0 destination hardware address register, R/W, default=0x00(�˿�0��Ŀ�������ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������ÿ���˿ڵ�Ŀ�������ַ)
#define W5100_S1_DHAR	COMMON_BASE+0x0506		//Socket 1 destination hardware address register, R/W, default=0x00(�˿�1��Ŀ�������ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������ÿ���˿ڵ�Ŀ�������ַ)
#define W5100_S2_DHAR	COMMON_BASE+0x0606		//Socket 2 destination hardware address register, R/W, default=0x00(�˿�2��Ŀ�������ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������ÿ���˿ڵ�Ŀ�������ַ)
#define W5100_S3_DHAR	COMMON_BASE+0x0706		//Socket 3 destination hardware address register, R/W, default=0x00(�˿�3��Ŀ�������ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,�üĴ�������ÿ���˿ڵ�Ŀ�������ַ)

#define W5100_S0_DIPR	COMMON_BASE+0x040c		//Socket 0 destination IP address register, R/W, default=0x00(�˿�0��Ŀ��IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�Ŀ��IP��ַ��������ģʽ(�ͻ���ģʽ),Ŀ��IP��ַ��������Ŀ��IP��ַ,��UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ��û���յ��Զ˵�ARP ��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S1_DIPR	COMMON_BASE+0x050c		//Socket 1 destination IP address register, R/W, default=0x00(�˿�1��Ŀ��IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�Ŀ��IP��ַ��������ģʽ(�ͻ���ģʽ),Ŀ��IP��ַ��������Ŀ��IP��ַ,��UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ��û���յ��Զ˵�ARP ��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S2_DIPR	COMMON_BASE+0x060c		//Socket 2 destination IP address register, R/W, default=0x00(�˿�2��Ŀ��IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�Ŀ��IP��ַ��������ģʽ(�ͻ���ģʽ),Ŀ��IP��ַ��������Ŀ��IP��ַ,��UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ��û���յ��Զ˵�ARP ��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S3_DIPR	COMMON_BASE+0x070c		//Socket 3 destination IP address register, R/W, default=0x00(�˿�3��Ŀ��IP��ַ�Ĵ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�Ŀ��IP��ַ��������ģʽ(�ͻ���ģʽ),Ŀ��IP��ַ��������Ŀ��IP��ַ,��UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ��û���յ��Զ˵�ARP ��Ӧ֮ǰ,�üĴ�����λ)

#define W5100_S0_DPORT	COMMON_BASE+0x0410		//Socket 0 destionation port register, R/W, default=0x00(�˿�0��Ŀ�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,��TCP ģʽ,�üĴ������ö˿ڵ�Ŀ�Ķ˿ںš�������ģʽ��(�ͻ���ģʽ),Ŀ�Ķ˿ںű������ú���ִ������(CONNECT)����ڱ���ģʽ��(������ģʽ),W5100�������Ӻ�,�ڲ��Զ�ˢ��Ŀ�Ķ˿ں�,�� UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ����û���յ��Զ˵�ARP��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S1_DPORT	COMMON_BASE+0x0510		//Socket 1 destionation port register, R/W, default=0x00(�˿�1��Ŀ�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,��TCP ģʽ,�üĴ������ö˿ڵ�Ŀ�Ķ˿ںš�������ģʽ��(�ͻ���ģʽ),Ŀ�Ķ˿ںű������ú���ִ������(CONNECT)����ڱ���ģʽ��(������ģʽ),W5100�������Ӻ�,�ڲ��Զ�ˢ��Ŀ�Ķ˿ں�,�� UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ����û���յ��Զ˵�ARP��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S2_DPORT	COMMON_BASE+0x0610		//Socket 2 destionation port register, R/W, default=0x00(�˿�2��Ŀ�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,��TCP ģʽ,�üĴ������ö˿ڵ�Ŀ�Ķ˿ںš�������ģʽ��(�ͻ���ģʽ),Ŀ�Ķ˿ںű������ú���ִ������(CONNECT)����ڱ���ģʽ��(������ģʽ),W5100�������Ӻ�,�ڲ��Զ�ˢ��Ŀ�Ķ˿ں�,�� UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ����û���յ��Զ˵�ARP��Ӧ֮ǰ,�üĴ�����λ)
#define W5100_S3_DPORT	COMMON_BASE+0x0710		//Socket 3 destionation port register, R/W, default=0x00(�˿�3��Ŀ�Ķ˿ںżĴ���,�ɶ���д,ȱʡֵ0x00,��TCP ģʽ,�üĴ������ö˿ڵ�Ŀ�Ķ˿ںš�������ģʽ��(�ͻ���ģʽ),Ŀ�Ķ˿ںű������ú���ִ������(CONNECT)����ڱ���ģʽ��(������ģʽ),W5100�������Ӻ�,�ڲ��Զ�ˢ��Ŀ�Ķ˿ں�,�� UDPģʽ,�յ��Զ˵�ARP��Ӧ��,�üĴ�����ȷ��Ϊ�û�д���ֵ����û���յ��Զ˵�ARP��Ӧ֮ǰ,�üĴ�����λ)

#define W5100_S0_MSS	COMMON_BASE+0x0412		//Socket 0 maximum segment size register, R/W, default=0x00(�˿�0����Ƭ���ȼĴ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�����Ƭ����,���TCP�ڱ���ģʽ(Passive Mode)��,�üĴ�����ֵ��������������)
#define W5100_S1_MSS	COMMON_BASE+0x0512		//Socket 1 maximum segment size register, R/W, default=0x00(�˿�1����Ƭ���ȼĴ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�����Ƭ����,���TCP�ڱ���ģʽ(Passive Mode)��,�üĴ�����ֵ��������������)
#define W5100_S2_MSS	COMMON_BASE+0x0612		//Socket 2 maximum segment size register, R/W, default=0x00(�˿�2����Ƭ���ȼĴ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�����Ƭ����,���TCP�ڱ���ģʽ(Passive Mode)��,�üĴ�����ֵ��������������)
#define W5100_S3_MSS	COMMON_BASE+0x0712		//Socket 3 maximum segment size register, R/W, default=0x00(�˿�3����Ƭ���ȼĴ���,�ɶ���д,ȱʡֵ0x00,��TCPģʽ,�üĴ������ö˿ڵ�����Ƭ����,���TCP�ڱ���ģʽ(Passive Mode)��,�üĴ�����ֵ��������������)

#define W5100_S0_PROTO	COMMON_BASE+0x0414		//Socket 0 IP protocol register, R/W, default=0x00(�˿�0��IPЭ��Ĵ���,�ɶ���д,ȱʡֵ0x00,��IP RAWģʽ��,IPЭ��Ĵ�����������IP���ݰ�ͷ��Э���ֶ�(Protocol Field)��ֵ��IANAԤ��ע����һЩЭ��ſ���ʹ�á�IANA��վ�ɲ��ȫ����IP��Э����롣�ο�IANA�������ĵ�://www.iana.org/assignments/protocol-number)
#define W5100_S1_PROTO	COMMON_BASE+0x0514		//Socket 1 IP protocol register, R/W, default=0x00(�˿�1��IPЭ��Ĵ���,�ɶ���д,ȱʡֵ0x00,��IP RAWģʽ��,IPЭ��Ĵ�����������IP���ݰ�ͷ��Э���ֶ�(Protocol Field)��ֵ��IANAԤ��ע����һЩЭ��ſ���ʹ�á�IANA��վ�ɲ��ȫ����IP��Э����롣�ο�IANA�������ĵ�://www.iana.org/assignments/protocol-number)
#define W5100_S2_PROTO	COMMON_BASE+0x0614		//Socket 2 IP protocol register, R/W, default=0x00(�˿�2��IPЭ��Ĵ���,�ɶ���д,ȱʡֵ0x00,��IP RAWģʽ��,IPЭ��Ĵ�����������IP���ݰ�ͷ��Э���ֶ�(Protocol Field)��ֵ��IANAԤ��ע����һЩЭ��ſ���ʹ�á�IANA��վ�ɲ��ȫ����IP��Э����롣�ο�IANA�������ĵ�://www.iana.org/assignments/protocol-number)
#define W5100_S3_PROTO	COMMON_BASE+0x0714		//Socket 3 IP protocol register, R/W, default=0x00(�˿�3��IPЭ��Ĵ���,�ɶ���д,ȱʡֵ0x00,��IP RAWģʽ��,IPЭ��Ĵ�����������IP���ݰ�ͷ��Э���ֶ�(Protocol Field)��ֵ��IANAԤ��ע����һЩЭ��ſ���ʹ�á�IANA��վ�ɲ��ȫ����IP��Э����롣�ο�IANA�������ĵ�://www.iana.org/assignments/protocol-number)

#define W5100_S0_TOS	COMMON_BASE+0x0415		//Socket 0 IP type of servce register, R/W, default=0x00(�˿�0��IP�������ͼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�еķ�������(TOS)�ֶε�ֵ)
#define W5100_S1_TOS	COMMON_BASE+0x0515		//Socket 1 IP type of servce register, R/W, default=0x00(�˿�1��IP�������ͼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�еķ�������(TOS)�ֶε�ֵ)
#define W5100_S2_TOS	COMMON_BASE+0x0615		//Socket 2 IP type of servce register, R/W, default=0x00(�˿�2��IP�������ͼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�еķ�������(TOS)�ֶε�ֵ)
#define W5100_S3_TOS	COMMON_BASE+0x0715		//Socket 3 IP type of servce register, R/W, default=0x00(�˿�3��IP�������ͼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�еķ�������(TOS)�ֶε�ֵ)

#define W5100_S0_TTL	COMMON_BASE+0x0416		//Socket 0 IP time to live register, R/W, default=0x80(�˿�0��IP���ݰ������ڼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�е�������(TTL)�ֶε�ֵ)
#define W5100_S1_TTL	COMMON_BASE+0x0516		//Socket 1 IP time to live register, R/W, default=0x80(�˿�1��IP���ݰ������ڼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�е�������(TTL)�ֶε�ֵ)
#define W5100_S2_TTL	COMMON_BASE+0x0616		//Socket 2 IP time to live register, R/W, default=0x80(�˿�2��IP���ݰ������ڼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�е�������(TTL)�ֶε�ֵ)
#define W5100_S3_TTL	COMMON_BASE+0x0716		//Socket 3 IP time to live register, R/W, default=0x80(�˿�3��IP���ݰ������ڼĴ���,�ɶ���д,ȱʡֵ0x00,�üĴ�����������IP���ݰ�ͷ�е�������(TTL)�ֶε�ֵ)

#define W5100_S0_TX_FSR		COMMON_BASE+0x0420		//Socket 0 TX free size register, RO, default=0x0800(�˿�0���ʹ洢��ʣ��ռ�Ĵ���,ֻ��,ȱʡֵ0x0800,�üĴ���ָʾ�û���ʹ�õķ������ݿռ�Ĵ�С���ڷ�������ʱ,�û������ȼ��ʣ��ռ�Ĵ�С,Ȼ����Ʒ������ݵ��ֽ��������üĴ���ʱ,�����ȶ����ֽ�(0x0420,0x0520,0x0620,0x0720),Ȼ���ٶ����ֽ�(0x0421,0x0521,0x0621,0x0721),�˿ڷ����ܿռ�Ĵ�С�ɷ��ʹ洢���ռ�Ĵ���(TMSR)ȷ���������ݷ��ʹ��������,ʣ��ռ�Ĵ�С����д�����ݶ�����,������ɺ��Զ�����)
#define W5100_S1_TX_FSR		COMMON_BASE+0x0520		//Socket 1 TX free size register, RO, default=0x0800(�˿�1���ʹ洢��ʣ��ռ�Ĵ���,ֻ��,ȱʡֵ0x0800,�üĴ���ָʾ�û���ʹ�õķ������ݿռ�Ĵ�С���ڷ�������ʱ,�û������ȼ��ʣ��ռ�Ĵ�С,Ȼ����Ʒ������ݵ��ֽ��������üĴ���ʱ,�����ȶ����ֽ�(0x0420,0x0520,0x0620,0x0720),Ȼ���ٶ����ֽ�(0x0421,0x0521,0x0621,0x0721),�˿ڷ����ܿռ�Ĵ�С�ɷ��ʹ洢���ռ�Ĵ���(TMSR)ȷ���������ݷ��ʹ��������,ʣ��ռ�Ĵ�С����д�����ݶ�����,������ɺ��Զ�����)
#define W5100_S2_TX_FSR		COMMON_BASE+0x0620		//Socket 2 TX free size register, RO, default=0x0800(�˿�2���ʹ洢��ʣ��ռ�Ĵ���,ֻ��,ȱʡֵ0x0800,�üĴ���ָʾ�û���ʹ�õķ������ݿռ�Ĵ�С���ڷ�������ʱ,�û������ȼ��ʣ��ռ�Ĵ�С,Ȼ����Ʒ������ݵ��ֽ��������üĴ���ʱ,�����ȶ����ֽ�(0x0420,0x0520,0x0620,0x0720),Ȼ���ٶ����ֽ�(0x0421,0x0521,0x0621,0x0721),�˿ڷ����ܿռ�Ĵ�С�ɷ��ʹ洢���ռ�Ĵ���(TMSR)ȷ���������ݷ��ʹ��������,ʣ��ռ�Ĵ�С����д�����ݶ�����,������ɺ��Զ�����)
#define W5100_S3_TX_FSR		COMMON_BASE+0x0720		//Socket 3 TX free size register, RO, default=0x0800(�˿�3���ʹ洢��ʣ��ռ�Ĵ���,ֻ��,ȱʡֵ0x0800,�üĴ���ָʾ�û���ʹ�õķ������ݿռ�Ĵ�С���ڷ�������ʱ,�û������ȼ��ʣ��ռ�Ĵ�С,Ȼ����Ʒ������ݵ��ֽ��������üĴ���ʱ,�����ȶ����ֽ�(0x0420,0x0520,0x0620,0x0720),Ȼ���ٶ����ֽ�(0x0421,0x0521,0x0621,0x0721),�˿ڷ����ܿռ�Ĵ�С�ɷ��ʹ洢���ռ�Ĵ���(TMSR)ȷ���������ݷ��ʹ��������,ʣ��ռ�Ĵ�С����д�����ݶ�����,������ɺ��Զ�����)

#define W5100_S0_TX_RR		COMMON_BASE+0x0422		//Socket 0 TX read pointer register, RO, default=0x0000(�˿�0���ʹ洢����ָ��Ĵ���,ֻ��,ȱʡֵ0x0000,�üĴ���ָʾ�˿��ڷ��͹�����ɺ��ʹ洢���ĵ�ǰλ�á����˿�n������Ĵ����յ�SEND����,�ӵ�ǰSn_TX_RR��Sn_TX_WR�����ݽ����ͳ�ȥ,������ɺ�,Sn_TX_RR��ֵ�Զ��ı䡣��˷�����ɺ�,Sn_TX_RR��ֵ��Sn_TX_WR��ֵ��ȡ��û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0422��0x0522��0x0622��0x0722),Ȼ���ٶ����ֽ�(0x0423��0x0523��0x0623��0x0723))
#define W5100_S1_TX_RR		COMMON_BASE+0x0522		//Socket 1 TX read pointer register, RO, default=0x0000(�˿�1���ʹ洢����ָ��Ĵ���,ֻ��,ȱʡֵ0x0000,�üĴ���ָʾ�˿��ڷ��͹�����ɺ��ʹ洢���ĵ�ǰλ�á����˿�n������Ĵ����յ�SEND����,�ӵ�ǰSn_TX_RR��Sn_TX_WR�����ݽ����ͳ�ȥ,������ɺ�,Sn_TX_RR��ֵ�Զ��ı䡣��˷�����ɺ�,Sn_TX_RR��ֵ��Sn_TX_WR��ֵ��ȡ��û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0422��0x0522��0x0622��0x0722),Ȼ���ٶ����ֽ�(0x0423��0x0523��0x0623��0x0723))
#define W5100_S2_TX_RR		COMMON_BASE+0x0622		//Socket 2 TX read pointer register, RO, default=0x0000(�˿�2���ʹ洢����ָ��Ĵ���,ֻ��,ȱʡֵ0x0000,�üĴ���ָʾ�˿��ڷ��͹�����ɺ��ʹ洢���ĵ�ǰλ�á����˿�n������Ĵ����յ�SEND����,�ӵ�ǰSn_TX_RR��Sn_TX_WR�����ݽ����ͳ�ȥ,������ɺ�,Sn_TX_RR��ֵ�Զ��ı䡣��˷�����ɺ�,Sn_TX_RR��ֵ��Sn_TX_WR��ֵ��ȡ��û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0422��0x0522��0x0622��0x0722),Ȼ���ٶ����ֽ�(0x0423��0x0523��0x0623��0x0723))
#define W5100_S3_TX_RR		COMMON_BASE+0x0722		//Socket 3 TX read pointer register, RO, default=0x0000(�˿�3���ʹ洢����ָ��Ĵ���,ֻ��,ȱʡֵ0x0000,�üĴ���ָʾ�˿��ڷ��͹�����ɺ��ʹ洢���ĵ�ǰλ�á����˿�n������Ĵ����յ�SEND����,�ӵ�ǰSn_TX_RR��Sn_TX_WR�����ݽ����ͳ�ȥ,������ɺ�,Sn_TX_RR��ֵ�Զ��ı䡣��˷�����ɺ�,Sn_TX_RR��ֵ��Sn_TX_WR��ֵ��ȡ��û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0422��0x0522��0x0622��0x0722),Ȼ���ٶ����ֽ�(0x0423��0x0523��0x0623��0x0723))

#define W5100_S0_TX_WR		COMMON_BASE+0x0424		//Socket 0 TX write pointer register, R/W, default=0x0000(�˿�0����дָ��Ĵ���,�ɶ���д,ȱʡֵ0x0000,�üĴ���ָʾ����TX �洢��д������ʱ�ĵ�ַ���û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0424��0x0524��0x0624��0x0724),Ȼ���ٶ����ֽ�(0x0425��0x0525��0x0625��0x0725))
#define W5100_S1_TX_WR		COMMON_BASE+0x0524		//Socket 1 TX write pointer register, R/W, default=0x0000(�˿�1����дָ��Ĵ���,�ɶ���д,ȱʡֵ0x0000,�üĴ���ָʾ����TX �洢��д������ʱ�ĵ�ַ���û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0424��0x0524��0x0624��0x0724),Ȼ���ٶ����ֽ�(0x0425��0x0525��0x0625��0x0725))
#define W5100_S2_TX_WR		COMMON_BASE+0x0624		//Socket 2 TX write pointer register, R/W, default=0x0000(�˿�2����дָ��Ĵ���,�ɶ���д,ȱʡֵ0x0000,�üĴ���ָʾ����TX �洢��д������ʱ�ĵ�ַ���û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0424��0x0524��0x0624��0x0724),Ȼ���ٶ����ֽ�(0x0425��0x0525��0x0625��0x0725))
#define W5100_S3_TX_WR		COMMON_BASE+0x0724		//Socket 3 TX write pointer register, R/W, default=0x0000(�˿�3����дָ��Ĵ���,�ɶ���д,ȱʡֵ0x0000,�üĴ���ָʾ����TX �洢��д������ʱ�ĵ�ַ���û���ȡ�üĴ���ʱ,�����ȶ����ֽ�(0x0424��0x0524��0x0624��0x0724),Ȼ���ٶ����ֽ�(0x0425��0x0525��0x0625��0x0725))

//should read upper byte first and lower byte later
#define W5100_S0_RX_RSR		COMMON_BASE+0x0426		//Socket 0 RX size register, RO, default=0x0000(���������ֽ����Ĵ���,ֻ��,ȱʡֵ0x0000,)
#define W5100_S1_RX_RSR		COMMON_BASE+0x0526		//Socket 1 RX size register, RO, default=0x0000
#define W5100_S2_RX_RSR		COMMON_BASE+0x0626		//Socket 2 RX size register, RO, default=0x0000
#define W5100_S3_RX_RSR		COMMON_BASE+0x0726		//Socket 3 RX size register, RO, default=0x0000

//should read upper byte first and lower byte later
#define W5100_S0_RX_RR		COMMON_BASE+0x0428		//Socket 0 RX read pointer register, R/W, default=0x0000(�˿�n ���ջ�������ָ��Ĵ���,�ɶ���д,ȱʡֵ0x0000,)
#define W5100_S1_RX_RR		COMMON_BASE+0x0528		//Socket 1 RX read pointer register, R/W, default=0x0000
#define W5100_S2_RX_RR		COMMON_BASE+0x0628		//Socket 2 RX read pointer register, R/W, default=0x0000
#define W5100_S3_RX_RR		COMMON_BASE+0x0728		//Socket 3 RX read pointer register, R/W, default=0x0000

//should read upper byte first and lower byte later
#define W5100_S0_RX_WR		COMMON_BASE+0x042A		//Socket 0 RX read pointer register, R/W, default=0x0000(,�ɶ���д,ȱʡֵ0x0000,)
#define W5100_S1_RX_WR		COMMON_BASE+0x052A		//Socket 1 RX read pointer register, R/W, default=0x0000
#define W5100_S2_RX_WR		COMMON_BASE+0x062A		//Socket 2 RX read pointer register, R/W, default=0x0000
#define W5100_S3_RX_WR		COMMON_BASE+0x072A		//Socket 3 RX read pointer register, R/W, default=0x0000

#define W5100_TX	COMMON_BASE+0x4000	//TX memory
#define W5100_RX	COMMON_BASE+0x6000	//RX memory

//#define TRUE	0xff
//#define FALSE	0x00

/* ����ʹ��CC2530��SPI���� */
#define W5100_USING_CC2530_SPI

/***************----- W5100 GPIO���� -----***************/
#define W5100_RST   P0_4	//����W5100��RST����
#define W5100_INT   P0_5	//����W5100��INT����
#define W5100_SCS   P1_4	//����W5100��CS����	 	
#define W5100_SCLK  P1_5	//����W5100��SCLK����	
#define W5100_MOSI  P1_6	//����W5100��MOSI����	 
#define W5100_MISO  P1_7	//����W5100��MISO���� 

/***************----- ��������������� -----***************/
extern unsigned char Gateway_IP[4];	//����IP��ַ 
extern unsigned char Sub_Mask[4];	//�������� 
extern unsigned char Phy_Addr[6];	//�����ַ(MAC) 
extern unsigned char IP_Addr[4];	//����IP��ַ 

extern unsigned char S0_Port[2];	//�˿�0�Ķ˿ں�(5000) 
extern unsigned char S0_DIP[4];		//�˿�0Ŀ��IP��ַ 
extern unsigned char S0_DPort[2];	//�˿�0Ŀ�Ķ˿ں�(6000) 

extern unsigned char UDP_DIPR[4];	//UDP(�㲥)ģʽ,Ŀ������IP��ַ
extern unsigned char UDP_DPORT[2];	//UDP(�㲥)ģʽ,Ŀ�������˿ں�

/***************----- �˿ڵ�����ģʽ -----***************/
extern unsigned char S0_Mode;	//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
#define TCP_SERVER		0x00	//TCP������ģʽ
#define TCP_CLIENT		0x01	//TCP�ͻ���ģʽ 
#define UDP_MODE		0x02	//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
extern unsigned char S0_State;	//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������)  
#define S_INIT			0x01	//�˿���ɳ�ʼ�� 
#define S_CONN			0x02	//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
extern unsigned char S0_Data;		//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
#define S_RECEIVE		0x01		//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK	0x02		//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
extern unsigned char Rx_Buffer[30];	//�˿ڽ������ݻ����� 
extern unsigned char Tx_Buffer[30];	//�˿ڷ������ݻ����� 

extern unsigned char W5100_Interrupt;	//W5100�жϱ�־(0:���ж�,1:���ж�)
typedef unsigned char SOCKET;			//�Զ���˿ں���������

extern void Delay(unsigned int d);//��ʱ����(ms)
extern void W5100_Hardware_Reset(void);//Ӳ����λW5100
extern void W5100_Init(void);//��ʼ��W5100�Ĵ�������
extern unsigned char Detect_Gateway(void);//������ط�����
extern void Socket_Init(SOCKET s);//ָ��Socket(0~3)��ʼ��
extern unsigned char Socket_Connect(SOCKET s);//����ָ��Socket(0~3)Ϊ�ͻ�����Զ�̷���������
extern unsigned char Socket_Listen(SOCKET s);//����ָ��Socket(0~3)��Ϊ�������ȴ�Զ������������
extern unsigned char Socket_UDP(SOCKET s);//����ָ��Socket(0~3)ΪUDPģʽ
extern unsigned short S_rx_process(SOCKET s);//ָ��Socket(0~3)�������ݴ���
extern unsigned char S_tx_process(SOCKET s, unsigned int size);//ָ��Socket(0~3)�������ݴ���
//extern void W5100_Interrupt_Process(void);//W5100�жϴ��������

extern void W5100_Initialization(void);
extern void W5100_Socket_Start(void);

/* ����W5100�ķ��ͽӿ� */
extern uint16 W5100_SocketSend(SOCKET s, unsigned char *pcBuff, unsigned int size);

/* ����W5100�Ľ��սӿ� */
extern uint16 W5100_SocketRecv(SOCKET s, unsigned char **ppcBuff);

#ifdef __cplusplus
}
#endif

#endif /* __W5100_H_ */
