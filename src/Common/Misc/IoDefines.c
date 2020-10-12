//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了一套io预定义库，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//
#include "Drivers.h"
#include "IoMap.c"

const IO_IN_HAL_DEFINE *gpIoInDefs=NULL;
const IO_OUT_HAL_DEFINE *gpIoOutDefs=NULL;

#if PRODUCT_IS_LIFE1
static const IO_IN_HAL_DEFINE gIoInDefs[IOIN_MAX]={
{IOIN_KEY1,		GPI_A,  GPin0, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI0_IRQn,EXTI_Pio_Priority},
{IOIN_KEY2,		GPI_A,  GPin1, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI1_IRQn,EXTI_Pio_Priority},
{IOIN_PIO2,		GPI_A,  GPin2, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO3,		GPI_A,  GPin3, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO4,		GPI_A,  GPin4, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO5,		GPI_A,  GPin5, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO6,		GPI_A,  GPin6, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO7,		GPI_A,  GPin7, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_WRF_DRV_INT,	GPI_B, 	GPin10, GPIO_Mode_IPD,EXTI_Trigger_Falling,EXTI15_10_IRQn,EXTI_Si_Priority},
};

static const IO_OUT_HAL_DEFINE gIoOutDefs[IOOUT_MAX]={
{IOOUT_LED1,			GPI_B,	GPin4,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_LED2,			GPI_B,	GPin3,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_FLASH_CS,	GPI_A,	GPin8,		GPIO_Mode_Out_PP,TRUE},//flash cs
{IOOUT_FLASH_WP,	GPI_B,	GPin12,	GPIO_Mode_Out_PP,FALSE},//flash wp
{IOOUT_WRF_DRV_CS,			GPI_B,	GPin11,	GPIO_Mode_Out_PP,FALSE},//rf cs
{IOOUT_WRF_DRV_RESET,	GPI_B,	GPin0,		GPIO_Mode_Out_PP,FALSE},//rf reset
{IOOUT_WRF_DRV_SDN,		GPI_B,	GPin1,		GPIO_Mode_Out_PP,FALSE},//rf reset
};
#else
static const IO_IN_HAL_DEFINE gIoInDefs[IOIN_MAX]={
{IOIN_PIO0,		GPI_A,  GPin0, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI0_IRQn,EXTI_Pio_Priority},
{IOIN_PIO1,		GPI_A,  GPin1, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO2,		GPI_A,  GPin2, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO3,		GPI_A,  GPin3, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO4,		GPI_A,  GPin4, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO5,		GPI_A,  GPin5, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO6,		GPI_A,  GPin6, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_PIO7,		GPI_A,  GPin7, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,0,0},
{IOIN_IR_IN,		GPI_A,  GPin11, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI15_10_IRQn,EXTI_Pio_Priority},
{IOIN_USER_KEY,		GPI_B,  GPin1, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI1_IRQn,EXTI_Pio_Priority},
};

static const IO_OUT_HAL_DEFINE gIoOutDefs[IOOUT_MAX]={
{IOOUT_LED1,			GPI_B,	GPin8,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_LED2,			GPI_B,	GPin9,		GPIO_Mode_Out_PP,TRUE},
{IOOUT_FLASH_CS,	GPI_A,	GPin8,		GPIO_Mode_Out_PP,TRUE},//flash cs
{IOOUT_FLASH_WP,	GPI_B,	GPin12,	GPIO_Mode_Out_PP,FALSE},//flash wp
{IOOUT_IR_OUT,		GPI_A,	GPin12,		GPIO_Mode_Out_PP,FALSE},//ir out
};
#endif

static void HwChoice(void)
{
	gpIoInDefs=gIoInDefs;
	gpIoOutDefs=gIoOutDefs;		
}

//配置单个io口
void GPIO_ConfigOne(const IO_IN_HAL_DEFINE *pInPin,const IO_OUT_HAL_DEFINE *pOutPin)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if(pInPin==NULL && pOutPin==NULL) return;
	if(pInPin!=NULL && pOutPin!=NULL) return;

	if(pInPin!=NULL)
	{
		RCC_APB2PeriphClockCmd(gGroupMap[pInPin->Group].RccId,ENABLE);
		GPIO_InitStructure.GPIO_Pin = gPinMap[pInPin->Pin].GpioPin;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = pInPin->GpioMode;
		GPIO_Init(gGroupMap[pInPin->Group].GpioGroup, &GPIO_InitStructure);
		//if(pInPin->GpioMode == GPIO_Mode_IPD) GPIO_ResetBits(pInPin->GpioGroup,pInPin->GpioPin);
		//else if(pInPin->GpioMode == GPIO_Mode_IPU) GPIO_SetBits(pInPin->GpioGroup,pInPin->GpioPin);
	}

	if(pOutPin!=NULL)
	{
		RCC_APB2PeriphClockCmd(gGroupMap[pOutPin->Group].RccId,ENABLE);
		GPIO_InitStructure.GPIO_Pin =gPinMap[pOutPin->Pin].GpioPin;
		GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = pOutPin->GpioMode;
		GPIO_Init(gGroupMap[pOutPin->Group].GpioGroup, &GPIO_InitStructure);
		if(pOutPin->InitSet) GPIO_SetBits(gGroupMap[pOutPin->Group].GpioGroup,gPinMap[pOutPin->Pin].GpioPin);
		else  GPIO_ResetBits(gGroupMap[pOutPin->Group].GpioGroup,gPinMap[pOutPin->Pin].GpioPin);
	}	
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
		GPIO_ConfigOne(&gpIoInDefs[i],NULL);
	}

	for(i=0;i<IOOUT_MAX;i++)
	{
		if(gpIoOutDefs[i].Idx>=IOOUT_MAX) break;
		if(gpIoOutDefs[i].Idx!=i) {Debug("IOOUT Config Failed %d!\n\r",i);while(1);}
		GPIO_ConfigOne(NULL,&gpIoOutDefs[i]);
	}	
}

