#include "Drivers.h"
#include "Si4432.h"
//#include "SecretRun.h"

#define RSSI_DEBUG_MODE 0//信号检测模式，开启后，设备将不会正常工作

static u32 gMySiRxAddr=0;

#define Select_Si4432()     IOOUT_SetIoStatus(IOOUT_WRF_DRV_CS, FALSE)
#define NotSelect_Si4432()    IOOUT_SetIoStatus(IOOUT_WRF_DRV_CS, TRUE)
#if PRODUCT_IS_JUMPER || PRODUCT_IS_LIFE1
#define Si4432_Spi_Init() SPI1_Init()
#define Si4432_Spi_Read() SPI_ReadByte(SPI1)
#define	Si4432_Spi_Write(x) SPI_WriteByte(SPI1,x)
#elif PRODUCT_IS_WAVER
#define Si4432_Spi_Init() SPI2_Init()
#define Si4432_Spi_Read() SPI_ReadByte(SPI2)
#define	Si4432_Spi_Write(x) SPI_WriteByte(SPI2,x)
#endif
#define ANT_TX1_RX0()	Si4432_WriteReg(R_IO_CONF,0x01)		// 发射状态的天线开关定义
#define ANT_TX0_RX1()	Si4432_WriteReg(R_IO_CONF,0x02)		// 接收状态的天线开关定义
#define ANT_TX0_RX0()	Si4432_WriteReg(R_IO_CONF,0x00)         // 非发射，接收状态的天线开关定义
#define WSDN_0()  IOOUT_SetIoStatus(IOOUT_WRF_DRV_SDN, FALSE)
#define WSDN_1()  IOOUT_SetIoStatus(IOOUT_WRF_DRV_SDN, TRUE)
#define NIRQ() 	IOIN_ReadIoStatus(IOIN_WRF_DRV_INT)
#define READREG        0x7f  	//读寄存器指令
#define WRITEREG       0x80 	//写寄存器指令

//寄存器及功能位
#define R_DEVICE_TYPE 0x00

#define R_VERSION 0x01

#define R_STAUS 0x02
#define B_STAUS_FFOVFL 0x80 //fifo上溢
#define B_STAUS_FFUNFL 0x40 //fifo下溢
#define B_STAUS_RXFFEM 0x20 //rx fifo空
#define B_STAUS_HEADERR 0x10 //帧头错误
#define B_STAUS_FREQERR 0x08 //频率错误
#define B_STAUS_LOCKDET 0x04 //合成器锁定侦测
#define B_STAUS_CPS 0x03 //芯片电源状态
#define V_STAUS_CPS_SUSPEND 0x00
#define V_STAUS_CPS_RECV 0x01
#define V_STATUS_CPS_SEND 0x02

#define R_IT_STAUS_1 0x03
#define B_IT1_IFFERR 0x80	//fifo上溢下溢
#define B_IT1_ITXFFAFULL 0x40 //tx fifo 几乎满
#define B_IT1_ITXFFAEM 0x20 //tx fifo 几乎空
#define B_IT1_IRXFFAFULL 0x10 //rx fifo 几乎满
#define B_IT1_IEXT 0x08 //外部中断
#define B_IT1_IPKSENT 0x04 //包发射中断
#define B_IT1_IPKVALID 0x02 //包接收中断
#define B_IT1_ICRCERROR 0x01 //crc错误

#define R_IT_STAUS_2 0x04
#define B_IT2_ISWDET 0x80 //侦测到同步字
#define B_IT2_IPREAVAL 0x40 //侦测到有效引导码
#define B_IT2_IPREAINVAL 0x20 //侦测到无效引导码
#define B_IT2_IRSSI 0x10 //信号等级超过门槛
#define B_IT2_IWUT 0x08 //定时唤醒
#define B_IT2_ILDB 0x04 //电池欠压
#define B_IT2_ICHIPRDY 0x02 //芯片预备
#define B_IT2_IPOR 0x01 //上电复位

#define R_IT_ENABLE_1 0x05 //中断使能1
#define R_IT_ENABLE_2 0x06 //中断使能2

#define R_RUN_MODE_1 0x07 //运行模式控制1
#define B_MODE_SWRET 0x80
#define B_MODE_READY 0x01
#define B_MODE_RECV 0x05
#define B_MODE_SEND 0x09

#define R_RUN_MODE_2 0x08 //运行模式控制2
#define B_RM2_CLEAN_RX_FIFO 0x02
#define B_RM2_CLENA_TX_FIFO 0x01

