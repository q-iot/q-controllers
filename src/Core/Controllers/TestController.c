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

本文件定义的控制器主要用做开发模板，当用户想定义一个业务或功能时，可以
直接拷贝本文件，修改控制器名进行后续开发。
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("Test InitEF\n\r");

	return EFR_OK;
}

static EVENT_HANDLER_RESUTL Evt1_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("Test Event 1\n\r");

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gTestController[]={
{EBF_INIT,Init_EF},
{EBF_USER_EVT1,Evt1_EF},




{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void TestControllerReg(void)
{
	ControllerRegister(gTestController,"My Test Controller");
}

