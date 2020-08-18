#include "Drivers.h"
#include "Sx1276.h"

static u32 gMyRxAddr=0;

#define Select_Sx1276()     IOOUT_SetIoStatus(IOOUT_WRF_DRV_CS, FALSE)
#define NotSelect_Sx1276()    IOOUT_SetIoStatus(IOOUT_WRF_DRV_CS, TRUE)
#if PRODUCT_IS_JUMPER
#define Sx1276_Spi_Init() SPI1_Init()
#define Sx1276_Spi_Read() SPI_ReadByte(SPI1)
#define	Sx1276_Spi_Write(x) SPI_WriteByte(SPI1,x)
#elif PRODUCT_IS_WAVER
#define Sx1276_Spi_Init() SPI2_Init()
#define Sx1276_Spi_Read() SPI_ReadByte(SPI2)
#define	Sx1276_Spi_Write(x) SPI_WriteByte(SPI2,x)
#else
#define Sx1276_Spi_Init() SPI2_Init()
#define Sx1276_Spi_Read() SPI_ReadByte(SPI2)
#define	Sx1276_Spi_Write(x) SPI_WriteByte(SPI2,x)
#endif
//#define ANT_TX1_RX0()	Sx1276_WriteReg(R_IO_CONF,0x01)		// 发射状态的天线开关定义
//#define ANT_TX0_RX1()	Sx1276_WriteReg(R_IO_CONF,0x02)		// 接收状态的天线开关定义
//#define ANT_TX0_RX0()	Sx1276_WriteReg(R_IO_CONF,0x00)         // 非发射，接收状态的天线开关定义
#define WRESET_0()  IOOUT_SetIoStatus(IOOUT_WRF_DRV_RESET, FALSE)
#define WRESET_1()  IOOUT_SetIoStatus(IOOUT_WRF_DRV_RESET, TRUE)
#define NIRQ() 	IOIN_ReadIoStatus(IOIN_WRF_DRV_INT)
#define READREG        0x7f  	//读寄存器指令
#define WRITEREG       0x80 	//写寄存器指令


//寄存器及功能位
#define REG_LR_FIFO                                  0x00 
#define REG_LR_OPMODE                                0x01 
#define REG_LR_BANDSETTING                           0x04
#define REG_LR_FRFMSB                                0x06 
#define REG_LR_FRFMID                                0x07
#define REG_LR_FRFLSB                                0x08 
 // Tx settings
#define REG_LR_PACONFIG                              0x09 
#define REG_LR_PARAMP                                0x0A 
#define REG_LR_OCP                                   0x0B 
 // Rx settings
#define REG_LR_LNA                                   0x0C 
 // LoRa registers
#define REG_LR_FIFOADDRPTR                           0x0D 
#define REG_LR_FIFOTXBASEADDR                        0x0E 
#define REG_LR_FIFORXBASEADDR                        0x0F 
#define REG_LR_FIFORXCURRENTADDR                     0x10 
#define REG_LR_IRQFLAGSMASK                          0x11 
#define REG_LR_IRQFLAGS                              0x12 
#define REG_LR_NBRXBYTES                             0x13 
#define REG_LR_RXHEADERCNTVALUEMSB                   0x14 
#define REG_LR_RXHEADERCNTVALUELSB                   0x15 
#define REG_LR_RXPACKETCNTVALUEMSB                   0x16 
#define REG_LR_RXPACKETCNTVALUELSB                   0x17 
#define REG_LR_MODEMSTAT                             0x18 
#define REG_LR_PKTSNRVALUE                           0x19 
#define REG_LR_PKTRSSIVALUE                          0x1A 
#define REG_LR_RSSIVALUE                             0x1B 
#define REG_LR_HOPCHANNEL                            0x1C 
#define REG_LR_MODEMCONFIG1                          0x1D 
#define REG_LR_MODEMCONFIG2                          0x1E 
#define REG_LR_SYMBTIMEOUTLSB                        0x1F 
#define REG_LR_PREAMBLEMSB                           0x20 
#define REG_LR_PREAMBLELSB                           0x21 
#define REG_LR_PAYLOADLENGTH                         0x22 
#define REG_LR_PAYLOADMAXLENGTH                      0x23 
#define REG_LR_HOPPERIOD                             0x24 //跳频设置，FreqHoppingPeriod位表示信道驻留时间
#define REG_LR_FIFORXBYTEADDR                        0x25
#define REG_LR_MODEMCONFIG3                          0x26
 // I/O settings