#define R_XTAL_P 0x09 //30mhz晶振负载电容
#define R_MCU_OUT_CLOCK 0x0a//处理器输出时钟
#define R_GPIO0_CONF 0x0B //gpio配置0
#define R_GPIO1_CONF 0x0C //gpio配置1
#define R_GPIO2_CONF 0x0D //gpio配置2
#define R_IO_CONF 0x0E //io端口配置
#define R_ADC_CONF 0x0F //adc配置
#define R_TEMP_SEN_OFFSET 0x12 //温度传感器校正
#define R_WUT_1 0x14  //唤醒定时器1
#define R_WUT_2 0x15  //唤醒定时器2
#define R_WUT_3 0x16  //唤醒定时器3
#define R_WUT_VAL_1 0x17  //唤醒定时器值1
#define R_WUT_VAL_2 0x18  //唤醒定时器值2
#define R_BAT_THRESHOLD 0x1A  //电池欠压检测门槛
#define R_BAT_LEVEL 0x1B  //电池电压级别
#define R_FILTER_BAUD_WITDH 0x1C  //滤波器带宽
#define R_AFC_LOOP_GEAR_OVER 0x1D //afc
#define R_RSSI 0x26  //信号指示器
#define R_RSSITH 0x27  //信号门限

#define R_DATA_RW_CTRL 0x30  //数据存取控制
#define B_DRC_ENPACRX  0x80 //使能接收数据包处理
#define B_DRC_LSBFRST 0x40 //LSB优先
#define B_DRC_CRCDONLY 0x20 //仅仅对数据进行CRC校验
#define B_DRC_ENPACTX 0x80 //使能数据包发射处理
#define B_DRC_ENCRC 0x40 //CRC使能
#define B_DRC_CRC 0x03 //CRC类型选择
#define V_DRC_CRC_16 0x01 //CRC16校验

#define R_FHC_1 0x32 //帧头控制1
#define B_FHC_BOARDCAST_EN 0xf0 //广播控制位
#define V_FHC_NO_BOARDCAST 0x00 //无字节用于广播
#define V_FHC_BYTE_BOARDCAST 0x01 // 1字节用于广播校验
#define B_FHC_BYTE_NUM 0x0f 	//字节数控制位
#define V_FHC_NO_BYTE_CHK 0x00 //无字节用于帧头
#define V_FHC_BYTE_CHK 0x01 // 1字节用于帧头

#define R_FHC_2 0x33 //帧头控制2
#define B_FHC_HDLEN 0x70//发射帧头长度
#define B_FHC_FIXPKLEN 0x08//固定帧头长度是否包含数据包长度
#define B_FHC_SYNCLEN 0x06//同步字长度
#define B_FHC_PREALEN 0x01//前导码使能

#define R_PREALEN_LEN 0x34 //前导码长度

#define R_PREATH_CTRL1 0x35 //前导码侦测控制1
#define B_PREATH_BYTH_HALF 0xf8 //在侦测期间处理半字节数

#define R_SYNC_CHAR_3 0x36 //同步字3
#define R_SYNC_CHAR_2 0x37 //同步字2
#define R_SYNC_CHAR_1 0x38 //同步字1
#define R_SYNC_CHAR_0 0x39 //同步字0
#define R_TX_FH3 0x3a //发射帧头3
#define R_TX_FH2 0x3b //发射帧头2
#define R_TX_FH1 0x3c //发射帧头1
#define R_TX_FH0 0x3d //发射帧头0
#define R_TX_PKTLEN 0x3e //发射数据包长度
#define R_FH_CHK3 0x3f //接收侦头3检测
#define R_FH_CHK2 0x40 //接收侦头2检测
#define R_FH_CHK1 0x41 //接收侦头1检测
#define R_FH_CHK0 0x42 //接收侦头0检测
#define R_FH_EN3 0x43 //帧头使能3
#define R_FH_EN2 0x44 //帧头使能2
#define R_FH_EN1 0x45 //帧头使能1
#define R_FH_EN0 0x46 //帧头使能0
#define R_FH_RX3 0x47 //接收到的帧头3
#define R_FH_RX2 0x48 //接收到的帧头2
#define R_FH_RX1 0x49 //接收到的帧头1
#define R_FH_RX0 0x4a //接收到的帧头0
#define R_RX_PKTLEN 0x4b //接收到的数据包长度
#define R_TX_POWER 0x6d //发射功率
#define R_TX_BAUD1 0x6e//发射波特率1
#define R_TX_BAUD0 0x6f//发射波特率0
#define R_MODULATION_CTRL1 0x70//调制控制模式1
#define R_MODULATION_CTRL2 0x71//调制控制模式2
#define R_FREQ_DEVIATATION 0x72//频率偏差
#define R_TX_FIFO_CTRL1 0x7c //tx fifo控制，tx几乎满门限
#define R_TX_FIFO_CTRL2 0x7d//tx fifo控制，tx几乎空门限
#define R_RX_FIFO_CTRL 0x7e//rx fifo 几乎满门限
#define R_FIFO 0x7f //fifo


