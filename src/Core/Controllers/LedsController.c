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

本文件定义的控制器主要用来处理led的模式
*/
//------------------------------------------------------------------//


#include "SysDefines.h"
#include "Product.h"
#include "LedsMode.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	//指示灯
	LedSet(IOOUT_LED1,0);
	LedSet(IOOUT_LED2,0);
	
	return EFR_OK;
}

static LEDS_MODE gLedsMode[2]={LM_OFF,LM_OFF};
static u32 gLedTimCnt[2]={0,0};//计时器，避免回调频繁调用

//定时器到期关灯
static void TimerExpLed(u32 Io,void *p)
{
	u32 TimDly=0;
	u16 Cnt=HBit16(Io);
	Io=LBit16(Io);
	
	//Debug("LedTimer Led%u Mode%u State %s %u\n\r",Io,gLedsMode[Io],p==NULL?"NULL":"TRUE",GetSysStartMs());
	
	switch(gLedsMode[Io])
	{
		case LM_OFF:	LedSet(IOOUT_LED1+Io,0);break;
		case LM_ON:	LedSet(IOOUT_LED1+Io,1);break; 

		case LM_ON_500MS:
		case LM_ON_1S:
		case LM_ON_2S:
			gLedsMode[Io]=LM_OFF;LedSet(IOOUT_LED1+Io,0);
			break;

		case LM_FLASH_200MS_L2S:
		case LM_FLASH_200MS:	TimDly=200;goto SetLoop;
		case LM_FLASH_500MS:	TimDly=500;goto SetLoop;
		case LM_FLASH_2S:			TimDly=2000;goto SetLoop;
	}	

	if(0)
	{
SetLoop:
		LedSet(IOOUT_LED1+Io,p==NULL?0:1);
		if(GetSysStartMs()-gLedTimCnt[Io] >= TimDly)
		{
			if(Cnt==0) return;
			else if(Cnt==0xffff);
			else Cnt--;

			gLedTimCnt[Io]=GetSysStartMs();
			AddOnceMsFunc(TimDly,TimerExpLed,(Cnt<<16)|Io,p==NULL?(void *)-1:NULL);
		}
	}
}

//用户自定义事件
static EVENT_HANDLER_RESUTL Led_ChangeMode_EF(EVENT_BIT_FLAG Event,LEDS_MODE Mode,void *p)
{
	u32 TimDly=0;
	u16 Cnt=0xffff;
	u8 Io=HBit4(Mode);
	Mode=LBit4(Mode);

	//Debug("Set Led %u Mode %u\n\r",Io,Mode);

	switch(Mode)
	{
		case LM_OFF:	gLedsMode[Io]=LM_OFF;LedSet(IOOUT_LED1+Io,0);break;
		case LM_ON:	gLedsMode[Io]=LM_ON;LedSet(IOOUT_LED1+Io,1);break;
		
		case LM_ON_500MS:	TimDly=500;goto SetLoop1;
		case LM_ON_1S:		TimDly=1000;goto SetLoop1;
		case LM_ON_2S:		TimDly=2000;goto SetLoop1;

		case LM_FLASH_200MS:	TimDly=200;goto SetLoop2;
		case LM_FLASH_500MS:	TimDly=500;goto SetLoop2;
		case LM_FLASH_2S:			TimDly=2000;goto SetLoop2;	
		case LM_FLASH_200MS_L2S: Cnt=10;TimDly=200;goto SetLoop2;
	}

	if(0)
	{
SetLoop1:
		gLedsMode[Io]=LM_OFF;
		LedSet(IOOUT_LED1+Io,1);
		AddOnceMsFunc(TimDly,TimerExpLed,Io,NULL);
	}

	if(0)
	{
SetLoop2:
		gLedsMode[Io]=Mode;
		gLedTimCnt[Io]=GetSysStartMs();			
		LedSet(IOOUT_LED1+Io,1);
		AddOnceMsFunc(TimDly,TimerExpLed,(Cnt<<16)|Io,NULL);
	}	

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gLedsController[]={
{EBF_INIT,Init_EF},
{EBF_LED_MODE,Led_ChangeMode_EF},


{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void LedsControllerReg(void)
{
	ControllerRegister(gLedsController,"Leds Controller");
}