#define REG_LR_DIOMAPPING1                           0x40
#define REG_LR_DIOMAPPING2                           0x41
 // Version
#define REG_LR_VERSION                               0x42
 // Additional settings
#define REG_LR_PLLHOP                                0x44
#define REG_LR_TCXO                                  0x4B
#define REG_LR_PADAC                                 0x4D
#define REG_LR_FORMERTEMP                            0x5B
#define REG_LR_BITRATEFRAC                           0x5D
#define REG_LR_AGCREF                                0x61
#define REG_LR_AGCTHRESH1                            0x62
#define REG_LR_AGCTHRESH2                            0x63
#define REG_LR_AGCTHRESH3                            0x64

//其他
#define RFLR_MODEMCONFIG2_SF_MASK                    0x0f
#define RFLR_MODEMCONFIG1_CODINGRATE_MASK            0xF1 
#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK          0xFB 
#define RFLR_MODEMCONFIG1_BW_MASK                    0x0F 
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK        0xFE 
#define RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK        0xfc
#define RFLR_MODEMCONFIG3_MOBILE_NODE_MASK           0xF7 

//中断标志位
#define RFLR_IRQFLAGS_TIMEOUT                                 0x80 
#define RFLR_IRQFLAGS_RXDONE                            0x40 
#define RFLR_IRQFLAGS_CRCERR                                0x20 
#define RFLR_IRQFLAGS_VAILDHEARDER                              0x10 
#define RFLR_IRQFLAGS_TXDONE                               0x08 
#define RFLR_IRQFLAGS_CAD                            0x04 
#define RFLR_IRQFLAGS_FHSS                           0x02 
#define RFLR_IRQFLAGS_CADDETECTED                           0x01 

//开启中断值
#define IRQN_TXD_EN                               0xF7
#define IRQN_RXD_EN                               0x9F
#define IRQN_CAD_EN                               0xFA
#define IRQN_SEELP_EN                             0xFF

#define PACKET_MIAX_Value                            0xff

typedef enum 
{
   RM_SLEEP=0,	
   RM_STDBY,
   RM_TX,
   RM_TRANSMIT,
   RM_RF,
   RM_RECEIVER,
   RM_RECV_SINGLE,
   RM_CAD
}RF_OP_MODE;//芯片的模式

typedef enum 
{
   RFM_FSK=0,
   RFM_LORA=0x80 
}RF_FSK_MODE;

static void Sx1276_DelayMs(u32 Ms)
{
	u32 i;
	for(;Ms;Ms--) for(i=10000;i;i--);
}

static void Sx1276_WriteReg(u8 Reg, u8 Val)
{	
	Select_Sx1276() ;
	Sx1276_Spi_Write(WRITEREG|Reg);
	Sx1276_Spi_Write(Val);
	NotSelect_Sx1276();
}

static u8 Sx1276_ReadReg(u8 Reg)
{
	u8 Val;
	
	Select_Sx1276() ;
	Sx1276_Spi_Write(READREG&Reg);
	Val = Sx1276_Spi_Write(0);
	NotSelect_Sx1276();
	
	return Val;
}

static void Sx1276_BulkWriteReg(u8 Reg, u8 *data, u8 num)
{
	u8 num_ctr;
	
	Select_Sx1276();  

	Sx1276_Spi_Write(WRITEREG|Reg); 
	for(num_ctr=0; num_ctr<num; num_ctr++) 
	{
		Sx1276_Spi_Write(data[num_ctr]);
	}
	NotSelect_Sx1276();
}

static void Sx1276_BulkReadReg(u8 Reg, u8 *data, u8 num)
{
	u8 num_ctr;
	
	Select_Sx1276();
	Sx1276_Spi_Write(READREG&Reg);
	for(num_ctr=0;num_ctr<num;num_ctr++)
		data[num_ctr] = Sx1276_Spi_Read();
	NotSelect_Sx1276();
}

#if 1//功能设置函数
static void Sx1276_SetOpMode(RF_OP_MODE Mode)
{
	u8 ModePrev;
	ModePrev=Sx1276_ReadReg(REG_LR_OPMODE);
	ModePrev &=0xf8;
	ModePrev |= (u8)Mode;
	Sx1276_WriteReg(REG_LR_OPMODE, ModePrev);		
}