const u8 GFSKRfSettings_Si443x[][15] ={
//{ IFBW(1C), COSR, CRO2(21), CRO1(22), CRO0(23), CTG1(24), CTG0(25), TDR1(6E), TDR0(6F), MMC1(70), TXFDEV,	RXFDEV 	B_TIME,  AFC(1E), AFC Limiter(2A)	
{0x1D, 0x41, 0x60, 0x27, 0x52, 0x00, 0x04, 0x13, 0xa9, 0x20, 0x3d, 	0x3d,		209,	0x00,	0x21},  	//DR: 2.4kbps, DEV: +-38.4kHz, BBBW: 95.3kHz	
{0x1D, 0xA1, 0x20, 0x4E, 0xA5, 0x00, 0x0C, 0x27, 0x52, 0x20, 0x3D, 	0x3D,		105,	0x00,	0x21},		//DR: 4.8kbps, DEV: +-38.4kHz, BBBW: 95.3kHz 
{0x1E, 0xD0, 0x00, 0x9D, 0x49, 0x00, 0x29, 0x4e, 0xa5, 0x20, 0x3D, 	0x3D,		53,		0x00,	0x21},		//DR: 9.6kbps, DEV: +-38.4kHz, BBBW: 102.2kHz
{0x1E, 0xc8, 0x00, 0xA3, 0xD7, 0x00, 0x2B, 0x51, 0xec, 0x20, 0x40, 	0x40,		50,		0x00,	0x21},		//DR: 10kbps, DEV: +-40kHz, BBBW: 102.2kHz
{0xAC, 0x96, 0x00, 0xDA, 0x74, 0x00, 0xDC, 0xa3, 0xd7, 0x20, 0x20, 	0x20,		25,		0x00,	0x1F},		//DR: 20kbps, DEV: +-20kHz, BBBW: 115.6kHz	
{0x04, 0x64, 0x01, 0x47, 0xae, 0x02, 0x91, 0x0A, 0x3D, 0x00, 0x20, 	0x20,		13,		0x00,	0x22},		//DR: 40kbps, DEV: +-20kHz, BBBW: 95.3kHz
{0x05, 0x50, 0x01, 0x99, 0x9A, 0x03, 0x35, 0x0C, 0xCD, 0x00, 0x28, 	0x28,		10,		0x00,	0x28},		//DR: 50kbps, DEV: +-25kHz, BBBW: 112.8kHz
{0x9A, 0x3C, 0x02, 0x22, 0x22, 0x07, 0xFF, 0x19, 0x9A, 0x00, 0x50, 	0x50,		8, 		0x00,	0x48},  	//DR: 100kbps, DEV: +-50kHz, BBBW: 208 kHz
{0x83, 0x5e, 0x01, 0x5D, 0x86, 0x02, 0xBB, 0x20, 0xc5, 0x00, 0x66, 	0x66,		8,		0x00,	0x50},		//DR: 128kbps, DEV: +-64kHz, BBBW: 269.3kHz
{0x8c, 0x2f, 0x02, 0xbb, 0x0d, 0x05, 0x74, 0x41, 0x89, 0x00, 0xcd, 	0xcd, 		4,		0x00, 	0x50},		//DR: 256kbps, DEV: +-128kHz, BBBW: 518.8kHz
};

volatile u8 gSiRssiThred=WRF_RSSI_THRESHOLD;//rssi阀值，必须在启动时被同步成数据库的值

static void Si4432_DelayMs(u32 Ms)
{
	u32 i;
	for(;Ms;Ms--) for(i=10000;i;i--);
}

static void Si4432_WriteReg(u8 Reg, u8 Val)
{	
	Select_Si4432() ;
	Si4432_Spi_Write(WRITEREG|Reg);
	Si4432_Spi_Write(Val);
	NotSelect_Si4432();
}

static u8 Si4432_ReadReg(u8 Reg)
{
	u8 Val;
	
	Select_Si4432() ;
	Si4432_Spi_Write(READREG&Reg);
	Val = Si4432_Spi_Write(0);
	NotSelect_Si4432();
	
	return Val;
}

static void Si4432_BulkWriteReg(u8 Reg, u8 *data, u8 num)
{
	u8 num_ctr;
	
	Select_Si4432();  

	Si4432_Spi_Write(WRITEREG|Reg); 
	for(num_ctr=0; num_ctr<num; num_ctr++) 
	{
		Si4432_Spi_Write(data[num_ctr]);
	}
	NotSelect_Si4432();
}

static void Si4432_BulkReadReg(u8 Reg, u8 *data, u8 num)
{
	u8 num_ctr;
	
	Select_Si4432();
	Si4432_Spi_Write(READREG&Reg);
	for(num_ctr=0;num_ctr<num;num_ctr++)
		data[num_ctr] = Si4432_Spi_Read();
	NotSelect_Si4432();
}

