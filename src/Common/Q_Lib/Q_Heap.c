//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件自定义了一个堆分配机制
*/
//------------------------------------------------------------------//

#include "Drivers.h"
#include "PublicFunc.h"
#include "Q_Heap.h"
#include "Product.h"
//#include "Os_Wrap.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#if Q_HEAP_DEBUG == 1 || Q_HEAP_DEBUG == 3
#define QH_Debug Debug
#else
#define QH_Debug(x,y...)
#endif

//为保证多线程下堆分配的原子操作，必须借助操作系统临界区
//如果此机制不被用于多线程，则可以定义下面三个个宏为空白
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	IntSaveInit()
#define Q_HEAP_ENTER_CRIT EnterCritical()
#define Q_HEAP_EXIT_CRIT LeaveCritical()

//用来验证内存是否被冲毁的标识
//占用2个字节，所以内存前后被冲毁而不能察觉的概率为65535分之1
#define Q_HEAP_RW_FLAG 0x5808 //放置于每个被分配内存块末尾的冲毁标识
#define Q_HEAP_RW_FLAG32 ((Q_HEAP_RW_FLAG<<16)|0x1234) //heap整个内存区域前后的冲毁标识，将Q_HEAP_UNIT_REWRITE_ID放到高16位是为了防止第一块分配块被释放造成的检查问题

//记录体
typedef struct{
	u16 RwFlag;//防冲毁标志
	u16 MemIsIdle:1;//内存是否空闲
	u16 Line:15;//调用的行号
	u32 Bytes;//Data所包含的字节数，必须是偶数
	void *pPrev;//前一个
	void *pNext;//后一个
#if Q_HEAP_DEBUG > 1
	const char *pFunc;//调用的函数名
#endif
	u16 Data[2];//内存，最后两个字节放防冲毁标志
}QHEAP_ITEM;

#if Q_HEAP_DEBUG > 1
#define QH_ITEM_COMM_BYTES 20//Data之前的大小
#define QH_ITEM_MIN_OCCUPY 36//一个item需要的最小占用bytes，包括头尾
#else
#define QH_ITEM_COMM_BYTES 16//Data之前的大小
#define QH_ITEM_MIN_OCCUPY 32//一个item需要的最小占用bytes，包括头尾
#endif

//定义堆地址
#define QH_HEAP_MEM_NUM 1
static u32 *pQHeap[QH_HEAP_MEM_NUM];//起点必须8字节对齐
static u32 *pQHeapEnd[QH_HEAP_MEM_NUM];
static QHEAP_ITEM *gpItemHdr=NULL;

//堆栈总大小
#define Q_HEAP_SIZE_BYTE (6*1024)
static u8 QHeapMem[Q_HEAP_SIZE_BYTE];//起点必须8字节对齐，使用内部CMM内存池

//调试
void QHeapDebug(void)
{
	QHEAP_ITEM *pItem=NULL;
	QHEAP_ITEM *p=NULL;
	u16 m;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Debug("-------------------------------------------------------------------------\n\r");
	Q_HEAP_ENTER_CRIT;	
	for(m=0;m<QH_HEAP_MEM_NUM;m++)
	{
		Debug("Heap[%u] 0x%x - 0x%x %u Byts\n\r",m,pQHeap[m],pQHeapEnd[m],(u32)pQHeapEnd[m]-(u32)pQHeap[m]);
	}

	Debug("\n\r");
	
	pItem=gpItemHdr;
	while(pItem)
	{
#if Q_HEAP_DEBUG > 1
		Debug("%8x<- %s Addr:%x - %05x p%x = %5uB ->%x %s:%u\n\r",pItem->pPrev,pItem->MemIsIdle?"[]":"XX",
			(u32)pItem,((u32)pItem+pItem->Bytes)&0xfffff,(u32)pItem->Data,
			pItem->Bytes,pItem->pNext,pItem->pFunc,pItem->Line);
#else
		Debug("%8x<- %s Addr:%x - %05x p%x = %5uB ->%x\n\r",pItem->pPrev,pItem->MemIsIdle?"[]":"XX",
			(u32)pItem,((u32)pItem+pItem->Bytes)&0xfffff,(u32)pItem->Data,
			pItem->Bytes,pItem->pNext);
#endif

		//检查链表指针准确性
		if(p=pItem->pNext)
		{
			if(p->pPrev!=pItem) Debug("PrevPoint ERROR!\n\r");
			if(((u32)(pItem->Data)+pItem->Bytes)!=(u32)p) Debug("- ADDR GAP -\n\r");
		}
		
		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;
	Debug("Total Idle Memory %u Byts\n\r",QHeapGetIdleSize());
	Debug("-------------------------------------------------------------------------\n\r\n\r");
}

//初始化
static void QHeapInit(void)
{
	u16 m;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Q_HEAP_ENTER_CRIT;

	//定义堆地址
	#if 1//各平台不同
	{
		pQHeap[0]=(void *)QHeapMem;
		*pQHeap[0]++=Q_HEAP_RW_FLAG32;
		pQHeapEnd[0]=(void *)((u32)QHeapMem+Q_HEAP_SIZE_BYTE);
	}
	#endif

	//建立记录
	for(m=0;m<QH_HEAP_MEM_NUM;m++)
	{
		QHEAP_ITEM *pItem=(void *)pQHeap[m];

		pItem->RwFlag=Q_HEAP_RW_FLAG;
		pItem->MemIsIdle=TRUE;
		pItem->Bytes=(u32)pQHeapEnd[m]-(u32)pQHeap[m]-QH_ITEM_COMM_BYTES;//用移位运算，可能比实际空间小
		pItem->pPrev=(m?pQHeap[m-1]:NULL);
		pItem->pNext=((m==(QH_HEAP_MEM_NUM-1))?NULL:pQHeap[m+1]);
		//MemSet(pItem->Data,0,pItem->Bytes);
#if Q_HEAP_DEBUG > 1
		pItem->pFunc="QHeapInit";
		pItem->Line=0;
#endif		
	}

	gpItemHdr=(void *)pQHeap[0];

	//QHeapDebug();

	Q_HEAP_EXIT_CRIT;
}

//检查冲毁标志
static bool _CheckRwFlag(void)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		u32 *p=pQHeap[i];
		
		if(*--p != Q_HEAP_RW_FLAG32) return FALSE;
	}

	return TRUE;
}