//调频模式选择
static void Sx1276_SetFskMode(RF_FSK_MODE Mode)
{
	u8 ModePrev;
	ModePrev=Sx1276_ReadReg(REG_LR_OPMODE);
	ModePrev &=0x7F;
	ModePrev |= (u8)Mode;
	Sx1276_WriteReg(REG_LR_OPMODE, ModePrev);		
}

//频率
static void Sx1276_SetRfFreq(void)
{
   Sx1276_WriteReg(REG_LR_FRFMSB,0x6c);
   Sx1276_WriteReg(REG_LR_FRFMID,0x80);
   Sx1276_WriteReg(REG_LR_FRFLSB,0x00);
}

//发射功率
static void Sx1276_SetRfPower(u8 power)//0-7
{
	const u8 PowerMap[8]={0X80,0X80,0X80,0X83,0X86,0x89,0x8c,0x8f};		
   Sx1276_WriteReg(REG_LR_PADAC,0x87);
   Sx1276_WriteReg(REG_LR_PACONFIG,PowerMap[power]);
}

//设置扩频因子，范围7-12，扩频因子越大，传输越远，速率越低
//当sf=6时，必须使用隐式报头
static void Sx1276_SetSpreadingFactor(u8 Factor)//7-12
{
   u8 RECVER_DAT;

   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG2);	  
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG2_SF_MASK) | (Factor<<4);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG2,RECVER_DAT );	 
}

//纠错编码率，有效数据比，数字越大，附带开销越大
static void Sx1276_SetErrorCoding(u8 Value)// 1-4
{	
   u8 RECVER_DAT;
   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG1);
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_CODINGRATE_MASK) | (Value<<1);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG1, RECVER_DAT);
   // LoRaSettings.ErrorCoding = value;
}

//crc校验
static void Sx1276_SetPacketCrcOn(bool Enable)
{	
   u8 RECVER_DAT;
   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG2);
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) | (Enable<<2);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG2,RECVER_DAT);
}

//带宽越大，速率越快，灵敏度越低
// 0:7.8k
// 1:10.4k
// 2:15.6k
// 3:20.8k
// 4:31.25k
// 5:41.7k
// 6:62.5k
// 7:125k
// 8:250k
// 9:500k
static void Sx1276_SetSignalBandwidth(u8 Bandwidth)//0-9
{
   u8 RECVER_DAT;
   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG1);
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_BW_MASK) | (Bandwidth << 4);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG1, RECVER_DAT);
   //LoRaSettings.SignalBw = bw;
}

//是否开启隐式报头，当sf=6时，必须使用隐式报头
//报头包括:有效负载长度+前向纠错码率+是否开启了crc
static void Sx1276_SetImplicitHeaderOn(bool Enable)
{
   u8 RECVER_DAT;
   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG1);
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) | (Enable);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG1, RECVER_DAT );
}