static __inline void Si4432_ClearNIRQ(void)
{
	Si4432_ReadReg(R_IT_STAUS_1);	 //读状态寄存器，以清RF模块中断	
	Si4432_ReadReg(R_IT_STAUS_2);	//读状态寄存器2，以清RF模块中断	
}

static void Si4432_SetRxMode(u32 RxAddr)
{	
	//Debug("SetRxAddr:0x%x\n\r",RxAddr);
	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_READY);	//进入 Ready 模式
	ANT_TX0_RX1();		//设置天线开关

	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)|B_RM2_CLEAN_RX_FIFO); //接收FIFO清0
	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)&(~B_RM2_CLEAN_RX_FIFO));
	
	Si4432_ClearNIRQ();

	Si4432_WriteReg(R_FH_CHK0,(RxAddr>>24)&0xff);//需要校验的头码,用于地址验证
	Si4432_WriteReg(R_FH_CHK1,(RxAddr>>16)&0xff);
	Si4432_WriteReg(R_FH_CHK2,(RxAddr>>8)&0xff);
	Si4432_WriteReg(R_FH_CHK3,RxAddr&0xff);   
		
	Si4432_WriteReg(R_IT_ENABLE_1, B_IT1_IPKVALID);  //RF模块收到整包数据后，产生中断
#if RSSI_DEBUG_MODE //信号调试模式
	Si4432_WriteReg(R_IT_ENABLE_2, B_IT2_ISWDET|B_IT2_IRSSI);//开启同步字检测
#else
	Si4432_WriteReg(R_IT_ENABLE_2, 0);//开启同步字检测
#endif

	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_RECV);  //RF模块进入接收模式

#if PRODUCT_IS_WEIBO
	IOIN_OpenExti(IOIN_WEIBO_IN);//开weibo
#endif	
}

static void Si4432_SetTxMode(u32 TxAddr)
{
#if PRODUCT_IS_WEIBO
	IOIN_CloseExti(IOIN_WEIBO_IN);//关weibo
#endif

	//Debug("SetTxAddr:0x%x\n\r",TxAddr);
	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_READY);	//rf模块进入Ready模式
	ANT_TX1_RX0();		//设置天线开关的方向

	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)|B_RM2_CLENA_TX_FIFO); //发射FIFO清0
	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)&(~B_RM2_CLENA_TX_FIFO));
	
	Si4432_ClearNIRQ();

	Si4432_WriteReg(R_TX_FH0,(TxAddr>>24)&0xff); //发射的头码，用于地址校验
	Si4432_WriteReg(R_TX_FH1,(TxAddr>>16)&0xff);
	Si4432_WriteReg(R_TX_FH2,(TxAddr>>8)&0xff);
	Si4432_WriteReg(R_TX_FH3,TxAddr&0xff);  
		
	Si4432_WriteReg(R_IT_ENABLE_1, B_IT1_IPKSENT);	//整包数据发射完后，产生中断
	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_SEND); //进入发射模式
}

void Si4432_SetTxPower(u8 Val)
{
	Si4432_WriteReg(R_TX_POWER,Val&0x07);
}

//设置不同频段
void Si4432_SetChannel(u16 Channel)
{
	if(Channel>=50) return;//480M以上要偏移

	if(Channel==0)
	{
		Debug("Freq 430.5M\n\r");
		Si4432_WriteReg(0x75, 0x53);  //默认频率设置433.5
		Si4432_WriteReg(0x76, 0x57);  
		Si4432_WriteReg(0x77, 0x80);
	}
	else
	{
		Debug("Freq %uM\n\r",430+Channel);
		Si4432_WriteReg(0x75, 0x53+Channel/10); //从430M开始，每1M一个频段
		Si4432_WriteReg(0x76, 25*(Channel%10));
		Si4432_WriteReg(0x77, 0x00);
		//Debug("75H:0x%x, 76H:%x\n\r",0x53+Channel/10,25*(Channel%10));
	}
}

void Si4432_Deinit(void)
{
	WSDN_1();
}

