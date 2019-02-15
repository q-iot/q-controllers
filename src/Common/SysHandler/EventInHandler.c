//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件处理了与事件相关的机制
*/
//------------------------------------------------------------------//
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
		volatile EVENT_ITEM *p=Q_Malloc(sizeof(EVENT_ITEM));//在WaitEvent中释放
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
		ControllerEvtPost(EBF_IDLE,0,NULL);
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

