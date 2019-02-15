//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件用来处理与控制器调度相关的机制
*/
//------------------------------------------------------------------//

#include "SysDefines.h"

typedef struct{
	const char *pName;
	const EVENT_FUNC_ITEM *ItemArray;
	u16 EvtFuncTotal;
}EVENT_CTRLER_ITME;//控制器记录体

#define EVENT_CONTROLLER_MAX 64
static EVENT_CTRLER_ITME gpEvtCtrlers[EVENT_CONTROLLER_MAX]={{NULL,0}};//控制器列表
static u16 gEvtCtrlerNum=0;

//打印信息
void ControllerDebug(void)
{
	u16 i;

	for(i=0;i<gEvtCtrlerNum;i++)
	{
		Debug("[%s] Has %u Evt\n\r",gpEvtCtrlers[i].pName,gpEvtCtrlers[i].EvtFuncTotal);
	}
}

//控制器注册自己到系统
void ControllerRegister(const EVENT_FUNC_ITEM *pItemArray,const char *pName)
{
	u16 i;
	
	for(i=0;i<EBF_MAX;i++)
	{
		if(pItemArray[i].Event==EBF_NULL || pItemArray[i].Event>=EBF_MAX) break;
		if(pItemArray[i].EvtFunc==NULL) break;
	}

	if(i)
	{
		gpEvtCtrlers[gEvtCtrlerNum].pName=pName;
		gpEvtCtrlers[gEvtCtrlerNum].ItemArray=pItemArray;
		gpEvtCtrlers[gEvtCtrlerNum].EvtFuncTotal=i;
		gEvtCtrlerNum++;
	}
}

//控制器事件分发
void ControllerEvtPost(EVENT_BIT_FLAG Event,int Param,void *p)
{
	u16 CtrlerNum=0,EvtNum=0;

	for(CtrlerNum=0;CtrlerNum<gEvtCtrlerNum;CtrlerNum++)//轮询控制器
	{
		const EVENT_FUNC_ITEM *pItemArray=gpEvtCtrlers[CtrlerNum].ItemArray;
		EVENT_HANDLER_RESUTL Res=EFR_OK;
		
		for(EvtNum=0;EvtNum<gpEvtCtrlers[CtrlerNum].EvtFuncTotal;EvtNum++)//轮询事件回调
		{
			if(Event==pItemArray[EvtNum].Event && pItemArray[EvtNum].EvtFunc!=NULL)
			{
				Res=pItemArray[EvtNum].EvtFunc(Event,Param,p);//事件处理函数回调
				if(Res==EFR_STOP) goto EvtFinish;
				break;//同一个事件每个控制器只能对应一个回调
			}
		}
	}

EvtFinish:
	//EventMemoryFree(Event,Param,p);//事件内存回收
	return;
}


