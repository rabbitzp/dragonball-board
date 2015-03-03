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

#define S_RX_SIZE	2048	/*定义Socket接收缓冲区的大小，可以根据W5100_RMSR的设置修改 */
#define S_TX_SIZE	2048  	/*定义Socket发送缓冲区的大小，可以根据W5100_TMSR的设置修改 */


/*-----------------------------------------------------------------------------*/
/*----------------- W5100 Register definition(W5100寄存器定义)-----------------*/
/*-----------------------------------------------------------------------------*/

#define	COMMON_BASE	0x0000		//Base address(根据需要修改基地址)

/*-----------------------------------------------------------------------------*/
/*-------------------- Common register(W5100命令寄存器定义)--------------------*/
/*-----------------------------------------------------------------------------*/
#define W5100_MODE	COMMON_BASE	//Mode register,R/W,Default=0x00(模式寄存器,可读可写,缺省值0x00,该寄存器用于软件复位、Ping关闭模式、PPPoE模式以及间接总线接口)
	#define MODE_RST	0x80	//Software reset(软件复位,置1有效,复位后自动清0,芯片内部寄存器将被初始化为缺省值)
	#define MODE_MT		0x20	//Memory test
	#define MODE_PB		0x10	//Ping block mode(Ping阻止模式,0关闭Ping阻止,1启动Ping阻止)
	#define MODE_PPPOE	0x08	//PPOE mode(PPPoE模式,0:关闭PPPoE模式,1:打开PPPoE模式,如果不经过路由器直接连接到ADSL,该位必须置“1”以便与ADSL服务器连接。欲知详情,请参照“How to connect ADSL”应用笔记)
	#define MODE_LB		0x04	//Little_endian/Big_endian ordering in indirect bus I/F
	#define MODE_AI		0x02	//Address auto increment in indirect bus I/F(间接总线接口模式下地址自动增加,0：禁止地址自动增加,1:开启地址自动增加,在间接总线接口模式下:当该位置“1”时:每次读/ 写操作后地址自动增加1。详情可参考”6.2间接总线口模式”)
	#define MODE_IND	0x01	//Indirect bus I/F mode(间接总线接口模式,0:禁止间接总线接口模式,1:启动间接总线接口模式,当该位置“1”时,采用间接总线接口模式。详情可参照”6.应用信息”,”6.2间接总线接口模式”)

#define	W5100_GAR	COMMON_BASE+0x01	//Gateway address register,R/W,default=0x00(网关IP地址寄存器,可读可写,缺省值0x00,该寄存器设置默认的网关地址,例:192.168.1.1)
#define	W5100_SUBR	COMMON_BASE+0x05	//Subnet mask address, R/W, default=0x00(子网掩码寄存器,可读可写,缺省值0x00,该寄存器用来设置子网掩码(Subnet Mask)值,例:255.255.255.0)
#define	W5100_SHAR	COMMON_BASE+0x09	//Source hardware address, R/W, default=0x00(本机物理地址(MAC)寄存器,可读可写,缺省值0x00,该寄存器用来设置本机物理地址,例:00.08.DC.01.02.03->0x00,0x08,0xdc,0x01,0x02,0x03)
#define	W5100_SIPR	COMMON_BASE+0x0f	//Source IP address, R/W, default=0x00(本机IP地址寄存器,可读可写,缺省值0x00,该寄存器用来设置本机IP地址,例:192.168.0.3->0xc0,0xa8,0x00,0x03)

#define	W5100_IR	COMMON_BASE+0x15	//Interrupt and interrupt mask register, RO, Default=0x00(中断寄存器,只读,缺省值0x00,CPU通过访问该寄存器获得产生中断的来源,任何中断源都可以被中断屏蔽寄存器（IMR ）中的位屏蔽。当任何一个未屏蔽的中断位为“1”,/INT的信号将保持低电平。只有当所有未屏蔽的中断位为0,/INT才恢复高电平。)
	#define IR_CONFLICT	0x80			//IP conflict(IP地址冲突,当一个与本机IP地址相同IP地址作ARP请求时,该位被置“1”,对该位写”1”可清0)
	#define IR_UNREACH	0x40			//Destination unreachable(无法到达地址,在UDP数据包发送过程中,如果目的IP地址不存在时,W5100将会收到一ICMP(目的无法到达)数据包,(参照5.2.2.UDP ),在该状态下,IP地址及端口号将被存到UIPR和UPORT 寄存器,同时该位置“1”,对该位写”1”可清0 )
	#define IR_PPPOE	0x20			//PPOE close(PPPoE连接关闭,在PPPoE模式,如果PPPoE连接被关闭,该位置“1”,对该位写”1”可清0)
	#define IR_S3_INT	0x08			//Occurrence of socket 3 socket interrupt(端口3中断,端口3中断产生时,该位被置“1”。(详情请参照端口中断寄存器S3_IR),当S3_IR清0,该位自动清0)
	#define IR_S2_INT	0x04			//Occurrence of socket 2 socket interrupt(端口2中断,端口2中断产生时,该位被置“1”。(详情请参照端口中断寄存器S2_IR),当S2_IR清0,该位自动清0)
	#define IR_S1_INT	0x02			//Occurrence of socket 1 socket interrupt(端口1中断,端口1中断产生时,该位被置“1”。(详情请参照端口中断寄存器S1_IR),当S1_IR清0,该位自动清0)
	#define IR_S0_INT	0x01			//Occurrence of socket 0 socket interrupt(端口0中断,端口0中断产生时,该位被置“1”。(详情请参照端口中断寄存器S0_IR),当S0_IR清0,该位自动清0)