//配置单个io的exti
void EXTI_ConfigOne(const IO_IN_HAL_DEFINE *pInPin)
{
	EXTI_InitTypeDef EXTI_InitStructure;  

	if(pInPin==NULL) return;
	
	if(pInPin->NvicIRQChannel)//有共用一个中断的，比如9_5,15_10,通过NvicIRQChannel来区分
	{
		GPIO_EXTILineConfig(gGroupMap[pInPin->Group].GpioPortSource, gPinMap[pInPin->Pin].GpioPinSource); 

		EXTI_InitStructure.EXTI_Line = gPinMap[pInPin->Pin].ExtiLine;             //外部中断线
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断模式
		EXTI_InitStructure.EXTI_Trigger = pInPin->ExtiTrigger;//中断触发方式
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //打开中断
		EXTI_Init(&EXTI_InitStructure);    //调用库函数给寄存器复制
		//EXTI_ClearITPendingBit(gPinMap[pInPin->ExtiLine);

		IOIN_CloseExti(pInPin->Idx);//默认关闭中断
	}
}

static void EXTI_Config(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;  
	u8 i;

	if(gpIoInDefs==NULL) return;
	
	for(i=0;i<IOIN_MAX;i++)
	{
		if(gpIoInDefs[i].Idx>=IOIN_MAX) break;
		EXTI_ConfigOne(&gpIoInDefs[i]);
	}
}

//配置单个io的nvic
void NVIC_ConfigOne(const IO_IN_HAL_DEFINE *pInPin)
{
	NVIC_InitTypeDef NVIC_InitStructure;	//定义数据结构体

	if(pInPin==NULL) return;
	
	if(pInPin->NvicIRQPriority)
	{
		NVIC_InitStructure.NVIC_IRQChannel = pInPin->NvicIRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pInPin->NvicIRQPriority;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}

//配置矢量中断，矢量的意思就是有顺序，有先后的意思。
static void NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;	//定义数据结构体
	u8 i;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置优先级配置的模式

	if(gpIoInDefs==NULL) return;
	
	for(i=0;i<IOIN_MAX;i++)
	{
		if(gpIoInDefs[i].Idx>=IOIN_MAX) break;
		NVIC_ConfigOne(&gpIoInDefs[i]);
	}
}

void IoDefinesInit(void)
{
	HwChoice();
	GPIO_Config();	
	EXTI_Config();
	NVIC_Config();
}

//------------------------------------------------------------------------------//

//输出io的api
void IOOUT_SetIoStatus(IO_OUT_DEFS Io,bool IsHigh)
{
	if(IsHigh)
		GPIO_SetBits(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
	else
		GPIO_ResetBits(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
}

//输出io的api
void IOOUT_SetOne(const IO_OUT_HAL_DEFINE *pOutPin,bool IsHigh)
{
	if(IsHigh)
		GPIO_SetBits(gGroupMap[pOutPin->Group].GpioGroup,gPinMap[pOutPin->Pin].GpioPin);
	else
		GPIO_ResetBits(gGroupMap[pOutPin->Group].GpioGroup,gPinMap[pOutPin->Pin].GpioPin);
}

//输出io的api
u8 IOOUT_ReadIoStatus(IO_OUT_DEFS Io)
{
	return GPIO_ReadOutputDataBit(gGroupMap[gpIoOutDefs[Io].Group].GpioGroup,gPinMap[gpIoOutDefs[Io].Pin].GpioPin);
}

//------------------------------------------------------------------------------//

//输入io的api
void IOIN_SetIoStatus(IO_IN_DEFS Io,bool IsHigh)
{
	if(IsHigh)
		GPIO_SetBits(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
	else
		GPIO_ResetBits(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
}

//输入io的api
u8 IOIN_ReadIoStatus(IO_IN_DEFS Io)
{
	return GPIO_ReadInputDataBit(gGroupMap[gpIoInDefs[Io].Group].GpioGroup,gPinMap[gpIoInDefs[Io].Pin].GpioPin);
}

//输入io的api
u8 IOIN_ReadOne(const IO_IN_HAL_DEFINE *pInPin)
{
	return GPIO_ReadInputDataBit(gGroupMap[pInPin->Group].GpioGroup,gPinMap[pInPin->Pin].GpioPin);
}

//------------------------------------------------------------------------------//

//io中断控制api
void IOIN_OpenExti(IO_IN_DEFS Io)
{
	EXTI->IMR |= gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}

void IOIN_OpenOneExti(const IO_IN_HAL_DEFINE *pInPin)
{
	EXTI->IMR |= gPinMap[pInPin->Pin].ExtiLine;
}

void IOIN_CloseExti(IO_IN_DEFS Io)
{
	EXTI->IMR &=~gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}

void IOIN_CloseOneExti(const IO_IN_HAL_DEFINE *pInPin)
{
	EXTI->IMR &=~gPinMap[pInPin->Pin].ExtiLine;
}

u16 IOIN_ReadExti(IO_IN_DEFS Io) 
{
	return EXTI->IMR & gPinMap[gpIoInDefs[Io].Pin].ExtiLine;
}

u16 IOIN_ReadOneExti(const IO_IN_HAL_DEFINE *pInPin) 
{
	return EXTI->IMR & gPinMap[pInPin->Pin].ExtiLine;
}



