#include "SysDefines.h"
#include "Product.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(int a,void *p)
{
	Debug("Test InitEF\n\r");

	return EFR_OK;
}

static EVENT_HANDLER_RESUTL Evt1_EF(int a,void *p)
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
	EventControllerRegister(gTestController,"My Test Controller");
}