//征用一个块
static void *_RequItem(QHEAP_ITEM *pItem,u32 Size)
{
	if((Size+QH_ITEM_MIN_OCCUPY) <= pItem->Bytes)//剩下的空间足够大，新建一个
	{
		QHEAP_ITEM *pNew=NULL;
		QHEAP_ITEM *pNext=NULL;

		//新建一个空闲块，加到原有item的后面
		pNew=(void *)&pItem->Data[Size>>1];	
		pNew->RwFlag=Q_HEAP_RW_FLAG;
		pNew->MemIsIdle=TRUE;
		pNew->Bytes=pItem->Bytes-Size-QH_ITEM_COMM_BYTES;
		pNew->pPrev=pItem;
		pNew->pNext=pItem->pNext;
#if Q_HEAP_DEBUG > 1	
		pNew->pFunc="Idle";
		pNew->Line=0;
#endif
		//处理旧的并返回
		pNext=pItem->pNext;
		if(pNext) pNext->pPrev=pNew;
		
		pItem->MemIsIdle=FALSE;
		pItem->Bytes=Size;
		pItem->pNext=pNew;
		pItem->Data[(pItem->Bytes>>1)-1]=Q_HEAP_RW_FLAG;
		
		return pItem->Data;
	}
	else//剩下的空间不够大，所以无需新建了，直接用完
	{
		pItem->MemIsIdle=FALSE;
		pItem->Data[(pItem->Bytes>>1)-1]=Q_HEAP_RW_FLAG;
		return pItem->Data;
	}
}

//释放一个块
static void _ReleseItem(QHEAP_ITEM *pItem)
{
	QHEAP_ITEM *pNext;
	QHEAP_ITEM *pPrev;
	bool HasMerge=FALSE;

	pItem->MemIsIdle=TRUE;
#if Q_HEAP_DEBUG > 1	
	pItem->pFunc="Idle";
	pItem->Line=0;
#endif

	//查看后面一个块是不是为空闲
	pNext=pItem->pNext;
	if(pNext && pNext->MemIsIdle==TRUE)
	{
		if(((u32)pItem->Data+pItem->Bytes) == (u32)pNext)//地址是连续的，进行合并
		{
			pItem->MemIsIdle=TRUE;
			pItem->Bytes+=(QH_ITEM_COMM_BYTES+pNext->Bytes);
			pItem->pNext=pNext->pNext;

			pNext=pItem->pNext;
			if(pNext) pNext->pPrev=pItem;
		}
	}

	//查看前面一个块是不是为空闲
	pPrev=pItem->pPrev;
	if(pPrev && pPrev->MemIsIdle==TRUE)
	{
		if(((u32)pPrev->Data+pPrev->Bytes) == (u32)pItem)//地址是连续的，进行合并
		{
			pPrev->Bytes+=(QH_ITEM_COMM_BYTES+pItem->Bytes);
			pPrev->pNext=pItem->pNext;

			pItem=pPrev->pNext;
			if(pItem) pItem->pPrev=pPrev;
			
			pItem=pPrev;
		}
	}

	//MemSet(pItem->Data,0,pItem->Bytes);//保证下一次分配出去的也是0
}

