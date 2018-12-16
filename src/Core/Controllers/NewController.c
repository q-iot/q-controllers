#include "SysDefines.h"
#include "Product.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(int a,void *p)
{
	Debug("InitEF\n\r");

	return EFR_OK;
}

//系统while(1)空等时循环调用的事件
static EVENT_HANDLER_RESUTL Idle_EF(int a,void *p)
{

	return EFR_OK;
}

//按键被按下并松开后的事件
static EVENT_HANDLER_RESUTL KeyHandler_EF(int a,void *p)
{
	u16 KeyIo=a&0xffff;
	u16 Ms=a>>16;

	Debug("New Key%u %umS\n\r",KeyIo,Ms);
	SendEvent(EBF_USER_EVT1,0,NULL);

	return EFR_OK;
}

//用户自定义事件
static EVENT_HANDLER_RESUTL Evt1_EF(int a,void *p)
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
	EventControllerRegister(gNewController,"My New Controller");
}

