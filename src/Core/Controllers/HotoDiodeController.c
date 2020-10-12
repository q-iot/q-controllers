//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。

Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。

所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于
酷物联视频（q-iot.cn）进行传播或有偿售卖，相应所得收入扣除税费及维护费用后，
均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件定义的控制器主要用来做代码演示，演示了控制器对按键事件等简单事件的
处理示例。
*/
//------------------------------------------------------------------//

//模式控制板程序，与q-ctrl主程序有干涉，为led1 2和key，需要在main里面关掉led1 2，中断5里设定key中断

#include "SysDefines.h"
#include "Product.h"

#define LED8_IO_NUM 7

const IO_IN_HAL_DEFINE gKeyDef={0,GPI_A,  GPin5, GPIO_Mode_IPU,EXTI_Trigger_Rising_Falling,EXTI0_IRQn,EXTI_Pio_Priority};

const IO_OUT_HAL_DEFINE gOutSingle1Def={0,GPI_A,	GPin3,		GPIO_Mode_Out_PP,FALSE};
const IO_OUT_HAL_DEFINE gOutSingle2Def={0,GPI_A,	GPin4,		GPIO_Mode_Out_PP,FALSE};
const IO_OUT_HAL_DEFINE gOutLedDef[LED8_IO_NUM]={
{0,GPI_A,	GPin15,	GPIO_Mode_Out_PP,TRUE},
{1,GPI_B,	GPin3,		GPIO_Mode_Out_PP,TRUE},//干涉
{2,GPI_B,	GPin5,		GPIO_Mode_Out_PP,TRUE},
{3,GPI_B,	GPin7,		GPIO_Mode_Out_PP,TRUE},
{4,GPI_B,	GPin8,		GPIO_Mode_Out_PP,TRUE},
{5,GPI_B,	GPin4,		GPIO_Mode_Out_PP,TRUE},//干涉
{6,GPI_B,	GPin6,		GPIO_Mode_Out_PP,TRUE}
};

static u8 gNowMode=1;

static void LED8_Set(u8 Map)
{
	u16 i;
	for(i=0;i<LED8_IO_NUM;i++) IOOUT_SetOne(&gOutLedDef[i],ReadBit(Map,i)?FALSE:TRUE);
}

static void LED8_SetModeNum(u8 Num)
{
	const u8 NumMap[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};//数码管映射
	LED8_Set(NumMap[Num]);

	switch(Num)
	{
		case 1:
			IOOUT_SetOne(&gOutSingle1Def,FALSE);//PA3
			IOOUT_SetOne(&gOutSingle2Def,TRUE);//PA4
			break;
		case 2:
			IOOUT_SetOne(&gOutSingle1Def,TRUE);//PA3
			IOOUT_SetOne(&gOutSingle2Def,TRUE);//PA4
			break;		
		case 3:
			IOOUT_SetOne(&gOutSingle1Def,TRUE);//PA3
			IOOUT_SetOne(&gOutSingle2Def,FALSE);//PA4
			break;		
		case 4:
			IOOUT_SetOne(&gOutSingle1Def,FALSE);//PA3
			IOOUT_SetOne(&gOutSingle2Def,FALSE);//PA4
			break;		
	}
	
} //110 1101    

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	u16 i;
	
	Debug("InitEF\n\r");

	GPIO_ConfigOne(&gKeyDef,NULL);
	GPIO_ConfigOne(NULL,&gOutSingle1Def);
	GPIO_ConfigOne(NULL,&gOutSingle2Def);
	for(i=0;i<LED8_IO_NUM;i++) GPIO_ConfigOne(NULL,&gOutLedDef[i]);

	EXTI_ConfigOne(&gKeyDef);
	NVIC_ConfigOne(&gKeyDef);

	LED8_SetModeNum(gNowDutMode);	
	
	return EFR_OK;
}

//系统while(1)空等时循环调用的事件
static EVENT_HANDLER_RESUTL Idle_EF(EVENT_BIT_FLAG Event,int a,void *p)
{

	return EFR_OK;
}

//按键被按下并松开后的事件
static EVENT_HANDLER_RESUTL KeyHandler_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	u16 KeyIo=a&0xffff;
	u16 Ms=a>>16;

	Debug("New Key%u %umS\n\r",KeyIo,Ms);
	//SendEvent(EBF_USER_EVT1,0,NULL);
	
	if(++gNowDutMode>4) gNowDutMode=1;
	LED8_SetModeNum(gNowDutMode);	

	return EFR_OK;
}

//用户自定义事件
static EVENT_HANDLER_RESUTL Evt1_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("New Event 1\n\r");

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gHotoDiodeController[]={
{EBF_INIT,Init_EF},
{EBF_IDLE,Idle_EF},
{EBF_KEY,KeyHandler_EF},
{EBF_USER_EVT1,Evt1_EF},




{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void HotoDiodeControllerReg(void)
{
	ControllerRegister(gHotoDiodeController,"HotoDiode Controller");
}

