#include "stm32f10x.h"
#include "uart.h"
#include "PublicFunc.h"
#include "Q_Heap.h"
#include "Q_Queue.h"

#define QUEUE_RIGHT_CODE 0x25842311//随机数

typedef struct{
	u32 RightCode;//用来代表正确的句柄
	u16 ItemSize;//不含QUEUE_ITEM部分
	u16 MaxNum;
	u16 NowTotal;//当前item个数
	void *pFirst;
	void *pLast;
	void *pNull;	
}QUEUE_HEADER;

typedef struct{
	void *pPre;
	void *pNext;
}QUEUE_ITEM;

//新建队列
void *Q_NewQueue(u16 ItemSize,u16 MaxNum)
{
	QUEUE_HEADER *pQueue;
	u16 MallcoSize;
	IntSaveInit();

	EnterCritical();
	ItemSize=AlignTo4(ItemSize);// 4字节对齐；
	MallcoSize=sizeof(QUEUE_HEADER)+MaxNum*(sizeof(QUEUE_ITEM)+ItemSize);
	//Debug("Queue Mallco %d\r\n",MallcoSize);
	pQueue=Q_Malloc(MallcoSize);//在Q_DeleteQueue中释放
	//Debug("MallcoQ %x\r\n",(u32)pQueue);
	MemSet((void *)pQueue,0,MallcoSize);
	pQueue->RightCode=QUEUE_RIGHT_CODE;
	pQueue->ItemSize=ItemSize;
	pQueue->MaxNum=MaxNum;
	pQueue->NowTotal=0;
	pQueue->pFirst=NULL;
	pQueue->pLast=NULL;
	pQueue->pNull=(void *)&pQueue[1];//要越过头部，所以用索引1
	LeaveCritical();
	
	return pQueue;
}

//删除一个队列，会回收资源
bool Q_DeleteQueue(void **ppHandler)
{
	QUEUE_HEADER *pQueue=*ppHandler;	
	IntSaveInit();
	
	EnterCritical();
	if(*ppHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) 
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	//Debug("FreeQ %x\r\n",(u32)pQueue);
	
	pQueue->RightCode=0;
	*ppHandler=NULL;
	Q_Free(pQueue);
	LeaveCritical();
	
	return TRUE;
}

//清除队列所有项
bool Q_QueueClean(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	u16 Size;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	Size=pQueue->MaxNum*(sizeof(QUEUE_ITEM)+pQueue->ItemSize);
	//Debug("Clean %d\n\r",Size);
	MemSet((void *)&pQueue[1],0,Size);//要越过头部，所以用索引1
	pQueue->NowTotal=0;
	pQueue->pFirst=NULL;
	pQueue->pLast=NULL;
	pQueue->pNull=(void *)&pQueue[1];//要越过头部，所以用索引1
	LeaveCritical();
	
	return TRUE;
}

//原子操作
//先进先出协议
//如果force=false，则加满就不加了
//如果force=true，则加满，就把头部的挤掉
bool Q_QueueAddItem(void *pHandler,void *New,bool Force)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pNull=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE || New==NULL)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	if(pQueue->pNull==NULL && Force==FALSE) //没有空位置了
	{
		LeaveCritical();
		return FALSE;
	}	
	
	if(pQueue->pNull==NULL && Force==TRUE) //强行加入,则删除头部
	{
		QUEUE_ITEM *pFirst=pQueue->pFirst;
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //队列只有一个成员
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewFirst=pFirst->pNext;
			pQueue->pFirst=pNewFirst;
			pNewFirst->pPre=pNewFirst;//避免被当做空闲块
		}
		pFirst->pNext=NULL;
		pFirst->pPre=NULL;	

		if(pQueue->pNull==NULL) //本来没空间，现在有了
		{
			pQueue->pNull=pFirst;
		}
	}
	
	pNull=pQueue->pNull;//得到空位置

	//Debug("QueueAddItem\r\n");

	if(pQueue->pFirst==NULL)//新加的是第一项
	{
		pQueue->pFirst=pNull;
		pQueue->pLast=pNull;
		pNull->pPre=pNull;//不能都为空，否则会被当做空闲块
		pNull->pNext=NULL;
	}
	else if(pQueue->pLast!=NULL)//添加到队列尾部
	{
		QUEUE_ITEM *pLast=pQueue->pLast;

		pQueue->pLast=pNull;
		pLast->pNext=pNull;
		pNull->pPre=pLast;
		pNull->pNext=NULL;
	}
	else
	{
		Debug("Queue Add Item Error!\r\n");
		LeaveCritical();
		while(1);
	}

	MemCpy((void *)&pNull[1],New,pQueue->ItemSize);//拷贝内容，要越过头部，所以用索引1
	pQueue->NowTotal++;
	
	//开始找空闲的块
	pQueue->pNull=NULL;
	{
		u16 i;
		QUEUE_ITEM *pNewNull=(void *)&pQueue[1];//要越过头部，所以用索引1
		
		for(i=0;i<pQueue->MaxNum;i++) //找新的空位置
		{
			if(pNewNull->pNext==NULL && pNewNull->pPre==NULL)
			{
				pQueue->pNull=pNewNull;
				LeaveCritical();
				return TRUE;
			}		
			pNewNull=(void *)(pQueue->ItemSize+(u32)&pNewNull[1]);
		}		
	}
	
	LeaveCritical();
	return TRUE;//从此处返回表示队列满
}