//初始化时寄存器配置
static void Si4432_RegConfig(void)
{
	//下面根据silabs的excel
	Si4432_WriteReg(R_MODULATION_CTRL1, 0x2c);  //write 0x2C to the Modulation Mode Control 1 register 
	Si4432_WriteReg(R_AFC_LOOP_GEAR_OVER, 0x40);  //使能 afc

	//set the desired TX data rate (9.6kbps)
	Si4432_WriteReg(0x1c, 0x1B);														//write 0x05 to the IF Filter Bandwidth register		
	Si4432_WriteReg(0x20, 0xD0);														//write 0xA1 to the Clock Recovery Oversampling Ratio register		
	Si4432_WriteReg(0x21, 0x00);														//write 0x20 to the Clock Recovery Offset 2 register		
	Si4432_WriteReg(0x22, 0x9d);														//write 0x4E to the Clock Recovery Offset 1 register		
	Si4432_WriteReg(0x23, 0x49);														//write 0xA5 to the Clock Recovery Offset 0 register		
	Si4432_WriteReg(0x24, 0x00);														//write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	Si4432_WriteReg(0x25, 0x34);														//write 0x13 to the Clock Recovery Timing Loop Gain 0 register		

	Si4432_WriteReg(R_TX_BAUD1, 0x4E);														//write 0x4E to the TXDataRate 1 register
	Si4432_WriteReg(R_TX_BAUD0, 0xA5);														//write 0xA5 to the TXDataRate 0 register

	//设置数据包相关功能
	//以下代码实现:8字节引导码+2字节同步+4字节帧头+1字节长度+64字节数据+2字节CRC
	// 4字节帧头中，会存放目的地址
	Si4432_WriteReg(R_DATA_RW_CTRL, 0x8D);  //使能PH+FIFO模式，高位在前面，使能CRC校验
#if RSSI_DEBUG_MODE//一旦开启了调试模式，就不用检测帧头了
	Si4432_WriteReg(R_FHC_1, 0); 
#else
	Si4432_WriteReg(R_FHC_1, 0xff);  //bit[0:3]用来设置需要检测的帧头个数，bit[4:7]用来设置开启广播的帧头个数
#endif
	Si4432_WriteReg(R_FHC_2, 0x42);  //bit[4:7]设置帧头长度为4字节，bit[1:2]取两个同步字，即同步字3,2作为同步字
	Si4432_WriteReg(R_PREALEN_LEN, 16);    //发射16个Nibble的Preamble，即8字节
	Si4432_WriteReg(R_PREATH_CTRL1, 4<<3);  //需要检测4个Nibble的Preamble
	Si4432_WriteReg(R_SYNC_CHAR_3, 'w');  //同步字
	Si4432_WriteReg(R_SYNC_CHAR_2, 'n');	//同步字
	Si4432_WriteReg(R_SYNC_CHAR_1, 'e');
	Si4432_WriteReg(R_SYNC_CHAR_0, 't');

	Si4432_WriteReg(R_FH_EN3, 0xff);  //头码1,2,3,4 的所有位都需要校验
	Si4432_WriteReg(R_FH_EN2, 0xff);  // 
	Si4432_WriteReg(R_FH_EN1, 0xff);  // 
	Si4432_WriteReg(R_FH_EN0, 0xff);  // 

	//设置通信参数
	Si4432_WriteReg(R_TX_POWER, 0x07);  //发射功率设置  0x00:+0dBM  0x01:+3dBM  0x02:+6dBM  0x03:+9dBM  0x04:+11dBM  0x05:+14dBM  0x06:+17dBM  0x07:20dBM
	Si4432_WriteReg(0x79, 0x0);   //不需要跳频
	Si4432_WriteReg(0x7a, 0x0);   //不需要跳频
	Si4432_WriteReg(R_MODULATION_CTRL2, 0x22);  //发射不需要CLK，FiFo，FSK模式
	Si4432_WriteReg(R_FREQ_DEVIATATION, 0x19);  //频偏为30KHz
	Si4432_WriteReg(0x73, 0x0);   //没有频率偏差
	Si4432_WriteReg(0x74, 0x0);   //没有频率偏差
	
	//设置通信频率
	Si4432_SetChannel(0);
}

