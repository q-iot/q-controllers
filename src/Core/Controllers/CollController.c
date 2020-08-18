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


#include "SysDefines.h"
#include "Product.h"

static u32 gLight=0;
static u32 gTemp=0;

//定时器回调
void AdcLoop_CB(int cnt,int a,void *p)
{
	gLight=Adc1_GetValByAio(0)*1000/4096;
	gTemp=Adc1_GetValByAio(1)*3300/4096;

	Debug("ReadAdc %u %u\n\r",gLight,gTemp);

	QCom_SetVarValue("QSXXTEMP",gTemp,FALSE);//先发第一个值，回包再发第二个值
}

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	Debug("Start Data Collection\n\r");
	Adc1_Rand_Init(Bit(0)|Bit(1));//adc pa0 pa1 pa2 pa3，每个bit代表一路
	AddMsFunc((u32)-1,5000,AdcLoop_CB,0,NULL);
	
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

	return EFR_OK;
}

//发出变量指令，收到回复
static EVENT_HANDLER_RESUTL QWIFI_Varible_Res_EF(EVENT_BIT_FLAG Event,int Value,const char *pVarTag)
{
	//Debug("Event %u call %s [%s]=%d\n\r",Event,__FUNCTION__,pVarTag,Value);
	//Debug("LastCmd:%s\n\r",QCom_GetLastCmd());

	if(Event==EBF_QWIFI_SET_VAR_RET && strcmp(pVarTag,"QSXXTEMP")==0)//发送成功再发第二个值
	{
		QCom_SetVarValue("QSXXLIGH",gLight,FALSE);	
	}
	
	if(Event==EBF_QWIFI_SET_VAR_RET && strcmp(pVarTag,"QSXXLIGH")==0)
	{
		Debug("Var Set Success.\n\r");
	}

	return EFR_OK;
}


//控制器定义和注册
static const EVENT_FUNC_ITEM gCollController[]={
{EBF_INIT,Init_EF},
{EBF_IDLE,Idle_EF},
{EBF_KEY,KeyHandler_EF},
{EBF_QWIFI_SET_VAR_RET,QWIFI_Varible_Res_EF},



{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void CollControllerReg(void)
{
	ControllerRegister(gCollController,"Data Collection Controller");
}