#define	W5100_IMR	COMMON_BASE+0x16	//R/W, Default=0x00(中断屏蔽寄存器,可读可写,缺省值0x00,中断屏蔽寄存器(IMR)用来屏蔽中断源,每个中断屏蔽位对应中断寄存器(IR)中的一个位。如果中断屏蔽位被置“1”时,任何时候只要IR对应的位也置”1”,中断将会产生。而当IMR中屏蔽位被清“0”,即使对应的IR中断位被置“1”,中断也不会产生)
	#define IMR_CONFLICT 0x80			//IP conflict(允许IP冲突产生中断)
	#define IMR_UNREACH	0x40			//Destination unreachable(允许地址无法到达产生中断)
	#define IMR_PPPOE	0x20			//PPOE close(允许PPPoE关闭产生中断)
	#define IMR_S3_INT	0x08			//Occurrence of socket 3 socket interrupt(允许端口3产生中断)
	#define IMR_S2_INT	0x04			//Occurrence of socket 2 socket interrupt(允许端口2产生中断)
	#define IMR_S1_INT	0x02			//Occurrence of socket 1 socket interrupt(允许端口1产生中断)
	#define IMR_S0_INT	0x01			//Occurrence of socket 0 socket interrupt(允许端口0产生中断)

#define W5100_RTR	COMMON_BASE+0x17	//Retry time value. Value 1 means 100us, R/W, default=0x07D0(重发时间寄存器,该寄存器用来设置溢出的时间值,每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒,当发出CONNECT、DISCON、CLOSE、SEND、SEND_MAC及SEND_KEEP等命令而没有收到远程对端的响应、或响应延时,都会导致重发过程)
#define W5100_RCR	COMMON_BASE+0X19	//Retry count, R/W, Default=0x08(重发计数寄存器,该寄存器内的数值设定可重发的次数。如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”))

//--------------------------------------------------------    S1      S0      Memory size
//  Socket3    |  Socket2    |  Socket1    |  Socket0    |    0       0           1KB
//-------------|-------------|-------------|-------------|    0       1           2KB
//S1       S0  |S1       S0  |S1       S0  |S1       S0  |    1       0           4KB
//--------------------------------------------------------    1       1           8KB
#define W5100_RMSR	COMMON_BASE+0x1a	//RX memory size register, R/W, default=0x55(接收存储器配置寄存器,可读可写,缺省值0x55,此寄存器配置全部8K的RX存储空间到各指定端口,在8K范围内,从端口0开始,由S0和S1确定4个端口的接收缓冲区的大小,如果有端口分配不到存储空间,该端口就不能使用,初始化值为0x55,4个端口分别分配2K的存储空间)
#define W5100_TMSR	COMMON_BASE+0x1b	//TX memory size register, R/W, default=0x55(发送存储配置寄存器,可读可写,缺省值0x55,该寄存器用来将8K的发送存储区分配给每个端口。发送存储区的设置方法与接收存储区的设置完全一样。初始化值为0x55，即每端口分别分配2KB 的空间)
#define W5100_PATR	COMMON_BASE+0x1c	//Authentication type in PPPOE mode, R, default=0x0000(PPPoE模式下的认证类型,只读,缺省值0x0000,在与PPPoE服务器连接时,该寄存器指示已经被通过的安全认证方法。W5100只支持两种安全认证类型:PAP(0xC023)及CHAP(0xC223))
#define W5100_PTIMER	COMMON_BASE+0x28//PPP LCP request timer register, R/W, default=0x28, Value 1 is about 25ms(PPP LCP请求计时寄存器,可读可写,缺省值0x28,该寄存器表示发出LCP Echo(响应请求)所需要的时间间隔,每1单位大约25毫秒) 
#define W5100_PMAGIC	COMMON_BASE+0x29//PPP LCP magic number register, R/W, default=0x00(PPP LCP魔数寄存器,可读可写,缺省值0x00,该寄存器用于LCP握手时采用的魔数选项。参照“How to connect ADSL”应用笔记)
#define W5100_UIPR	COMMON_BASE+0x2a	//Unreachable IP address, RO, default=0x00(无法到达的IP 地址寄存器,只读,缺省值0x00,在UDP数据传输时(参照5.2.2. UDP),如果目的IP地址不存在,将会收到一个ICMP(地址无法到达)数据包。在这种情况下,无法到达的IP地址及端口号将分别存储到UIPR和UPORT中)
#define W5100_UPORT	COMMON_BASE+0x2e	//Unreachable port register, RO, default=0x0000(无法到达的端口号寄存器,只读,缺省值0x0000)

/*-----------------------------------------------------------------------------*/
/*-------------------- Socket register(W5100端口寄存器定义)--------------------*/
/*-----------------------------------------------------------------------------*/
#define W5100_S0_MR	COMMON_BASE+0x0400	//Socket 0 mode register, R/W, default=0x00(端口0模式寄存器,可读可写,缺省值0x00,该寄存器设置相应端口的选项或协议类型)
#define W5100_S1_MR	COMMON_BASE+0x0500	//Socket 1 mode register, R/W, default=0x00(端口1模式寄存器,可读可写,缺省值0x00,该寄存器设置相应端口的选项或协议类型)
#define W5100_S2_MR	COMMON_BASE+0x0600	//Socket 2 mode register, R/W, default=0x00(端口2模式寄存器,可读可写,缺省值0x00,该寄存器设置相应端口的选项或协议类型)
#define W5100_S3_MR	COMMON_BASE+0x0700	//Socket 3 mode register, R/W, default=0x00(端口3模式寄存器,可读可写,缺省值0x00,该寄存器设置相应端口的选项或协议类型)
	#define S_MR_MULTI	0x80	//Multcasting(广播功能,0:关闭广播功能,1:启动多广播功能,广播功能只能在UDP模式下使用。使用广播时,必需在OPEN命令前,将广播组地址和端口号写入到端口n的目的IP地址寄存器和目的端口号寄存器(n为0~3))
	#define S_MR_MC		0x20	//Multcast(使用无延时响应,0:禁止无延时响应选项,1:允许无延时响应选项,该功能只有在TCP模式下有效，为”1”时，无论什么时候从对端收到数据，都会发送一个ACK响应，为”0”时，将根据内部延时机制发送ACK响应)
								//  P3   P2   P1   P0     Meaning
	#define S_MR_CLOSED	0x00	//  0    0    0    0      Closed(关闭)
	#define S_MR_TCP	0x01	//  0    0    0    1      TCP(TCP)
	#define S_MR_UDP	0x02	//  0    0    1    0      UDP(UDP)
	#define S_MR_IPRAW	0x03	//  0    0    1    1      IPRAW(IPRAW)
	#define S_MR_MACRAW	0x04	//  0    1    0    0      MACRAW(端口0的协议类型增加了MACRAW)
	#define S_MR_PPPOE	0x05	//  0    1    0    1      PPPOE(端口0的协议类型增加了PPPOE)

