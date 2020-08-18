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

本文件定义的控制器主要用来做代码演示，演示了控制器应该如何处理与qwifi的通信，
而不需要接触任何字符串解析，仅仅调用qcom的api即可。
如果用户需要自己写一个功能与qwifi交互，则将本文件拷贝改名即可。
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"
#include "StrParse.h"

//收到qwifi状态事件
static EVENT_HANDLER_RESUTL QWIFI_State_EF(EVENT_BIT_FLAG Event,QWIFI_STATE State,const char *pCmd)
{
	Debug("Call %s [%u]%s\n\r",__FUNCTION__,State,pCmd);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
	return EFR_OK;
}

//收到qwifi的app按钮事件
static EVENT_HANDLER_RESUTL QWIFI_Key_EF(EVENT_BIT_FLAG Event,u32 Key,u32 DevID)
{
	Debug("Call %s %u Dev%u\n\r",__FUNCTION__,Key,DevID);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
	return EFR_OK;
}

//收到qwifi系统消息事件
static EVENT_HANDLER_RESUTL QWIFI_Message_EF(EVENT_BIT_FLAG Event,u32 Len,const char *pMsg)
{
	Debug("Call %s [%u]%s\n\r",__FUNCTION__,Len,pMsg);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
	return EFR_OK;
}

//发出系统消息，收到回复
static EVENT_HANDLER_RESUTL QWIFI_Message_Res_EF(EVENT_BIT_FLAG Event,u32 Len,const char *pMsg)
{
	Debug("Event %u call %s [%u]%s\n\r",Event,__FUNCTION__,Len,pMsg);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
	return EFR_OK;
}

//收到qwifi变量事件
static EVENT_HANDLER_RESUTL QWIFI_Varible_EF(EVENT_BIT_FLAG Event,int Value,const char *pVarTag)
{
	Debug("Call %s [%s]=%d\n\r",__FUNCTION__,pVarTag,Value);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
	return EFR_OK;
}

//发出变量指令，收到回复
static EVENT_HANDLER_RESUTL QWIFI_Varible_Res_EF(EVENT_BIT_FLAG Event,int Value,const char *pVarTag)
{
	Debug("Event %u call %s [%s]=%d\n\r",Event,__FUNCTION__,pVarTag,Value);
	Debug("LastCmd:%s\n\r",QCom_GetLastCmd());

#if 0//此部分用于解析命令的示例
	if(strlen(QCom_GetLastCmd()))
	{
		const char *pLastCmd=QCom_GetLastCmd();
		char *pCmd[6];
		char *pBuf=NULL;
		u16 i,Num=0;
		
		pBuf=Q_Malloc(64);
		Num=StrCmdParse(pLastCmd,pCmd,pBuf,TRUE);//解析指令和参数

		for(i=0;i<Num;i++)
		{
			Debug("%u:%s\n\r",i,pCmd[i]);
		}

		Q_Free(pBuf);
	}
#endif

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gQWifiController[]={
{EBF_QWIFI_STATE,QWIFI_State_EF},
{EBF_QWIFI_KEY,QWIFI_Key_EF},
{EBF_QWIFI_MSG,QWIFI_Message_EF},
{EBF_QWIFI_MSG_RET,QWIFI_Message_Res_EF},
{EBF_QWIFI_STR_RET,QWIFI_Message_Res_EF},
{EBF_QWIFI_VAR,QWIFI_Varible_EF},
{EBF_QWIFI_READ_VAR_RET,QWIFI_Varible_Res_EF},
{EBF_QWIFI_READ_VAR_ERROR,QWIFI_Varible_Res_EF},
{EBF_QWIFI_SET_VAR_RET,QWIFI_Varible_Res_EF},
{EBF_QWIFI_SET_VAR_ERROR,QWIFI_Varible_Res_EF},




{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void QWifiControllerReg(void)
{
	ControllerRegister(gQWifiController,"QWifi Controller");
}