//在单一接收模式下，调制解调器在给定的时间窗口内搜索前导码。如果在该时间窗口结束时还未
//找到前导码，则芯片会产生RxTimeout中断信号并切换回待机模式。时间窗口长度（以符号
//计）由RegSymbTimeout寄存器定义，必须为4（调制解调器获取前导码锁的最短时间）到
//1023个符号。缺省值为5。如果在时间窗口内未发现前导码，则会产生RxTimeout中断信号，
//同时芯片切换回待机模式。
static void Sx1276_SetSymbTimeout(u16 Value)
{
   u8 RECVER_DAT[2];
   RECVER_DAT[0] = Sx1276_ReadReg(REG_LR_MODEMCONFIG2);
   RECVER_DAT[1] = Sx1276_ReadReg(REG_LR_SYMBTIMEOUTLSB);
   RECVER_DAT[0] = (RECVER_DAT[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) | ((Value>>8) &~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK);
   RECVER_DAT[1] = Value & 0xFF;
   Sx1276_WriteReg( REG_LR_MODEMCONFIG2,RECVER_DAT[0]);
   Sx1276_WriteReg( REG_LR_SYMBTIMEOUTLSB,RECVER_DAT[1]);
}

//负载长度
static void Sx1276_SetPayloadLength(u8 Value)
{
   Sx1276_WriteReg(REG_LR_PAYLOADLENGTH,Value);
}

//前导码长度	，默认取12即可
static void Sx1276_SetPreamLength(u16 Value)
{
   u8 a[2];
   a[0]=(Value&0xff00)>>8;
   a[1]=Value&0xff;
   Sx1276_WriteReg(REG_LR_PREAMBLEMSB,a[0]);
   Sx1276_WriteReg(REG_LR_PREAMBLELSB,a[1]);
}

//LowDataRateOptimize，符号长度超过16ms时必须打开
//当扩频因子较高时，数据包的发送时间可能较长，有效数据速率较低，
//可通过LowDataRateOptimize位提高LoRa链路的健壮性。
//当单个符号传输时间超过16毫秒时，必须使用LowDataRateOptimize位
//发射机和接收机的LowDataRateOptimize位设置必须一致
static void Sx1276_LowDataRateOptimize(bool Enable)//默认关闭
{	 
   u8 RECVER_DAT;
   RECVER_DAT = Sx1276_ReadReg(REG_LR_MODEMCONFIG3);
   RECVER_DAT = (RECVER_DAT & RFLR_MODEMCONFIG3_MOBILE_NODE_MASK) | (Enable<<3);
   Sx1276_WriteReg(REG_LR_MODEMCONFIG3,RECVER_DAT);
}

static void Sx1276_SendPacket(u8 *Data,u8 Len)
{
   //lpTypefunc.paSwitchCmdfunc(txOpen);//pa天线转换
   Sx1276_SetOpMode(RM_STDBY);
   Sx1276_WriteReg(REG_LR_HOPPERIOD,0);	//不做频率跳变
   Sx1276_WriteReg(REG_LR_IRQFLAGSMASK,IRQN_TXD_EN);	//打开发送中断
   Sx1276_WriteReg(REG_LR_PAYLOADLENGTH,Len);	 //最大数据包
   Sx1276_WriteReg(REG_LR_FIFOTXBASEADDR,0);
   Sx1276_WriteReg(REG_LR_FIFOADDRPTR,0);
   Sx1276_BulkWriteReg(0,Data,Len);
   Sx1276_WriteReg(REG_LR_DIOMAPPING1,0x01);
   Sx1276_WriteReg(REG_LR_DIOMAPPING2,0x00);
   Sx1276_SetOpMode(RM_TRANSMIT);
}

static void Sx1276_IntoRecvMode(void)
{
   Sx1276_SetOpMode(RM_STDBY);
   Sx1276_WriteReg(REG_LR_IRQFLAGSMASK,IRQN_RXD_EN);  //打开中断
   Sx1276_WriteReg(REG_LR_HOPPERIOD,PACKET_MIAX_Value);
   Sx1276_WriteReg(REG_LR_DIOMAPPING1,0X00);
   Sx1276_WriteReg(REG_LR_DIOMAPPING2,0X00);	
   Sx1276_SetOpMode(RM_RECEIVER);
   //lpTypefunc.paSwitchCmdfunc(rxOpen);//pa天线转换
}
	
static void Sx1276_IntoCadMode(void)
{
   Sx1276_SetOpMode(RM_STDBY);
   Sx1276_WriteReg(REG_LR_IRQFLAGSMASK, IRQN_CAD_EN);	//打开中断
   Sx1276_WriteReg(REG_LR_DIOMAPPING1,0X80);
   Sx1276_WriteReg(REG_LR_DIOMAPPING2,0X00);	
   Sx1276_SetOpMode(RM_CAD);
   //lpTypefunc.paSwitchCmdfunc(rxOpen);//pa天线转换
}
	
static void Sx1276_IntoSleepMode(void)
{
   Sx1276_SetOpMode(RM_STDBY);
   Sx1276_WriteReg(REG_LR_IRQFLAGSMASK, IRQN_SEELP_EN);  //打开中断
   Sx1276_WriteReg(REG_LR_DIOMAPPING1,0X00);
   Sx1276_WriteReg(REG_LR_DIOMAPPING2,0X00);	
   Sx1276_SetOpMode(RM_SLEEP);
}
#endif



void Sx1276_Deinit(void)
{

}

void Sx1276_Init(u32 MyAddr,u8 RssiThred)
{
	u8 a,b;
	u32 BusyCnt=0;
	
	//总线初始化
	Sx1276_Spi_Init();

	gMyRxAddr=MyAddr;

	//硬件复位
	Sx1276_DelayMs(100);
	WRESET_0();Sx1276_DelayMs(50);
	WRESET_1();Sx1276_DelayMs(100);//延时最少15ms

	//状态监测
	a=Sx1276_ReadReg(REG_LR_VERSION);
	b=Sx1276_ReadReg(REG_LR_PLLHOP);
	Debug("Sx1276:0x%02x,0x%02x\n\r",a,b);
	if(a!=0x12||b!=0x2D)
	{
		Debug("Sx1276 Init Error!\r\n");
		return;
		//while(1);
	}

	//开始初始化序列
   Sx1276_SetOpMode(RM_SLEEP);  //设置睡眠模式
   Sx1276_SetFskMode(RFM_LORA);	     //设置扩频模式
   Sx1276_SetOpMode(RM_STDBY);  //设置为普通模式
   Sx1276_WriteReg(REG_LR_DIOMAPPING1,0);
   Sx1276_WriteReg(REG_LR_DIOMAPPING2,0);
   Sx1276_SetRfFreq();//设置频率
   Sx1276_SetRfPower(7);//设置功耗
   
   Sx1276_SetSignalBandwidth(8);	 //设置扩频带宽
   Sx1276_SetSpreadingFactor(7);	 //扩频因子设置
   Sx1276_SetErrorCoding(2);		 //有效数据比
   
   Sx1276_SetImplicitHeaderOn(FALSE);//同步头是显性模式
   Sx1276_SetPayloadLength(64);
   Sx1276_SetPacketCrcOn(TRUE);//CRC校验打开
    
   Sx1276_IntoRecvMode();//连续接收模式



#if 0//test
	Sx1276_SendPacket();


#endif









	
}

void Sx1276_SetChannel(u16 Channel)
{

}

u8 Sx1276_GetRssi(void)
{
	return 0;
}

void Sx1276_SetRssiThred(u8 RssiThred)
{

}

u8 Sx1276_GetRssiThred(void)
{
	return 0;
}

bool Sx1276_TxPacket(u32 TxAddr,u8 *Buf,u8 Len)
{
	return 0;
}

void Sx1276_SetTxPower(u8 Val)
{

}

void Sx1276_TestPacket(void)
{
	Sx1276_SendPacket("1234567890",10);
}

bool Sx1276_ChannelBusy(void)
{
	return 0;
}

u8 gRxBuf[64];
void Sx1276_ISR(void)
{
	u8 Flag=Sx1276_ReadReg(REG_LR_IRQFLAGS);
	u8 RxLen=0;
	u8 Crc;
	
	Debug("ISR:0x%x, 0x%x\n\r",Flag,Sx1276_ReadReg(REG_LR_MODEMCONFIG2));

	if(Flag&RFLR_IRQFLAGS_RXDONE)//RxDone
	{
		Crc=Sx1276_ReadReg(REG_LR_MODEMCONFIG2);
		if(Crc&0x04)//crc is enable
		{
			Sx1276_WriteReg(REG_LR_FIFOADDRPTR,0x00);
			RxLen = Sx1276_ReadReg(REG_LR_NBRXBYTES);//rece length

			MemSet(gRxBuf,0,sizeof(gRxBuf));
			Sx1276_BulkReadReg(0,gRxBuf,RxLen);
			DisplayBuf(gRxBuf,RxLen,16);
		}
		
		Sx1276_IntoRecvMode();//重新进入rx模式
	}
	else if(Flag&RFLR_IRQFLAGS_CRCERR)//crc error
	{

	}
	else if(Flag&RFLR_IRQFLAGS_TXDONE)//TxDone
	{
		Sx1276_IntoRecvMode();//重新进入rx模式
	}
	else if(Flag&RFLR_IRQFLAGS_CAD)//CAD Done
	{  
		if((Flag&0x01)==0x01)//CAD Detected
		{     //表示CAD 检测到扩频信号 模块进入了接收状态来接收数据
			Sx1276_IntoRecvMode();//重新进入rx模式
		}
		else//没检测到
		{                          
			Sx1276_IntoSleepMode();
		}
	}
	else
	{
		Sx1276_IntoRecvMode();//重新进入rx模式
	}

	Sx1276_WriteReg(REG_LR_IRQFLAGS,0xff);	
}


void Sx1276_IntoOokMode(void)
{

}

void Sx1276_LeaveOokMode(void)
{

}
