#define W5100_S0_CR	COMMON_BASE+0x0401	//Socket 0 command register, R/W, default=0x00(端口0命令寄存器,可读可写,缺省值0x00,该寄存器用来设置端口的初始化、关闭、建立连接、断开连接、数据传输以及命令接收等,命令执行后,寄存器的值自动清0x00)
#define W5100_S1_CR	COMMON_BASE+0x0501	//Socket 1 command register, R/W, default=0x00(端口1命令寄存器,可读可写,缺省值0x00,该寄存器用来设置端口的初始化、关闭、建立连接、断开连接、数据传输以及命令接收等,命令执行后,寄存器的值自动清0x00)
#define W5100_S2_CR	COMMON_BASE+0x0601	//Socket 2 command register, R/W, default=0x00(端口2命令寄存器,可读可写,缺省值0x00,该寄存器用来设置端口的初始化、关闭、建立连接、断开连接、数据传输以及命令接收等,命令执行后,寄存器的值自动清0x00)
#define W5100_S3_CR	COMMON_BASE+0x0701	//Socket 3 command register, R/W, default=0x00(端口3命令寄存器,可读可写,缺省值0x00,该寄存器用来设置端口的初始化、关闭、建立连接、断开连接、数据传输以及命令接收等,命令执行后,寄存器的值自动清0x00)
	#define S_CR_OPEN	0x01	//It is used to initialize the socket(用于初始化端口,根据端口n模式寄存器(Sn_MR) 的设置,端口n的状态寄存器(Sn_SSR)的值将改变为SOCK_INIT、SOCK_UDP、SOCK_IPRAW 或SOCK_MACRAW ,详情参照”5.功能描述”)
	#define S_CR_LISTEN	0x02	//In TCP mode, it waits for a connection request from any remote peer(Client)(只有TCP模式有效,它将端口n的状态寄存器(Sn_SSR)改变为SOCK_LISTEN,以便等待远程对端发送的连接请求。详情参照”5.2.1.1 服务器模式”)
	#define S_CR_CONNECT 0x04	//In TCP mode, it sends a connection request to remote peer(SERVER)(只有TCP模式有效,它发送一个连接请求到远程对端服务器。如连接失败,将产生超时中断。详情参照”5.2.1.2客户端模式)
	#define S_CR_DISCON	0x08	//In TCP mode, it sends a connection termination request(只有TCP模式时有效,它发送终止连接请求。如连接终止失败,将产生时间溢时中断。详情参照”5.2.1.1 服务器模式”,如果用CLOSE命令代替DISCON命令,只有端口n的状态寄存器(Sn_SSR)改变成SOCK_CLOSED,而不进行终止连接过程)
	#define S_CR_CLOSE	0x10	//Used to close the socket(该命令用于关闭端口。它改变端口n的状态寄存器(Sn_SSR)为SOCK_CLOSED)
	#define S_CR_SEND	0x20	//It transmit the data as much as the increase size of write pointer(按照端口n的TX写指针增加的大小发送数据。详情参考端口n发送剩余空间寄存器(Sn_TX_FSR)、端口n发送写指针寄存器(Sn_TX_WR)、端口n发送读指针寄存器(Sn_TX_RR),或”5.2.1.1 服务器模式”)
	#define S_CR_SEND_MAC  0x21	//In UDP mode, same as SEND(它在UDP模式有效,基本功能与SEND相同。通常SEND操作过程需要用到由ARP解析的目标硬件地址。而SEND_MAC操作时使用用户设置的目的物理地址(Sn_DHAR),而没有ARP过程)
	#define S_CR_SEND_KEEP 0x22	//In TCP mode(只有TCP模式有效,它通过发送1个字节的数据检查端口的连接状态。如果连接已经终止或对端没有响应，将产生超时中断)
	#define S_CR_RECV	0x40	//Receiving is processed including the value of socket RX read pointer register(按照端口n的读指针寄存器(Sn_RX_RR)中的数据接收处理完成。详情请参照端口n接收数据大小寄存器(Sn_RX_RSR)、端口n接收写指针寄存器(Sn_RX_WR)、端口n接收读指针寄存器(Sn_RX_RR))

	/* Definition for PPPOE */
	#define S_CR_PCON	0x23	//Start of ADSL connection
	#define S_CR_PDISCON 0x24	//End of ADSL connection
	#define S_CR_PCR	0x25	//Send REQ message ineach phase
	#define S_CR_PCN	0x26	//Send NAK message in each phase
	#define S_CR_PCJ	0x27	//Senf REJECT message in each phase