//原子操作
//队列是先进先出，不过此函数用来将对象加到队列头部，破坏了先进先出秩序
//如果force=false，则加满就不加了
//如果force=true，则加满，就把尾部的挤掉
bool Q_QueueAddItemToFirst(void *pHandler,void *New,bool Force)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pNull=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE || New==NULL)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	if(pQueue->pNull==NULL && Force==FALSE) //没有空位置了
	{
		LeaveCritical();
		return FALSE;
	}	
	
	if(pQueue->pNull==NULL && Force==TRUE) //空间满，强行加入,则删除尾部
	{
		QUEUE_ITEM *pLast=pQueue->pLast;
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //队列只有一个成员
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else//队列多个成员
		{
			QUEUE_ITEM *pNewLast=pLast->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		pLast->pNext=NULL;
		pLast->pPre=NULL;	

		if(pQueue->pNull==NULL) //本来没空间，现在有了
		{
			pQueue->pNull=pLast;
		}
	}
	
	pNull=pQueue->pNull;//得到空位置

	//Debug("QueueAddItem\r\n");

	if(pQueue->pFirst==NULL)//新加的是第一项
	{
		pQueue->pFirst=pNull;
		pQueue->pLast=pNull;
		pNull->pPre=pNull;//不能都为空，否则会被当做空闲块
		pNull->pNext=NULL;
	}
	else if(pQueue->pLast!=NULL)//添加到队列头部
	{
		QUEUE_ITEM *pFirst=pQueue->pFirst;

		pQueue->pFirst=pNull;
		pFirst->pPre=pNull;
		pNull->pPre=pNull;//赋值，防止被当做空闲块
		pNull->pNext=pFirst;
	}
	else
	{
		Debug("Queue Add Item To First Error!\r\n");
		LeaveCritical();
		while(1);
	}

	MemCpy((void *)&pNull[1],New,pQueue->ItemSize);//拷贝内容，要越过头部，所以用索引1
	pQueue->NowTotal++;
	
	//开始找空闲的块
	pQueue->pNull=NULL;
	{
		u16 i;
		QUEUE_ITEM *pNewNull=(void *)&pQueue[1];//要越过头部，所以用索引1
		
		for(i=0;i<pQueue->MaxNum;i++) //找新的空位置
		{
			if(pNewNull->pNext==NULL && pNewNull->pPre==NULL)
			{
				pQueue->pNull=pNewNull;
				LeaveCritical();
				return TRUE;
			}		
			pNewNull=(void *)(pQueue->ItemSize+(u32)&pNewNull[1]);
		}		
	}
	
	LeaveCritical();
	return TRUE;//从此处返回表示队列满
}


//取队列头
bool Q_FetchQueueFirst(void *pHandler,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pFirst=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	pFirst=pQueue->pFirst;
	if(pFirst==NULL)	//没有内容
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}

	if(pRead!=NULL) MemCpy(pRead,(void *)&pFirst[1],pQueue->ItemSize);//拷贝内容出去，要越过头部，所以用索引1
	
	if(NeedDelete)
	{		
		pQueue->NowTotal--;

		if(pQueue->pLast==pQueue->pFirst) //队列只有一个成员
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewFirst=pFirst->pNext;
			pQueue->pFirst=pNewFirst;
			pNewFirst->pPre=pNewFirst;//避免被当做空闲块
		}
		pFirst->pNext=NULL;
		pFirst->pPre=NULL;	

		if(pQueue->pNull==NULL) //本来没空间，现在有了
		{
			pQueue->pNull=pFirst;
		}	
	}
	
	LeaveCritical();
	return TRUE;
}

