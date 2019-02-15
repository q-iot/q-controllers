//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义的控制器主要用来做代码演示，演示了控制器对按键事件等简单事件的
处理示例。
*/
//------------------------------------------------------------------//


#include "SysDefines.h"
#include "Product.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("InitEF\n\r");

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
	SendEvent(EBF_USER_EVT1,0,NULL);

	return EFR_OK;
}

//用户自定义事件
static EVENT_HANDLER_RESUTL Evt1_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("New Event 1\n\r");

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gNewController[]={
{EBF_INIT,Init_EF},
{EBF_IDLE,Idle_EF},
{EBF_KEY,KeyHandler_EF},
{EBF_USER_EVT1,Evt1_EF},




{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void NewControllerReg(void)
{
	ControllerRegister(gNewController,"My New Controller");
}