#define W5100_S0_IR	COMMON_BASE+0x0402	//Socket 0 interrup register, RO, default=0x00(端口0中断寄存器,只读,缺省值0x00,该寄存器指示建立和终止连接、接收数据、发送完成以及时间溢出等信息。寄存器中相应的位被置”1”后必须写入”1”才清0)
#define W5100_S1_IR	COMMON_BASE+0x0502	//Socket 1 interrup register, RO, default=0x00(端口1中断寄存器,只读,缺省值0x00,该寄存器指示建立和终止连接、接收数据、发送完成以及时间溢出等信息。寄存器中相应的位被置”1”后必须写入”1”才清0)
#define W5100_S2_IR	COMMON_BASE+0x0602	//Socket 2 interrup register, RO, default=0x00(端口2中断寄存器,只读,缺省值0x00,该寄存器指示建立和终止连接、接收数据、发送完成以及时间溢出等信息。寄存器中相应的位被置”1”后必须写入”1”才清0)
#define W5100_S3_IR	COMMON_BASE+0x0702	//Socket 3 interrup register, RO, default=0x00(端口3中断寄存器,只读,缺省值0x00,该寄存器指示建立和终止连接、接收数据、发送完成以及时间溢出等信息。寄存器中相应的位被置”1”后必须写入”1”才清0)
	#define S_IR_SENDOK		0x10	//Send data complete(数据发送操作完成该位被置”1”)
	#define S_IR_TIMEOUT	0x08	//Set if timeout occurs during connection or termination or data transmission(在连接或终止、数据发送等过程中超时，该位被置“1”)
	#define S_IR_RECV		0x04	//Set if data is received(当端口接收到数据时置“1”，当执行CMD_RECV 命令后数据仍然保留，该位也为”1”)
	#define S_IR_DISCON		0x02	//Set if receiv connection termination request(当收到终止连接请求或终止连接过程已结束时，该位被置“1”)
	#define S_IR_CON		0x01	//Set if connetion is established(当连接成功时,该位被置“1”)

	/* Definition for PPPOE */
	#define S_IR_PRECV	0x80		//Indicate receiving no support option data
	#define S_IR_PFAIL	0x40		//Indicate PAP Authentication fail
	#define S_IR_PNEXT	0x20		//Go next phase

#define W5100_S0_SSR	COMMON_BASE+0x0403	//Socket 0 status register, RO, default=0x00(端口0状态寄存器,只读,缺省值0x00,该寄存器指示端口0的状态数值)
#define W5100_S1_SSR	COMMON_BASE+0x0503	//Socket 1 status register, RO, default=0x00(端口1状态寄存器,只读,缺省值0x00,该寄存器指示端口1的状态数值)
#define W5100_S2_SSR	COMMON_BASE+0x0603	//Socket 2 status register, RO, default=0x00(端口2状态寄存器,只读,缺省值0x00,该寄存器指示端口2的状态数值)
#define W5100_S3_SSR	COMMON_BASE+0x0703	//Socket 3 status register, RO, default=0x00(端口3状态寄存器,只读,缺省值0x00,该寄存器指示端口3的状态数值)
	#define S_SSR_CLOSED	0x00	//In case that OPEN command are given to Sn_CR, Timeout interrupt is asserted or connection is terminated(当向Sn_CR写入CLOSE命令产生超时中断或连接被终止,将出现SOCK_CLOSED状态。在SOCK_CLOSED 状态不产生任何操作,释放所有连接资源)
	#define S_SSR_INIT		0x13	//In case that Sn_MR is set as TCP and OPEN commands are given to Sn_CR(当Sn_MR设为TCP模式时,向Sn_CR中写入OPEN命令时将出现SOCK_INIT状态。这是端口建立TCP连接的开始。在SOCK_INIT状态,Sn_CR命令的类型决定了操作类型—TCP服务器模式或TCP客户端模式)
	#define S_SSR_LISTEN	0x14	//In case that under the SOCK_INIT status, LISTEN commands are given to Sn_CR(当端口处于SOCK_INIT状态时,向Sn_CR写入LISTEN命令时将产生SOCK_LISTEN状态。相应的端口设置为TCP服务器模式,如果收到连接请求将改变为ESTABLISHED状态)
	#define S_SSR_ESTABLISHED 0x17	//In case that connection is established(当端口建立连接时将产生SOCK_ESTABLISHED状态,在这种状态下可以发送和接收TCP数据)
	#define S_SSR_CLOSE_WAIT  0x1c	//In case that connection temination request is received(当收到来自对端的终止连接请求时,将产生SOCK_CLOSE_WAIT状态。在这种状态,从对端收到响应信息,但没有断开。当收到DICON或CLOSE命令时,连接断开)
	#define S_SSR_UDP		0x22	//In case that OPEN command is given to Sn_CR when Sn_MR is set as UDP(当Sn_MR设为UDP模式,向Sn_CR写入OPEN命令时将产生SOCK_UDP状态,在这种状态下通信不需要与对端建立连接,数据可以直接发送和接收)
	#define S_SSR_IPRAW		0x32	//In case that OPEN command is given to Sn_CR when Sn_MR is set as IPRAW(当Sn_MR设为IPRAW模式,向Sn_CR写入OPEN命令时将产生SOCK_IPRAW状态,在IP RAW状态,IP层以上的协议将不处理。详情请参考”IPRAW”)
	#define S_SSR_MACRAW	0x42	//In case that OPEN command is given to S0_CR when S0_MR is set as MACRAW(当Sn_MR设为MACRAW模式,向S0_CR写入OPEN命令时将产生SOCK_MACRAW状态。在MAC RAW状态,所有协议数据包都不处理,详情请参考”MAC RAW”)
	#define S_SSR_PPPOE		0x5f	//In case that OPEN command is given to S0_CR when S0_MR is set as PPPOE(当Sn_MR设为PPPoE模式,向S0_CR写入OPEN命令时将产生SOCK_PPPOE状态)
	//Below is shown in the status change, and does not need much attention
	#define S_SSR_SYNSEND	0x15	//(端口n在SOCK_INIT状态,向Sn_CR写入CONNECT命令时将出现SOCK_SYNSENT状态。如果连接成功,将自动改变为SOCK_ESTABLISH)
	#define S_SSR_SYNRECV	0x16	//(当接收到远程对端(客户端)发送的连接请求时,将出现SOCK_SYNRECV状态。响应请求后,状态改变为SOCK_ESTABLISH)
	#define S_SSR_FIN_WAIT	0x18	//(0x18,0x1a,0x1b,0x1b:在连接终止过程中将出现这些状态。如果连接终止已完成,或产生了时间溢出中断,状态将自动被改变为SOCK_CLOSED)
	#define S_SSR_CLOSING	0x1a
	#define S_SSR_TIME_WAIT	0x1b
	#define S_SSR_LAST_ACK	0x1d
	#define S_SSR_ARP0		0x11	//(0x11,0x21,0x31:在TCP模式下发出连接请求或在UDP模式下发送数据时,当发出ARP请求以获得远程对端的物理地址时,将出现SOCK_ARP状态。如果收到ARP响应,将产生SOCK_SYNSENT、SOCK_UDP 或SOCK_ICMP状态,以便下面的操作)
	#define S_SSR_ARP1		0x21
	#define S_SSR_ARP2		0x31