//初始化时寄存器配置
static void Si4432_RegConfig2(void)
{
	//下面根据silabs的excel
	Si4432_WriteReg(R_MODULATION_CTRL1, 0x2c);  //write 0x2C to the Modulation Mode Control 1 register 
	Si4432_WriteReg(R_AFC_LOOP_GEAR_OVER, 0x40);  //使能 afc

	//set the desired TX data rate (19.2kbps)
	Si4432_WriteReg(0x1c, 0x05);														//write 0x05 to the IF Filter Bandwidth register		
	Si4432_WriteReg(0x20, 0xD0);														//write 0xA1 to the Clock Recovery Oversampling Ratio register		
	Si4432_WriteReg(0x21, 0x00);														//write 0x20 to the Clock Recovery Offset 2 register		
	Si4432_WriteReg(0x22, 0x9d);														//write 0x4E to the Clock Recovery Offset 1 register		
	Si4432_WriteReg(0x23, 0x49);														//write 0xA5 to the Clock Recovery Offset 0 register		
	Si4432_WriteReg(0x24, 0x02);														//write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	Si4432_WriteReg(0x25, 0x78);														//write 0x13 to the Clock Recovery Timing Loop Gain 0 register		

	Si4432_WriteReg(R_TX_BAUD1, 0x9d);														//write 0x4E to the TXDataRate 1 register
	Si4432_WriteReg(R_TX_BAUD0, 0x49);														//write 0xA5 to the TXDataRate 0 register

	//设置数据包相关功能
	//以下代码实现:8字节引导码+2字节同步+4字节帧头+1字节长度+64字节数据+2字节CRC
	// 4字节帧头中，会存放目的地址
	Si4432_WriteReg(R_DATA_RW_CTRL, 0x8D);  //使能PH+FIFO模式，高位在前面，使能CRC校验
#if RSSI_DEBUG_MODE//一旦开启了调试模式，就不用检测帧头了
	Si4432_WriteReg(R_FHC_1, 0); 
#else
	Si4432_WriteReg(R_FHC_1, 0xff);  //bit[0:3]用来设置需要检测的帧头个数，bit[4:7]用来设置开启广播的帧头个数
#endif
	Si4432_WriteReg(R_FHC_2, 0x42);  //bit[4:7]设置帧头长度为4字节，bit[1:2]取两个同步字，即同步字3,2作为同步字
	Si4432_WriteReg(R_PREALEN_LEN, 16);    //发射16个Nibble的Preamble，即8字节
	Si4432_WriteReg(R_PREATH_CTRL1, 4<<3);  //需要检测4个Nibble的Preamble
	Si4432_WriteReg(R_SYNC_CHAR_3, 'w');  //同步字
	Si4432_WriteReg(R_SYNC_CHAR_2, '2');	//同步字
	Si4432_WriteReg(R_SYNC_CHAR_1, 0);
	Si4432_WriteReg(R_SYNC_CHAR_0, 0);

	Si4432_WriteReg(R_FH_EN3, 0xff);  //头码1,2,3,4 的所有位都需要校验
	Si4432_WriteReg(R_FH_EN2, 0xff);  // 
	Si4432_WriteReg(R_FH_EN1, 0xff);  // 
	Si4432_WriteReg(R_FH_EN0, 0xff);  // 

	//设置通信参数
	Si4432_WriteReg(R_TX_POWER, 0x07);  //发射功率设置  0x00:+0dBM  0x01:+3dBM  0x02:+6dBM  0x03:+9dBM  0x04:+11dBM  0x05:+14dBM  0x06:+17dBM  0x07:20dBM
	Si4432_WriteReg(0x79, 0x0);   //不需要跳频
	Si4432_WriteReg(0x7a, 0x0);   //不需要跳频
	Si4432_WriteReg(R_MODULATION_CTRL2, 0x22);  //发射不需要CLK，FiFo，FSK模式
	Si4432_WriteReg(R_FREQ_DEVIATATION, 0x17);  //频偏为4.8KHz
	Si4432_WriteReg(0x73, 0x0);   //没有频率偏差
	Si4432_WriteReg(0x74, 0x0);   //没有频率偏差
	
	//设置通信频率
	Si4432_WriteReg(0x75, 0x53);  //频率设置868
	Si4432_WriteReg(0x76, 0x4b);  //
	Si4432_WriteReg(0x77, 0x00);
}


//软件复位并初始化
void Si4432_Init(u32 MyAddr,u8 RssiThred)
{	
	u8 a,b,c;
	u32 BusyCnt=0;
	
	//总线初始化
	Si4432_Spi_Init();
	
	if(0)//if(CheckSecretKey(BIF_KEY4)==FALSE)
	{
        //此处返回，是不是用户版本没有setkey的情况
        Debug("AKEY EA\n\r");
		return;
	}

	gMySiRxAddr=MyAddr;

	//硬件复位
	Si4432_DelayMs(100);
	WSDN_1();Si4432_DelayMs(50);
	WSDN_0();Si4432_DelayMs(100);//延时最少15ms

	//状态监测
	a=Si4432_ReadReg(0x00);
	b=Si4432_ReadReg(0x01);
	c=Si4432_ReadReg(0x02);
	Debug("Si4432[%02x%02x]:%x\n\r",a,b,c);
	if(a!=0x08||b!=0x06)
	{
		Debug("Si4432 Init Error!\r\n");
		return;
		//while(1);
	}

	Si4432_SetRxMode(gMySiRxAddr);//设置输入状态
	if(NIRQ()==0)
	{
		Debug("Si4432 Status Error!\n\r");
		while(1);
	}
	
	Debug("%d",NIRQ());
		
	//软件复位
	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_SWRET); //write 0x80 to the Operating & Function Control1 register 
	Debug("%d",NIRQ());
	while (NIRQ() == 1)
	{
		Debug("."); //read interrupt status registers to clear  the interrupt flags and release NIRQ()pin 
		if(BusyCnt++>1000) RebootBoard();
	}
	Debug("%d",NIRQ());
	Si4432_ClearNIRQ();
	Debug("\n\r");
	
	//开始设置芯片
	Si4432_WriteReg(R_RUN_MODE_1, B_MODE_READY);  //进入Ready模式
	Si4432_WriteReg(R_IT_ENABLE_2, 0);//关闭不必要的中断
	
	Si4432_WriteReg(R_XTAL_P, 0xf7);  //负载电容,0x7f=12P,0x60=6p，绿色模块7f，蓝色模块f7
	Si4432_WriteReg(R_MCU_OUT_CLOCK, 0x05);  //关闭低频输出
	Si4432_WriteReg(R_GPIO0_CONF, 0xea);  //GPIO 0 当做普通输出口
	Si4432_WriteReg(R_GPIO1_CONF, 0xea);  //GPIO 1 当做普通输出口
	Si4432_WriteReg(R_GPIO2_CONF, 0x1f);  //GPIO 2 

	Si4432_RegConfig();//设置芯片通讯参数
	
	Si4432_SetRssiThred(RssiThred);//过滤掉SI4432_RSSI_THRESHOLD以下的信号
	
	Si4432_SetRxMode(gMySiRxAddr);	
}