#if Q_HEAP_DEBUG > 1
void *_Q_Malloc(u16 Size,const char *pFuncCaller,u32 Line)
#else
void *_Q_Malloc(u16 Size)
#endif
{
	static bool IsFirst=TRUE;
	QHEAP_ITEM *pItem=NULL;
	u16 *pMem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(IsFirst)
	{
		QHeapInit();
		IsFirst=FALSE;
	}

	if(Size==0) return NULL;

	Q_HEAP_ENTER_CRIT;	
	if(_CheckRwFlag()==FALSE)//检查前后防冲毁标志
	{
		Debug("Heap RwFlag Error!\n\r");
		while(1);
	}

	//计算需求的实际大小
	Size=AlignTo4(Size+2);//加上尾部校验

#if Q_HEAP_DEBUG >1 
	QH_Debug("# Malco %3uB From %s:%u   ",Size,pFuncCaller,Line);
#else
	QH_Debug("# Malco %3uB   ",Size);
#endif

	//找合适的块
	pItem=gpItemHdr;
	while(pItem)
	{
		if(pItem->RwFlag!=Q_HEAP_RW_FLAG)
		{
			Debug("Heap Hd Rw!\n\r");
			while(1);
		}

		if(pItem->MemIsIdle==FALSE && pItem->Data[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)
		{
			Debug("Heap End Rw!\n\r");
			while(1);
		}

		if(pItem->MemIsIdle && pItem->Bytes>=Size)//找到空闲块，必须大于等于申请大小
		{
			pMem=_RequItem(pItem,Size);//征用一个块
			MemSet(pItem->Data,0,pItem->Bytes-2);//尾部预留防冲毁
			
#if Q_HEAP_DEBUG > 1
			pItem->pFunc=pFuncCaller;
			pItem->Line=Line;
#endif

#if Q_HEAP_DEBUG //检查是不是都0，检查标志
			{
				u16 i;

				if(pItem->Bytes < Size)
				{
					Debug("Malloc %uB size error\n\r",Size);
					while(1);
				}

				for(i=0;i<((pItem->Bytes>>1)-1);i++)
				{
					if(pMem[i]!=0) 
					{
						Debug("Malloc %uB not init 0\n\r",Size);
						while(1);
					}
				}

				if(pMem[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)//检查尾部防冲毁
				{
					Debug("Malloc %uB rwflag error\n\r",Size);
					QHeapDebug();
					while(1);
				}
			}
#endif

			Q_HEAP_EXIT_CRIT;
			QH_Debug("p%x\n\r",pMem);
			return pMem;
		}

		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;

	Debug("!!!No Get Heap,Size:%d!!!\n\r",Size);
	QHeapDebug();
	return NULL;
}

#if Q_HEAP_DEBUG > 1
void _Q_Free(void *pMem,const char *pFuncName,u32 Line)
#else
void _Q_Free(void *pMem)
#endif
{
	QHEAP_ITEM *pItem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(pMem==NULL) return;

	Q_HEAP_ENTER_CRIT;
	if(_CheckRwFlag()==FALSE)//检查前后防冲毁标志
	{
		Debug("Heap Free RwFlag Error!\n\r");
		QHeapDebug();
		while(1);
	}

	pItem=(void *)((u32)pMem-QH_ITEM_COMM_BYTES);

#if Q_HEAP_DEBUG >1 
	QH_Debug("# Free  %3uB %s:%u For %s:%u   p%x\n\r",pItem->Bytes,pFuncName,Line,pItem->pFunc,pItem->Line,pMem);
#else
	QH_Debug("# Free  %3uB   p%x\n\r",pItem->Bytes,pMem);
#endif

	if(pItem->MemIsIdle!=FALSE)
	{
		Debug("Heap Free %uB Idle error\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}

	if(pItem->RwFlag!=Q_HEAP_RW_FLAG)//检查头部防冲毁
	{
		Debug("Heap Free %uB Hd Rw!\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}

	if(pItem->Data[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)//检查尾部防冲毁
	{
		Debug("Heap Free %uB End Rw!\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}

	_ReleseItem(pItem);

	Q_HEAP_EXIT_CRIT;
}

//获取空闲空间
u32 QHeapGetIdleSize(void)
{
	QHEAP_ITEM *pItem=NULL;
	u32 TotalBytes=0;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Q_HEAP_ENTER_CRIT;
	pItem=gpItemHdr;
	while(pItem)
	{
		if(pItem->MemIsIdle) TotalBytes+=pItem->Bytes;
		
		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;
	
	return TotalBytes;
}

//检查该地址是否属于堆空间
bool IsHeapRam(void *pMem)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		if(((u32)pMem>=(u32)pQHeap[i]) && ((u32)pMem<(u32)pQHeapEnd[i])) return TRUE;
	}
	
	return FALSE;
}