#define W5100_S0_PORT	COMMON_BASE+0x0404		//Socket 0 Source port register, R/W, default=0x00(端口0的端口号寄存器,可读可写,缺省值0x00,该寄存器在TCP或UDP模式下设定对应端口的端口号。这些端口号必须在进行OPEN指令之前完成)
#define W5100_S1_PORT	COMMON_BASE+0x0504		//Socket 1 Source port register, R/W, default=0x00(端口1的端口号寄存器,可读可写,缺省值0x00,该寄存器在TCP或UDP模式下设定对应端口的端口号。这些端口号必须在进行OPEN指令之前完成)
#define W5100_S2_PORT	COMMON_BASE+0x0604		//Socket 2 Source port register, R/W, default=0x00(端口2的端口号寄存器,可读可写,缺省值0x00,该寄存器在TCP或UDP模式下设定对应端口的端口号。这些端口号必须在进行OPEN指令之前完成)
#define W5100_S3_PORT	COMMON_BASE+0x0704		//Socket 3 Source port register, R/W, default=0x00(端口3的端口号寄存器,可读可写,缺省值0x00,该寄存器在TCP或UDP模式下设定对应端口的端口号。这些端口号必须在进行OPEN指令之前完成)
 
#define W5100_S0_DHAR	COMMON_BASE+0x0406		//Socket 0 destination hardware address register, R/W, default=0x00(端口0的目的物理地址寄存器,可读可写,缺省值0x00,该寄存器设置每个端口的目的物理地址)
#define W5100_S1_DHAR	COMMON_BASE+0x0506		//Socket 1 destination hardware address register, R/W, default=0x00(端口1的目的物理地址寄存器,可读可写,缺省值0x00,该寄存器设置每个端口的目的物理地址)
#define W5100_S2_DHAR	COMMON_BASE+0x0606		//Socket 2 destination hardware address register, R/W, default=0x00(端口2的目的物理地址寄存器,可读可写,缺省值0x00,该寄存器设置每个端口的目的物理地址)
#define W5100_S3_DHAR	COMMON_BASE+0x0706		//Socket 3 destination hardware address register, R/W, default=0x00(端口3的目的物理地址寄存器,可读可写,缺省值0x00,该寄存器设置每个端口的目的物理地址)

#define W5100_S0_DIPR	COMMON_BASE+0x040c		//Socket 0 destination IP address register, R/W, default=0x00(端口0的目的IP地址寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的目的IP地址。在主动模式(客户端模式),目的IP地址必需设新目的IP地址,在UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。没有收到对端的ARP 响应之前,该寄存器复位)
#define W5100_S1_DIPR	COMMON_BASE+0x050c		//Socket 1 destination IP address register, R/W, default=0x00(端口1的目的IP地址寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的目的IP地址。在主动模式(客户端模式),目的IP地址必需设新目的IP地址,在UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。没有收到对端的ARP 响应之前,该寄存器复位)
#define W5100_S2_DIPR	COMMON_BASE+0x060c		//Socket 2 destination IP address register, R/W, default=0x00(端口2的目的IP地址寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的目的IP地址。在主动模式(客户端模式),目的IP地址必需设新目的IP地址,在UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。没有收到对端的ARP 响应之前,该寄存器复位)
#define W5100_S3_DIPR	COMMON_BASE+0x070c		//Socket 3 destination IP address register, R/W, default=0x00(端口3的目的IP地址寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的目的IP地址。在主动模式(客户端模式),目的IP地址必需设新目的IP地址,在UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。没有收到对端的ARP 响应之前,该寄存器复位)

#define W5100_S0_DPORT	COMMON_BASE+0x0410		//Socket 0 destionation port register, R/W, default=0x00(端口0的目的端口号寄存器,可读可写,缺省值0x00,在TCP 模式,该寄存器设置端口的目的端口号。在主动模式下(客户端模式),目的端口号必需设置后再执行连接(CONNECT)命令。在被动模式下(服务器模式),W5100建立连接后,内部自动刷新目的端口号,在 UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。在没有收到对端的ARP响应之前,该寄存器复位)
#define W5100_S1_DPORT	COMMON_BASE+0x0510		//Socket 1 destionation port register, R/W, default=0x00(端口1的目的端口号寄存器,可读可写,缺省值0x00,在TCP 模式,该寄存器设置端口的目的端口号。在主动模式下(客户端模式),目的端口号必需设置后再执行连接(CONNECT)命令。在被动模式下(服务器模式),W5100建立连接后,内部自动刷新目的端口号,在 UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。在没有收到对端的ARP响应之前,该寄存器复位)
#define W5100_S2_DPORT	COMMON_BASE+0x0610		//Socket 2 destionation port register, R/W, default=0x00(端口2的目的端口号寄存器,可读可写,缺省值0x00,在TCP 模式,该寄存器设置端口的目的端口号。在主动模式下(客户端模式),目的端口号必需设置后再执行连接(CONNECT)命令。在被动模式下(服务器模式),W5100建立连接后,内部自动刷新目的端口号,在 UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。在没有收到对端的ARP响应之前,该寄存器复位)
#define W5100_S3_DPORT	COMMON_BASE+0x0710		//Socket 3 destionation port register, R/W, default=0x00(端口3的目的端口号寄存器,可读可写,缺省值0x00,在TCP 模式,该寄存器设置端口的目的端口号。在主动模式下(客户端模式),目的端口号必需设置后再执行连接(CONNECT)命令。在被动模式下(服务器模式),W5100建立连接后,内部自动刷新目的端口号,在 UDP模式,收到对端的ARP响应后,该寄存器才确定为用户写入的值。在没有收到对端的ARP响应之前,该寄存器复位)