//获取信道强度
u8 Si4432_GetRssi(void)
{
	return Si4432_ReadReg(R_RSSI);
}

//开机后一定要运行一次，同步数据库的值到gRssiThred
void Si4432_SetRssiThred(u8 RssiThred)
{	
	gSiRssiThred=RssiThred;
	Si4432_WriteReg(R_RSSITH,gSiRssiThred);
}

u8 Si4432_GetRssiThred(void)
{
	return Si4432_ReadReg(R_RSSITH);
}

//检测信道是否忙
bool Si4432_ChannelBusy(void)
{
	return Si4432_ReadReg(R_RSSI)>gSiRssiThred?TRUE:FALSE;
}

u8 Si4432_RxPacket(u8 *Buf,u32 *pFH)
{	
	u8 Len = Si4432_ReadReg(R_RX_PKTLEN);
 	if(Len)
	{			
	    Si4432_BulkReadReg(R_FIFO, Buf, Len);
		*pFH=(Si4432_ReadReg(R_FH_RX0)<<24)|(Si4432_ReadReg(R_FH_RX1)<<16)|
			(Si4432_ReadReg(R_FH_RX2)<<8)|(Si4432_ReadReg(R_FH_RX3));
		//Debug("R%08x:%d\n\r",*pFH,Len);Debug("Recvd:\n\r");DisplayBuf(Buf,Len,16);
	}
	
	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)|B_RM2_CLEAN_RX_FIFO); //接收FIFO清0
	Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)&(~B_RM2_CLEAN_RX_FIFO));
	Si4432_SetRxMode(gMySiRxAddr);
	  
	return Len;
}

bool Si4432_TxPacket(u32 TxAddr,u8 *Buf,u8 Len)
{
    u32 TX_Timeout;
    u8 ItStatus1,ItStatus2;

	Si4432_SetTxMode(TxAddr);
	
	Si4432_WriteReg(R_TX_PKTLEN,Len); 
	Si4432_BulkWriteReg(R_FIFO,Buf,Len);   
	//Debug("Send:");DisplayBuf(Buf,Len,16);

	IOIN_CloseExti(IOIN_WRF_DRV_INT);//关中断
	TX_Timeout = 0;
	while(NIRQ())		//等待中断
	{
	   TX_Timeout++;
		if(TX_Timeout>=1000)
		{
			IOIN_OpenExti(IOIN_WRF_DRV_INT);//开中断
		    Debug("Tx FAILED\r\n");
			Si4432_Init(gMySiRxAddr,gSiRssiThred);
			return FALSE;//返回错误
		}
		Si4432_DelayMs(1);
	}
	IOIN_OpenExti(IOIN_WRF_DRV_INT);//开中断

	ItStatus1 = Si4432_ReadReg(R_IT_STAUS_1);  //读中断寄存器
	ItStatus2 = Si4432_ReadReg(R_IT_STAUS_2);  //读中断寄存器
	if(ItStatus1&B_IT1_IPKSENT)
	{
		//Debug("Tx Ok %x\r\n",TxAddr); 	
		Si4432_SetRxMode(gMySiRxAddr);
		return TRUE;
	}

	Si4432_SetRxMode(gMySiRxAddr);
	return FALSE;
}

#if 1
#define Si4432_Sdi_Group GPIOB
#define Si4432_Sdi_Pin	GPIO_Pin_5

