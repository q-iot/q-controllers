//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了adc驱动，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//

#include "Drivers.h" 

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define ADC_SAMPLE_NUM (16+2)
#define ADC_SAMPLE_OFFSET 4 //根据ADC_SAMPLE_NUM-2得来

static volatile u16 gAdcChanVals[6*ADC_SAMPLE_NUM]; //用来存放ADC转换结果，也是DMA的目标地址
static u8 gAdcChanNum=0;

#define WAVER_AIO_NUM 4
static u8 gAdcIoChMap[WAVER_AIO_NUM+2]={0,0,0,0};

static void Adc1_DmaConfig(volatile u16 *pBuf,u8 SampleNum,u8 ChanNum)
{
    DMA_InitTypeDef DMA;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);    
    DMA_DeInit(DMA1_Channel1);
    
    DMA.DMA_PeripheralBaseAddr  = ADC1_DR_Address;     //DMA对应的外设基地址
    DMA.DMA_MemoryBaseAddr      = (u32)pBuf;       //内存存储基地址
    DMA.DMA_DIR                 = DMA_DIR_PeripheralSRC;//DMA的转换模式为SRC模式，由外设搬移到内存
    DMA.DMA_M2M                 = DMA_M2M_Disable;      //M2M模式禁用
    DMA.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;//定义外设数据宽度为16位
    DMA.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;  //DMA搬移数据尺寸，HalfWord就是为16位    
    DMA.DMA_BufferSize          = SampleNum*ChanNum;                       //DMA缓存大小CH_NUM个
    DMA.DMA_MemoryInc           = DMA_MemoryInc_Enable;         //接收一次数据后，目标内存地址后移
    DMA.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;    //接收一次数据后，设备地址禁止后移
    DMA.DMA_Mode                = DMA_Mode_Circular;            //转换模式，循环缓存模式。
    DMA.DMA_Priority            = DMA_Priority_High;            //DMA优先级高
    DMA_Init(DMA1_Channel1,&DMA); 
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

//按bit设置adc
void Adc1_Rand_Init(u8 UseAio)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;  
	u8 UseAdcIoNum=0;

	if(UseAio)
	{
		Debug("ADC Init:");
		if(ReadBit(UseAio,0)) Debug("1 ");
		if(ReadBit(UseAio,1)) Debug("2 ");
		if(ReadBit(UseAio,2)) Debug("3 ");
		if(ReadBit(UseAio,3)) Debug("4 ");
		Debug("\n\r");
	}
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_ADC1, ENABLE);

	if(ReadBit(UseAio,0))
	{
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);
	    UseAdcIoNum++;
	}

	if(ReadBit(UseAio,1))
	{
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);
	    UseAdcIoNum++;
	}

	if(ReadBit(UseAio,2))
	{
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);
	    UseAdcIoNum++;
	}

	if(ReadBit(UseAio,3))
	{
	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);
	    UseAdcIoNum++;
	}	

	//PB0设置为模拟通道8                   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	gAdcChanNum=2;//默认加了pb0和cpu temp
	
	Adc1_DmaConfig(gAdcChanVals,ADC_SAMPLE_NUM,gAdcChanNum+UseAdcIoNum);//初始化DMA
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	ADC_DeInit(ADC1);
 
    //以下是ADC1的寄存器配置
  	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//AD模式选为独立模式
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//自动扫描模式使能
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//连续转换模式使能
  	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//没有中断触发转换
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//数据向右对齐
  	ADC_InitStructure.ADC_NbrOfChannel = gAdcChanNum+UseAdcIoNum;//初始化ADC通道号数1
  	ADC_Init(ADC1, &ADC_InitStructure);//构建ADC1设备

  	ADC_TempSensorVrefintCmd(ENABLE); //开启内部温度传感器

  	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);  	//PB0对应ADC1通道是8
  	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);	//Vref 1.2v 对应ADC1通道是17
	if(ReadBit(UseAio,0)) 
	{
	    gAdcChanNum++;
		gAdcIoChMap[0]=gAdcChanNum;
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0,gAdcChanNum, ADC_SampleTime_239Cycles5);  	//PA0对应ADC1通道是0
	}
	if(ReadBit(UseAio,1))
	{
		gAdcChanNum++;
		gAdcIoChMap[1]=gAdcChanNum;
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1,gAdcChanNum, ADC_SampleTime_239Cycles5);  	//PA1对应ADC1通道是1
	}
	if(ReadBit(UseAio,2))
	{
		gAdcChanNum++;
		gAdcIoChMap[2]=gAdcChanNum;
		ADC_RegularChannelConfig(ADC1, ADC_Channel_2,gAdcChanNum, ADC_SampleTime_239Cycles5);  	//PA2对应ADC1通道是2
	}
	if(ReadBit(UseAio,3))
	{
		gAdcChanNum++;
		gAdcIoChMap[3]=gAdcChanNum;
		ADC_RegularChannelConfig(ADC1, ADC_Channel_3,gAdcChanNum, ADC_SampleTime_239Cycles5);  	//PA3对应ADC1通道是3
	}
	
	ADC_DMACmd(ADC1,ENABLE);//开启adc的DMA
  	ADC_Cmd(ADC1, ENABLE);//使能ADC1  	
  	
  	ADC_ResetCalibration(ADC1);//复位ADC1的寄存器      	
  	while(ADC_GetResetCalibrationStatus(ADC1));//等待复位结束 
  	
  	ADC_StartCalibration(ADC1);//开始ADC1校准  	
  	while(ADC_GetCalibrationStatus(ADC1));//等待ADC1校准结束 	

  	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能ADC1转换  	
}

//从dma中读取channal的采样值
//channal从1开始，channal序号根据初始化顺序来，默认pb0
u16 Adc1_GetVal(u8 Chan)
{
	u32 Sum=0;
	u16 Min=0xffff;
	u16 Max=0;
	u16 Val,i;

	if(Chan==0 || Chan>gAdcChanNum) return 0;

	Chan--;

	for(i=0;i<ADC_SAMPLE_NUM;i++)
	{
		Val=gAdcChanVals[i*gAdcChanNum+Chan];
		
		//Debug("Adc %u\n\r",Val);
		
		Sum+=Val;
		if(Val<Min) Min=Val;
		if(Val>Max) Max=Val;
	}	
	//Debug("\n\r");
	
	Sum-=Min;
	Sum-=Max;

	return Sum>>ADC_SAMPLE_OFFSET;//根据ADC_SAMPLE_NUM-2得来
}

//AioID 0-3
u16 Adc1_GetValByAio(u8 AioID)
{
	if(AioID>=WAVER_AIO_NUM) return 0;
	return Adc1_GetVal(gAdcIoChMap[AioID]);
}

//从channal 1读取数据，channal序号根据初始化顺序来，默认pb0
u32 GetAdcRand(void)
{
	u32 Num=0;
	u8 i;
	
	for(i=0;i<8;i++)
	{
		Num|=((gAdcChanVals[i*gAdcChanNum]&0x0f)<<(i*4));
	}
	
	return Num;
}

//输入0-4096的采样值
//输出以0.01摄氏度为单位的温度值
static u16 Vat2Temp(u16 Vat)
{
	u32 Temp=Vat;
	
	// (14300-val*33000/4096)/43 *100 +2500
	Temp*=33000;
	Temp/=4096;
	Temp=14300-Temp;
	Temp*=100;
	Temp/=43;
	Temp+=2500;

	return Temp;
}

//输出以0.01摄氏度为单位的温度值
u16 GetCpuTemp(void)
{
	return Vat2Temp(Adc1_GetVal(ADC_CPU_CH));
}