#define W5100_S0_MSS	COMMON_BASE+0x0412		//Socket 0 maximum segment size register, R/W, default=0x00(端口0最大分片长度寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的最大分片长度,如果TCP在被动模式(Passive Mode)下,该寄存器的值由其它部分设置)
#define W5100_S1_MSS	COMMON_BASE+0x0512		//Socket 1 maximum segment size register, R/W, default=0x00(端口1最大分片长度寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的最大分片长度,如果TCP在被动模式(Passive Mode)下,该寄存器的值由其它部分设置)
#define W5100_S2_MSS	COMMON_BASE+0x0612		//Socket 2 maximum segment size register, R/W, default=0x00(端口2最大分片长度寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的最大分片长度,如果TCP在被动模式(Passive Mode)下,该寄存器的值由其它部分设置)
#define W5100_S3_MSS	COMMON_BASE+0x0712		//Socket 3 maximum segment size register, R/W, default=0x00(端口3最大分片长度寄存器,可读可写,缺省值0x00,在TCP模式,该寄存器设置端口的最大分片长度,如果TCP在被动模式(Passive Mode)下,该寄存器的值由其它部分设置)

#define W5100_S0_PROTO	COMMON_BASE+0x0414		//Socket 0 IP protocol register, R/W, default=0x00(端口0的IP协议寄存器,可读可写,缺省值0x00,在IP RAW模式下,IP协议寄存器用来设置IP数据包头的协议字段(Protocol Field)的值。IANA预先注册了一些协议号可以使用。IANA网站可查出全部的IP层协议代码。参考IANA的在线文档://www.iana.org/assignments/protocol-number)
#define W5100_S1_PROTO	COMMON_BASE+0x0514		//Socket 1 IP protocol register, R/W, default=0x00(端口1的IP协议寄存器,可读可写,缺省值0x00,在IP RAW模式下,IP协议寄存器用来设置IP数据包头的协议字段(Protocol Field)的值。IANA预先注册了一些协议号可以使用。IANA网站可查出全部的IP层协议代码。参考IANA的在线文档://www.iana.org/assignments/protocol-number)
#define W5100_S2_PROTO	COMMON_BASE+0x0614		//Socket 2 IP protocol register, R/W, default=0x00(端口2的IP协议寄存器,可读可写,缺省值0x00,在IP RAW模式下,IP协议寄存器用来设置IP数据包头的协议字段(Protocol Field)的值。IANA预先注册了一些协议号可以使用。IANA网站可查出全部的IP层协议代码。参考IANA的在线文档://www.iana.org/assignments/protocol-number)
#define W5100_S3_PROTO	COMMON_BASE+0x0714		//Socket 3 IP protocol register, R/W, default=0x00(端口3的IP协议寄存器,可读可写,缺省值0x00,在IP RAW模式下,IP协议寄存器用来设置IP数据包头的协议字段(Protocol Field)的值。IANA预先注册了一些协议号可以使用。IANA网站可查出全部的IP层协议代码。参考IANA的在线文档://www.iana.org/assignments/protocol-number)

#define W5100_S0_TOS	COMMON_BASE+0x0415		//Socket 0 IP type of servce register, R/W, default=0x00(端口0的IP服务类型寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的服务类型(TOS)字段的值)
#define W5100_S1_TOS	COMMON_BASE+0x0515		//Socket 1 IP type of servce register, R/W, default=0x00(端口1的IP服务类型寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的服务类型(TOS)字段的值)
#define W5100_S2_TOS	COMMON_BASE+0x0615		//Socket 2 IP type of servce register, R/W, default=0x00(端口2的IP服务类型寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的服务类型(TOS)字段的值)
#define W5100_S3_TOS	COMMON_BASE+0x0715		//Socket 3 IP type of servce register, R/W, default=0x00(端口3的IP服务类型寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的服务类型(TOS)字段的值)

#define W5100_S0_TTL	COMMON_BASE+0x0416		//Socket 0 IP time to live register, R/W, default=0x80(端口0的IP数据包生存期寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的生存期(TTL)字段的值)
#define W5100_S1_TTL	COMMON_BASE+0x0516		//Socket 1 IP time to live register, R/W, default=0x80(端口1的IP数据包生存期寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的生存期(TTL)字段的值)
#define W5100_S2_TTL	COMMON_BASE+0x0616		//Socket 2 IP time to live register, R/W, default=0x80(端口2的IP数据包生存期寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的生存期(TTL)字段的值)
#define W5100_S3_TTL	COMMON_BASE+0x0716		//Socket 3 IP time to live register, R/W, default=0x80(端口3的IP数据包生存期寄存器,可读可写,缺省值0x00,该寄存器用来设置IP数据包头中的生存期(TTL)字段的值)