//取队列尾
bool Q_FetchQueueLast(void *pHandler,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pLast=NULL;
	IntSaveInit();

	EnterCritical();
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	pLast=pQueue->pLast;
	if(pLast==NULL)	//没有内容
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}
	
	if(pRead!=NULL) MemCpy(pRead,(void *)&pLast[1],pQueue->ItemSize);//拷贝内容出去，要越过头部，所以用索引1

	if(NeedDelete)
	{
		pQueue->NowTotal--;
		
		if(pQueue->pLast==pQueue->pFirst) //队列只有一个成员
		{
			pQueue->pFirst=NULL;
			pQueue->pLast=NULL;
		}
		else
		{
			QUEUE_ITEM *pNewLast=pLast->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		pLast->pNext=NULL;
		pLast->pPre=NULL;

		if(pQueue->pNull==NULL) //本来没空间，现在有了
		{
			pQueue->pNull=pLast;
		}		
	}
	
	LeaveCritical();
	return TRUE;
}

//idx从1开始
//取指定索引的对象
//未取到返回false
bool Q_FetchQueueItem(void *pHandler,u16 Idx,void *pRead,bool NeedDelete)
{
	QUEUE_HEADER *pQueue=pHandler;
	QUEUE_ITEM *pItem=NULL;
	u16 Cnt=0;
	IntSaveInit();

	EnterCritical();
	if(Idx==0 || pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE)
	{
		LeaveCritical();
		return FALSE;//错误的句柄
	}
	
	pItem=pQueue->pFirst;
	if(pItem==NULL)	//没有内容
	{
		if(pRead!=NULL) MemSet(pRead,0,pQueue->ItemSize);
		LeaveCritical();
		return FALSE;
	}

	for(Cnt=1;Cnt<Idx;Cnt++)
	{
		pItem=pItem->pNext;
		if(pItem==NULL) 
		{
			LeaveCritical();
			return FALSE;//读完还没读到对应的idx
		}
	}	

	if(pRead!=NULL) MemCpy(pRead,(void *)&pItem[1],pQueue->ItemSize);//拷贝内容出去，要越过头部，所以用索引1

	if(NeedDelete)
	{
		pQueue->NowTotal--;
		
		if(Idx==1)//头成员
		{
			if(pQueue->pLast==pQueue->pFirst) //队列只有一个成员
			{
				pQueue->pFirst=NULL;
				pQueue->pLast=NULL;
			}
			else
			{
				QUEUE_ITEM *pNewFirst=pItem->pNext;
				pQueue->pFirst=pNewFirst;
				pNewFirst->pPre=pNewFirst;//避免被当做空闲块
			}
		}
		else if(pItem->pNext==NULL)//尾成员
		{			
			QUEUE_ITEM *pNewLast=pItem->pPre;
			pQueue->pLast=pNewLast;
			pNewLast->pNext=NULL;
		}
		else//中间成员
		{
			QUEUE_ITEM *pItemTmp;
			pItemTmp=pItem->pPre;
			pItemTmp->pNext=pItem->pNext;
			pItemTmp=pItem->pNext;
			pItemTmp->pPre=pItem->pPre;
		}

		//删除本块
		pItem->pNext=NULL;
		pItem->pPre=NULL;	

		if(pQueue->pNull==NULL) //本来没空间，现在有了
		{
			pQueue->pNull=pItem;
		}		
	}
	
	LeaveCritical();
	return TRUE;
}

//获取队列项目个数
u16 Q_GetQueueItemTotal(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return 0;//错误的句柄

	return pQueue->NowTotal;
}

//队列空
bool Q_QueueEmpty(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//错误的句柄

	if(pQueue->NowTotal==0) return TRUE;

	return FALSE;
}

//队列非空
bool Q_QueueNotEmpty(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;
	
	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//错误的句柄

	if(pQueue->NowTotal==0) return FALSE;

	return TRUE;
}

//队列满
bool Q_QueueFull(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//错误的句柄
	
	if(pQueue->pNull==NULL) return TRUE;

	return FALSE;
}

//队列未满
bool Q_QueueNotFull(void *pHandler)
{
	QUEUE_HEADER *pQueue=pHandler;

	if(pHandler==NULL || pQueue->RightCode!=QUEUE_RIGHT_CODE) return FALSE;//错误的句柄
	
	if(pQueue->pNull==NULL) return FALSE;

	return TRUE;
}

