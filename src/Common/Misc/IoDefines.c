#include "Drivers.h"
#include "IoMap.c"

const IO_IN_HAL_DEFINE *gpIoInDefs=NULL;
const IO_OUT_HAL_DEFINE *gpIoOutDefs=NULL;

static const IO_IN_HAL_DEFINE gIoInDefs[IOIN_MAX]={
{IOIN_PIO0,		GPI_A,  GPin0, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI0_IRQn,EXTI_Pio_Priority},
{IOIN_PIO1,		GPI_A,  GPin1, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO2,		GPI_A,  GPin2, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO3,		GPI_A,  GPin3, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO4,		GPI_A,  GPin4, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO5,		GPI_A,  GPin5, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO6,		GPI_A,  GPin6, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO7,		GPI_A,  GPin7, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_IR_IN,		GPI_B,  GPin3, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI3_IRQn,EXTI_Pio_Priority},

};

static const IO_OUT_HAL_DEFINE gIoOutDefs[IOOUT_MAX]={
{IOOUT_LED1,			GPI_B,	GPin8,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_LED2,			GPI_B,	GPin9,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_FLASH_CS,	GPI_A,	GPin8,		GPIO_Mode_Out_PP,TRUE},//flash cs
{IOOUT_FLASH_WP,	GPI_B,	GPin12,	GPIO_Mode_Out_PP,FALSE},//flash wp
{IOOUT_IR_OUT,		GPI_B,	GPin4,		GPIO_Mode_Out_PP,FALSE},//ir out

};

static void HwChoice(void)
{
	gpIoInDefs=gIoInDefs;
	gpIoOutDefs=gIoOutDefs;		
}

static void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u8 i;

	if(gpIoInDefs==NULL|| gpIoOutDefs==NULL) return;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//初始化
	for(i=0;i<IOIN_MAX;i++)
	{
		if(gpIoInDefs[i].Idx>=IOIN_MAX) break;
		if(gpIoInDefs[i].Idx!=i) {Debug("IOIN Config Failed %d!\n\r",i);while(1);}
		RCC_APB2PeriphClockCmd(gGroupMap[gpIoInDefs[i].Group].RccId,ENABLE);
		GPIO_InitStructure.GPIO_Pin = gPinMap[gpIoInDefs[i].Pin].GpioPin;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = gpIoInDefs[i].GpioMode;
		GPIO_Init(gGroupMap[gpIoInDefs[i].Group].GpioGroup, &GPIO_InitStructure);
		//if(gpIoInDefs[i].GpioMode == GPIO_Mode_IPD) GPIO_ResetBits(gpIoInDefs[i].GpioGroup,gpIoInDefs[i].GpioPin);
		//else if(gpIoInDefs[i].GpioMode == GPIO_Mode_IPU) GPIO_SetBits(gpIoInDefs[i].GpioGroup,gpIoInDefs[i].GpioPin);
	}

	for(i=0;i<IOOUT_MAX;i++)
	{
		if(gpIoOutDefs[i].Idx>=IOOUT_MAX) break;
		if(gpIoOutDefs[i].Idx!=i) {Debug("IOOUT Config Failed %d!\n\r",i);while(1);}
		RCC_APB2PeriphClockCmd(gGroupMap[gpIoOutDefs[i].Group].RccId,ENABLE);
		GPIO_InitStructure.GPIO_Pin =gPinMap[gpIoOutDefs[i].Pin].GpioPin;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = gpIoOutDefs[i].GpioMode;
		GPIO_Init(gGroupMap[gpIoOutDefs[i].Group].GpioGroup, &GPIO_InitStructure);
		if(gpIoOutDefs[i].InitSet) GPIO_SetBits(gGroupMap[gpIoOutDefs[i].Group].GpioGroup,gPinMap[gpIoOutDefs[i].Pin].GpioPin);
		else  GPIO_ResetBits(gGroupMap[gpIoOutDefs[i].Group].GpioGroup,gPinMap[gpIoOutDefs[i].Pin].GpioPin);
	}	
}

static void EXTI_Config(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;  
	u8 i;

	if(gpIoInDefs==NULL|| gpIoOutDefs==NULL) return;
	
	for(i=0;i<IOIN_MAX;i++)
	{
		if(gpIoInDefs[i].Idx>=IOIN_MAX) break;
		if(gpIoInDefs[i].NvicIRQChannel)//有共用一个中断的，比如9_5,15_10,通过NvicIRQChannel来区分
		{
			GPIO_EXTILineConfig(gGroupMap[gpIoInDefs[i].Group].GpioPortSource, gPinMap[gpIoInDefs[i].Pin].GpioPinSource); 

			EXTI_InitStructure.EXTI_Line = gPinMap[gpIoInDefs[i].Pin].ExtiLine;             //外部中断线
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断模式
			EXTI_InitStructure.EXTI_Trigger = gpIoInDefs[i].ExtiTrigger;//中断触发方式
			EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //打开中断
			EXTI_Init(&EXTI_InitStructure);    //调用库函数给寄存器复制
			//EXTI_ClearITPendingBit(gPinMap[gpIoInDefs[i].Pin].ExtiLine);

			IOIN_CloseExti(gpIoInDefs[i].Idx);//默认关闭中断
		}
	}
}

//配置矢量中断，矢量的意思就是有顺序，有先后的意思。
static void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;	//定义数据结构体
	u8 i;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置优先级配置的模式

	if(gpIoInDefs==NULL|| gpIoOutDefs==NULL) return;
	
	for(i=0;i<IOIN_MAX;i++)
	{
		if(gpIoInDefs[i].Idx>=IOIN_MAX) break;
		if(gpIoInDefs[i].NvicIRQPriority)
		{
			NVIC_InitStructure.NVIC_IRQChannel = gpIoInDefs[i].NvicIRQChannel;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = gpIoInDefs[i].NvicIRQPriority;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			NVIC_Init(&NVIC_InitStructure);
		}
	}
}

void IoDefinesInit(void)
{
	HwChoice();
	GPIO_Config();	
	EXTI_Config();
	NVIC_Config();
}

void IOOUT_SetIoStatus(IO_OUT_DEFS Io,bool IsHigh)
{
	if(IsHigh)
		GPIO_SetBits(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
	else
		GPIO_ResetBits(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
}

u8 IOOUT_ReadIoStatus(IO_OUT_DEFS Io)
{
	return GPIO_ReadOutputDataBit(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
}

void IOIN_SetIoStatus(IO_IN_DEFS Io,bool IsHigh)
{
	if(IsHigh)
		GPIO_SetBits(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
	else
		GPIO_ResetBits(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
}

u8 IOIN_ReadIoStatus(IO_IN_DEFS Io)
{
	return GPIO_ReadInputDataBit(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
}

void IOIN_OpenExti(IO_IN_DEFS Io)
{
	EXTI->IMR |= gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}

void IOIN_CloseExti(IO_IN_DEFS Io)
{
	EXTI->IMR &=~gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}

u16 IOIN_ReadExti(IO_IN_DEFS Io) 
{
	return EXTI->IMR & gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}