#define W5100_S0_TX_FSR		COMMON_BASE+0x0420		//Socket 0 TX free size register, RO, default=0x0800(端口0发送存储器剩余空间寄存器,只读,缺省值0x0800,该寄存器指示用户可使用的发送数据空间的大小。在发送数据时,用户必需先检查剩余空间的大小,然后控制发送数据的字节数。检查该寄存器时,必需先读高字节(0x0420,0x0520,0x0620,0x0720),然后再读低字节(0x0421,0x0521,0x0621,0x0721),端口发送总空间的大小由发送存储器空间寄存器(TMSR)确定。在数据发送处理过程中,剩余空间的大小将因写入数据而减少,发送完成后自动增加)
#define W5100_S1_TX_FSR		COMMON_BASE+0x0520		//Socket 1 TX free size register, RO, default=0x0800(端口1发送存储器剩余空间寄存器,只读,缺省值0x0800,该寄存器指示用户可使用的发送数据空间的大小。在发送数据时,用户必需先检查剩余空间的大小,然后控制发送数据的字节数。检查该寄存器时,必需先读高字节(0x0420,0x0520,0x0620,0x0720),然后再读低字节(0x0421,0x0521,0x0621,0x0721),端口发送总空间的大小由发送存储器空间寄存器(TMSR)确定。在数据发送处理过程中,剩余空间的大小将因写入数据而减少,发送完成后自动增加)
#define W5100_S2_TX_FSR		COMMON_BASE+0x0620		//Socket 2 TX free size register, RO, default=0x0800(端口2发送存储器剩余空间寄存器,只读,缺省值0x0800,该寄存器指示用户可使用的发送数据空间的大小。在发送数据时,用户必需先检查剩余空间的大小,然后控制发送数据的字节数。检查该寄存器时,必需先读高字节(0x0420,0x0520,0x0620,0x0720),然后再读低字节(0x0421,0x0521,0x0621,0x0721),端口发送总空间的大小由发送存储器空间寄存器(TMSR)确定。在数据发送处理过程中,剩余空间的大小将因写入数据而减少,发送完成后自动增加)
#define W5100_S3_TX_FSR		COMMON_BASE+0x0720		//Socket 3 TX free size register, RO, default=0x0800(端口3发送存储器剩余空间寄存器,只读,缺省值0x0800,该寄存器指示用户可使用的发送数据空间的大小。在发送数据时,用户必需先检查剩余空间的大小,然后控制发送数据的字节数。检查该寄存器时,必需先读高字节(0x0420,0x0520,0x0620,0x0720),然后再读低字节(0x0421,0x0521,0x0621,0x0721),端口发送总空间的大小由发送存储器空间寄存器(TMSR)确定。在数据发送处理过程中,剩余空间的大小将因写入数据而减少,发送完成后自动增加)

#define W5100_S0_TX_RR		COMMON_BASE+0x0422		//Socket 0 TX read pointer register, RO, default=0x0000(端口0发送存储器读指针寄存器,只读,缺省值0x0000,该寄存器指示端口在发送过程完成后发送存储器的当前位置。当端口n的命令寄存器收到SEND命令,从当前Sn_TX_RR到Sn_TX_WR的数据将发送出去,发送完成后,Sn_TX_RR的值自动改变。因此发送完成后,Sn_TX_RR的值与Sn_TX_WR的值相等。用户读取该寄存器时,必须先读高字节(0x0422，0x0522，0x0622，0x0722),然后再读低字节(0x0423，0x0523，0x0623，0x0723))
#define W5100_S1_TX_RR		COMMON_BASE+0x0522		//Socket 1 TX read pointer register, RO, default=0x0000(端口1发送存储器读指针寄存器,只读,缺省值0x0000,该寄存器指示端口在发送过程完成后发送存储器的当前位置。当端口n的命令寄存器收到SEND命令,从当前Sn_TX_RR到Sn_TX_WR的数据将发送出去,发送完成后,Sn_TX_RR的值自动改变。因此发送完成后,Sn_TX_RR的值与Sn_TX_WR的值相等。用户读取该寄存器时,必须先读高字节(0x0422，0x0522，0x0622，0x0722),然后再读低字节(0x0423，0x0523，0x0623，0x0723))
#define W5100_S2_TX_RR		COMMON_BASE+0x0622		//Socket 2 TX read pointer register, RO, default=0x0000(端口2发送存储器读指针寄存器,只读,缺省值0x0000,该寄存器指示端口在发送过程完成后发送存储器的当前位置。当端口n的命令寄存器收到SEND命令,从当前Sn_TX_RR到Sn_TX_WR的数据将发送出去,发送完成后,Sn_TX_RR的值自动改变。因此发送完成后,Sn_TX_RR的值与Sn_TX_WR的值相等。用户读取该寄存器时,必须先读高字节(0x0422，0x0522，0x0622，0x0722),然后再读低字节(0x0423，0x0523，0x0623，0x0723))
#define W5100_S3_TX_RR		COMMON_BASE+0x0722		//Socket 3 TX read pointer register, RO, default=0x0000(端口3发送存储器读指针寄存器,只读,缺省值0x0000,该寄存器指示端口在发送过程完成后发送存储器的当前位置。当端口n的命令寄存器收到SEND命令,从当前Sn_TX_RR到Sn_TX_WR的数据将发送出去,发送完成后,Sn_TX_RR的值自动改变。因此发送完成后,Sn_TX_RR的值与Sn_TX_WR的值相等。用户读取该寄存器时,必须先读高字节(0x0422，0x0522，0x0622，0x0722),然后再读低字节(0x0423，0x0523，0x0623，0x0723))

#define W5100_S0_TX_WR		COMMON_BASE+0x0424		//Socket 0 TX write pointer register, R/W, default=0x0000(端口0传输写指针寄存器,可读可写,缺省值0x0000,该寄存器指示在向TX 存储器写入数据时的地址。用户读取该寄存器时,必须先读高字节(0x0424，0x0524，0x0624，0x0724),然后再读低字节(0x0425，0x0525，0x0625，0x0725))
#define W5100_S1_TX_WR		COMMON_BASE+0x0524		//Socket 1 TX write pointer register, R/W, default=0x0000(端口1传输写指针寄存器,可读可写,缺省值0x0000,该寄存器指示在向TX 存储器写入数据时的地址。用户读取该寄存器时,必须先读高字节(0x0424，0x0524，0x0624，0x0724),然后再读低字节(0x0425，0x0525，0x0625，0x0725))
#define W5100_S2_TX_WR		COMMON_BASE+0x0624		//Socket 2 TX write pointer register, R/W, default=0x0000(端口2传输写指针寄存器,可读可写,缺省值0x0000,该寄存器指示在向TX 存储器写入数据时的地址。用户读取该寄存器时,必须先读高字节(0x0424，0x0524，0x0624，0x0724),然后再读低字节(0x0425，0x0525，0x0625，0x0725))
#define W5100_S3_TX_WR		COMMON_BASE+0x0724		//Socket 3 TX write pointer register, R/W, default=0x0000(端口3传输写指针寄存器,可读可写,缺省值0x0000,该寄存器指示在向TX 存储器写入数据时的地址。用户读取该寄存器时,必须先读高字节(0x0424，0x0524，0x0624，0x0724),然后再读低字节(0x0425，0x0525，0x0625，0x0725))

