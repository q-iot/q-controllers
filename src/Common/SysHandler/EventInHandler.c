#include "SysDefines.h"

typedef struct{
	EVENT_BIT_FLAG Event;
	s32 Param;
	void *pParam;
	void *pNext;
}EVENT_ITEM;
static volatile EVENT_ITEM *gpEventItems=NULL;
static volatile EVENT_ITEM *gpEventItemsLast=NULL;
static EVENT_BIT_FLAG gCurrentEvent;//记录当前处理的事件

void EventDebug(void)
{
	volatile EVENT_ITEM *pHandler=gpEventItems;

	while(pHandler!=NULL)
	{
		Debug("Event[%u] p=%u,0x%x | %x -> %x\n\r",pHandler->Event,pHandler->Param,pHandler->pParam,(u32)pHandler,(u32)pHandler->pNext);
		pHandler=pHandler->pNext;
	}

	Debug("Last:0x%x\n\r",(u32)gpEventItemsLast);
}

//无论多少次，都当做一次处理
void SetEventFlag(EVENT_BIT_FLAG Event)
{
	//Debug("F%u ",Event);
	if(Event>(u32)EBF_NULL && Event<(u32)EBF_MAX) 
	{
		EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));
		IntSaveInit();
		
		p->Event=Event;
		p->Param=0;
		p->pParam=NULL;
		p->pNext=NULL;

		EnterCritical();
		if(gpEventItems==NULL)
		{
			gpEventItems=p;
			gpEventItemsLast=p;
		}
		else
		{
			volatile EVENT_ITEM *pItem=gpEventItems;
			
			//找找有没有一样的，覆盖
			while(pItem!=NULL)
			{
				if(pItem->Event==Event)
				{
					pItem->Param=0;
					pItem->pParam=NULL;
					Q_Free(p);
					LeaveCritical();
					return;
				}
				pItem=pItem->pNext;
			}

			//没有一样的，建立新的
			gpEventItemsLast->pNext=(void *)p;
			gpEventItemsLast=p;
		}
		LeaveCritical();
	}
}

//调用一次，则发送一次事件处理
void SendEvent(EVENT_BIT_FLAG Event,s32 S32Param,void *pParam)
{
	//Debug("E%u ",Event);
	if(Event>(u32)EBF_NULL && Event<(u32)EBF_MAX) 
	{
		volatile EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));
		IntSaveInit();
		
		p->Event=Event;
		p->Param=S32Param;
		p->pParam=pParam;
		p->pNext=NULL;

		EnterCritical();
		if(gpEventItems==NULL)
		{
			gpEventItems=p;
			gpEventItemsLast=p;
		}
		else
		{
			gpEventItemsLast->pNext=(void *)p;
			gpEventItemsLast=p;
		}
		LeaveCritical();
	}
}

void CleanAllEvent(void)
{
	IntSaveInit();
	EnterCritical();
	while(gpEventItems!=NULL)
	{
		volatile EVENT_ITEM *p=gpEventItems->pNext;
		Q_Free((void *)gpEventItems);
		gpEventItems=p;
	}
	gpEventItemsLast=NULL;
	LeaveCritical();
}

//等待事件
void *WaitEvent(EVENT_BIT_FLAG *pEvent,s32 *pS32)
{
	void *pRet=NULL;
	IntSaveInit();

	gCurrentEvent=EBF_NULL;//进wait event表示前面的事件已经处理完了
	
	while(gpEventItems==NULL)//wait data in flag.等着也是没事，不如做点事
	{
		EventControllerPost(EBF_IDLE,0,NULL);
	}

	EnterCritical();
	gCurrentEvent=gpEventItems->Event;//记录当前执行的事件
	*pEvent=gpEventItems->Event;
	*pS32=gpEventItems->Param;
	pRet=gpEventItems->pParam;

	if(gpEventItems==gpEventItemsLast)
	{
		Q_Free((void *)gpEventItems);
		gpEventItems=gpEventItemsLast=NULL;
	}
	else
	{
		volatile EVENT_ITEM *p=gpEventItems->pNext;
		Q_Free((void *)gpEventItems);
		gpEventItems=p;	
	}
	LeaveCritical();

	return pRet;
}

//检查事件处理有没有完成，如果完成了，返回true
bool CheckEventFinished(EVENT_BIT_FLAG Event)
{
	bool Ret=TRUE;
	IntSaveInit();
	
	if(gCurrentEvent==Event) return FALSE;

	EnterCritical();
	{
		volatile EVENT_ITEM *pItem=gpEventItems;
		
		//找找有没有一样的
		while(pItem!=NULL)
		{
			if(pItem->Event==Event)
			{
				Ret=FALSE;
				break;
			}
			pItem=pItem->pNext;
		}
	}
	LeaveCritical();

	return Ret;
}

//------------------------------------------控制器相关------------------------------------------
typedef struct{
	const char *pName;
	const EVENT_FUNC_ITEM *ItemArray;
	u16 EvtFuncTotal;
}EVENT_CTRLER_ITME;//控制器记录体

#define EVENT_CONTROLLER_MAX 64
static EVENT_CTRLER_ITME gEvtCtrlers[EVENT_CONTROLLER_MAX]={{NULL,0}};//控制器列表
static u16 gEvtCtrlerNum=0;

//控制器注册自己到系统
void EventControllerRegister(const EVENT_FUNC_ITEM *pItemArray,const char *pName)
{
	u16 i=0;
	
	for(i=0;i<EBF_MAX;i++)
	{
		if(pItemArray[i].Event==EBF_NULL || pItemArray[i].Event>=EBF_MAX) break;
		if(pItemArray[i].EvtFunc==NULL) break;
	}

	if(i)
	{
		gEvtCtrlers[gEvtCtrlerNum].pName=pName;
		gEvtCtrlers[gEvtCtrlerNum].ItemArray=pItemArray;
		gEvtCtrlers[gEvtCtrlerNum].EvtFuncTotal=i;
		gEvtCtrlerNum++;
	}
}

//控制器事件分发
void EventControllerPost(EVENT_BIT_FLAG Event,int Param,void *p)
{
	u16 CtrlerNum=0,EvtNum=0;

	for(CtrlerNum=0;CtrlerNum<gEvtCtrlerNum;CtrlerNum++)//轮询控制器
	{
		const EVENT_FUNC_ITEM *pItemArray=gEvtCtrlers[CtrlerNum].ItemArray;
		EVENT_HANDLER_RESUTL Res=EFR_OK;
		
		for(EvtNum=0;EvtNum<gEvtCtrlers[CtrlerNum].EvtFuncTotal;EvtNum++)//轮询事件回调
		{
			if(Event==pItemArray[EvtNum].Event && pItemArray[EvtNum].EvtFunc!=NULL)
			{
				Res=pItemArray[EvtNum].EvtFunc(Param,p);//事件处理函数回调
				if(Res==EFR_STOP) goto EvtFinish;
				break;//同一个事件每个控制器只能对应一个回调
			}
		}
	}

EvtFinish:
	return;
}