//通过ook发送数据
//此时mosi口做发射口
void Si4432_IntoOokMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//寄存器设置
	//Si4432_WriteReg(0x1c, 0xba);														//write 0x05 to the IF Filter Bandwidth register		
	//Si4432_WriteReg(0x20, 0x9c);														//write 0xA1 to the Clock Recovery Oversampling Ratio register		
	//Si4432_WriteReg(0x21, 0x00);														//write 0x20 to the Clock Recovery Offset 2 register		
	//Si4432_WriteReg(0x22, 0xd1);														//write 0x4E to the Clock Recovery Offset 1 register		
	//Si4432_WriteReg(0x23, 0xb7);														//write 0xA5 to the Clock Recovery Offset 0 register		
	//Si4432_WriteReg(0x24, 0x00);														//write 0x00 to the Clock Recovery Timing Loop Gain 1 register		
	//Si4432_WriteReg(0x25, 0xd4);														//write 0x13 to the Clock Recovery Timing Loop Gain 0 register		
	Si4432_WriteReg(R_MODULATION_CTRL2, 0x91);  //ook模式

	//频率设置
#if 1	 //433.92
	Si4432_WriteReg(0x75, 0x53);  
	Si4432_WriteReg(0x76, 0x62);  
	Si4432_WriteReg(0x77, 0x00);
#else //315
	Si4432_WriteReg(0x75, 0x47);  
	Si4432_WriteReg(0x76, 0x7d);  
	Si4432_WriteReg(0x77, 0x00);
#endif
	
	Si4432_SetTxMode(gMySiRxAddr);

	GPIO_InitStructure.GPIO_Pin = Si4432_Sdi_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(Si4432_Sdi_Group, &GPIO_InitStructure);
	NotSelect_Si4432();//nSel need high
	
	//SysVars()->RfSendBySi=TRUE;
}

void Si4432_LeaveOokMode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = Si4432_Sdi_Pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(Si4432_Sdi_Group, &GPIO_InitStructure);

	Si4432_RegConfig();
	
	Si4432_SetRxMode(gMySiRxAddr);
	//SysVars()->RfSendBySi=FALSE;
}
#endif

#include "WNetRecvQ.h"
#include "EventInHandler.h"
void Si4432_ISR(void)
{
	u8 ItStatus1,ItStatus2;

	ItStatus1 = Si4432_ReadReg(R_IT_STAUS_1);  //读中断寄存器
	ItStatus2 = Si4432_ReadReg(R_IT_STAUS_2);

	//Debug("It %x %x %d\n\r",ItStatus1,ItStatus2,Si4432_ReadReg(R_RX_PKTLEN));

#if RSSI_DEBUG_MODE //信号调试模式
	//设置了门限，并开了irssi中断后，当超过rssi门限强度，此位会置位
	if(ItStatus2&B_IT2_IRSSI)//信号强度超过门限
	{
		//Debug("IR%d\n\r",Si4432_ReadReg(R_IT_STAUS_2)&B_IT2_IRSSI);
	}
	
	if(ItStatus2&B_IT2_ISWDET)//检测到同步字
	{
		Debug("%5d  | ",WRF_DRV.pWRF_GetRssi());
	}
#endif

	if(ItStatus1&B_IT1_IPKVALID)
	{
		WNET_RECV_BLOCK *pRecv=GetWNetIdleRecvQ();

		if(pRecv!=NULL)
		{
			pRecv->Len=Si4432_RxPacket(pRecv->Data,&pRecv->DstAddr);//读取数据
			SetEventFlag(EBF_WNET_PACKET_IN);
		}
		else
		{
			Debug("Full!\n\r");
		}
	}
}

void Si4432_TestPacket(void)
{

#if 1//send	
	while(1)
	{
		Si4432_TxPacket(0x200027f2,"2234567890",10);
		Si4432_DelayMs(1000);
	}
#else
	u8 RxBuf[WRF_DRV_PKT_LEN+1];
	u32 DstAddr;

	Si4432_SetRxMode(gMySiRxAddr);
	while(1)
	{	
		u8 Len=0;
		u8 Rssi;
		
		//if(gSiIntFlag == FALSE) continue;
		//gSiIntFlag=FALSE;
		while (NIRQ() == 1);
		Len=Si4432_RxPacket(RxBuf,&Rssi,&DstAddr);
		if(Len)
		{
			//IOOUT_SetIoStatus(IOOUT_BEEP,TRUE);
			RxBuf[Len]=0;
			Debug("%d[%d]%s\n\r",Rssi,Len,RxBuf);
			//DisplayBuf(RxBuf,16,16);
			//Si4432_DelayMs(Rssi*5);
			
			Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)|B_RM2_CLEAN_RX_FIFO); //接收FIFO清0
			Si4432_WriteReg(R_RUN_MODE_2, Si4432_ReadReg(R_RUN_MODE_2)&(~B_RM2_CLEAN_RX_FIFO));
			MemSet(RxBuf,0,Len);
			//IOOUT_SetIoStatus(IOOUT_BEEP,FALSE);
		}
		Si4432_SetRxMode(gMySiRxAddr);
	}
#endif
}