//should read upper byte first and lower byte later
#define W5100_S0_RX_RSR		COMMON_BASE+0x0426		//Socket 0 RX size register, RO, default=0x0000(接收数据字节数寄存器,只读,缺省值0x0000,)
#define W5100_S1_RX_RSR		COMMON_BASE+0x0526		//Socket 1 RX size register, RO, default=0x0000
#define W5100_S2_RX_RSR		COMMON_BASE+0x0626		//Socket 2 RX size register, RO, default=0x0000
#define W5100_S3_RX_RSR		COMMON_BASE+0x0726		//Socket 3 RX size register, RO, default=0x0000

//should read upper byte first and lower byte later
#define W5100_S0_RX_RR		COMMON_BASE+0x0428		//Socket 0 RX read pointer register, R/W, default=0x0000(端口n 接收缓冲区读指针寄存器,可读可写,缺省值0x0000,)
#define W5100_S1_RX_RR		COMMON_BASE+0x0528		//Socket 1 RX read pointer register, R/W, default=0x0000
#define W5100_S2_RX_RR		COMMON_BASE+0x0628		//Socket 2 RX read pointer register, R/W, default=0x0000
#define W5100_S3_RX_RR		COMMON_BASE+0x0728		//Socket 3 RX read pointer register, R/W, default=0x0000

//should read upper byte first and lower byte later
#define W5100_S0_RX_WR		COMMON_BASE+0x042A		//Socket 0 RX read pointer register, R/W, default=0x0000(,可读可写,缺省值0x0000,)
#define W5100_S1_RX_WR		COMMON_BASE+0x052A		//Socket 1 RX read pointer register, R/W, default=0x0000
#define W5100_S2_RX_WR		COMMON_BASE+0x062A		//Socket 2 RX read pointer register, R/W, default=0x0000
#define W5100_S3_RX_WR		COMMON_BASE+0x072A		//Socket 3 RX read pointer register, R/W, default=0x0000

#define W5100_TX	COMMON_BASE+0x4000	//TX memory
#define W5100_RX	COMMON_BASE+0x6000	//RX memory

//#define TRUE	0xff
//#define FALSE	0x00

/* 定义使用CC2530的SPI界面 */
#define W5100_USING_CC2530_SPI

/***************----- W5100 GPIO定义 -----***************/
#define W5100_RST   P0_4	//定义W5100的RST引脚
#define W5100_INT   P0_5	//定义W5100的INT引脚
#define W5100_SCS   P1_4	//定义W5100的CS引脚	 	
#define W5100_SCLK  P1_5	//定义W5100的SCLK引脚	
#define W5100_MOSI  P1_6	//定义W5100的MOSI引脚	 
#define W5100_MISO  P1_7	//定义W5100的MISO引脚 

/***************----- 网络参数变量定义 -----***************/
extern unsigned char Gateway_IP[4];	//网关IP地址 
extern unsigned char Sub_Mask[4];	//子网掩码 
extern unsigned char Phy_Addr[6];	//物理地址(MAC) 
extern unsigned char IP_Addr[4];	//本机IP地址 

extern unsigned char S0_Port[2];	//端口0的端口号(5000) 
extern unsigned char S0_DIP[4];		//端口0目的IP地址 
extern unsigned char S0_DPort[2];	//端口0目的端口号(6000) 

extern unsigned char UDP_DIPR[4];	//UDP(广播)模式,目的主机IP地址
extern unsigned char UDP_DPORT[2];	//UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
extern unsigned char S0_Mode;	//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER		0x00	//TCP服务器模式
#define TCP_CLIENT		0x01	//TCP客户端模式 
#define UDP_MODE		0x02	//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
extern unsigned char S0_State;	//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据)  
#define S_INIT			0x01	//端口完成初始化 
#define S_CONN			0x02	//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
extern unsigned char S0_Data;		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE		0x01		//端口接收到一个数据包 
#define S_TRANSMITOK	0x02		//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
extern unsigned char Rx_Buffer[30];	//端口接收数据缓冲区 
extern unsigned char Tx_Buffer[30];	//端口发送数据缓冲区 

extern unsigned char W5100_Interrupt;	//W5100中断标志(0:无中断,1:有中断)
typedef unsigned char SOCKET;			//自定义端口号数据类型

extern void Delay(unsigned int d);//延时函数(ms)
extern void W5100_Hardware_Reset(void);//硬件复位W5100
extern void W5100_Init(void);//初始化W5100寄存器函数
extern unsigned char Detect_Gateway(void);//检查网关服务器
extern void Socket_Init(SOCKET s);//指定Socket(0~3)初始化
extern unsigned char Socket_Connect(SOCKET s);//设置指定Socket(0~3)为客户端与远程服务器连接
extern unsigned char Socket_Listen(SOCKET s);//设置指定Socket(0~3)作为服务器等待远程主机的连接
extern unsigned char Socket_UDP(SOCKET s);//设置指定Socket(0~3)为UDP模式
extern unsigned short S_rx_process(SOCKET s);//指定Socket(0~3)接收数据处理
extern unsigned char S_tx_process(SOCKET s, unsigned int size);//指定Socket(0~3)发送数据处理
//extern void W5100_Interrupt_Process(void);//W5100中断处理程序框架

extern void W5100_Initialization(void);
extern void W5100_Socket_Start(void);

/* 定义W5100的发送接口 */
extern uint16 W5100_SocketSend(SOCKET s, unsigned char *pcBuff, unsigned int size);

/* 定义W5100的接收接口 */
extern uint16 W5100_SocketRecv(SOCKET s, unsigned char **ppcBuff);

#ifdef __cplusplus
}
#endif

#endif /* __W5100_H_ */
