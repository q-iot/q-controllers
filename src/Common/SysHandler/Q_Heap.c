#include "Drivers.h"
#include "PublicFunc.h"
#include "Q_Heap.h"
#include "Product.h"


//概述:这是一个简单的堆管理机制
//其主要思想是将内存分配为若干个最小单元，申请内存时，会分配整数个最小单元的内存
//因为将内存分配为最小单元了，所以用来记录内存块索引的变量只需要16位即可
//如果最小单元是4byte，那么此代码可以管理的堆大小为0xffff的4倍
//但实际上，16bit里面，最高1位用来表示内存块是否被使用，所以实际大小是0x7fff的4倍，
//大约为130k的空间。如果需要管理更大空间，只需要提高最小单元的Byte数目即可。
//此堆管理机制的额外开销有2部分
// 1.需要一个记录表用来记录内存块信息(定义类型HEAP_RECORD)，每一块内存对应一个记录表成员
//   这个记录表的大小MAX_RECORD_NUM可以根据实际情况修改
// 2.当申请的空间不是最小单元的整数倍+2时，会有多余的内存消耗，最后2个字节用于存放防冲标记。
//   例如当最小单元是4byte，如果申请大小为7，则实际分配的内存是12，为4的整数倍。
//在堆分配时，为节省分配时间，此机制不负责将申请空间清零
//该堆管理机制没有解决的问题:内存碎片

//此堆管理可用于操作系统，也可以用于裸奔程序中
//如用于裸奔程序，需修改如下几步
// 1.修改堆大小，这个根据实际情况来，只需要修改Q_HEAP_SIZE_BYTE这个宏就行了
// 2.修改记录块大小，每分配一次内存，会耗费一个记录块，可根据实际情况修改MAX_RECORD_NUM宏即可
// 3.将三个关于临界区的宏定义成空的就行了:Q_HEAP_CRIT_SAVE_DATA_DEFINE、Q_HEAP_ENTER_CRIT、Q_HEAP_EXIT_CRIT
// 4.如果需要管理的空间很大，那么你需要修改最小单元来适应，相关宏Q_HEAP_MINI_UNIT、Q_HEAP_UNIT_OFFSET

#define Q_HEAP_TRACK_DEBUG Q_HEAP_DEBUG

#ifndef NULL
#define NULL ((void *)0)
#endif

#if Q_HEAP_TRACK_DEBUG > 1
#define Need_Debug 1
#else
#define Need_Debug 0
#endif

#if Need_Debug
#define QH_Debug Debug
#else
#define QH_Debug(x,y...)
#endif

#define Q_HEAP_USE_MALLCO_ALIGN8  1//开启后分配的内存将8字节对齐

//为保证多线程下堆分配的原子操作，必须借助操作系统临界区
//如果此机制不被用于多线程，则可以定义下面三个个宏为空白
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	IntSaveInit()
#define Q_HEAP_ENTER_CRIT EnterCritical()
#define Q_HEAP_EXIT_CRIT LeaveCritical()

//用来验证内存是否被冲毁的标识
//占用2个字节，所以内存前后被冲毁而不能察觉的概率为65535分之1
#define Q_HEAP_UNIT_REWRITE_ID 0x5808 //放置于每个被分配内存块末尾的冲毁标识
#define Q_HEAP_REWRITE_ID ((Q_HEAP_UNIT_REWRITE_ID<<16)|0x1234) //整个内存区域前后的冲毁标识，将Q_HEAP_UNIT_REWRITE_ID放到高16位是为了防止第一块分配块被释放造成的检查问题

//堆栈总大小，因为只用16bit表示内存块索引，所以实际大小必须小于(0xffff*内存单元大小)
#if PRODUCT_IS_FLY_0XX
#define Q_HEAP_SIZE_BYTE (16*1024)
#else
#define Q_HEAP_SIZE_BYTE (6*1024-32) //留点空间给尾部的footer字段
#endif

//下面两个宏必须同时修改
#define Q_HEAP_MINI_UNIT 4 //内存单元大小，单位Byte，考虑到32位处理器的4字节对齐，此值必须为4倍数
#define Q_HEAP_UNIT_OFFSET 2 // 2的Q_HEAP_UNIT_OFFSET次方必须等于Q_HEAP_MINI_UNIT ,用于移位代替除法或乘法
#define Q_HEAP_UNIT_MASK 0x03//用于和运算代替余运算

#define QH_UNIT_MAX_NUM (Q_HEAP_SIZE_BYTE/Q_HEAP_MINI_UNIT)//单元个数

//内存块索引
#define QH_UnitIdxMask 0x7fff //用低15位表示内存块的索引，以单元为单位。最高位表示是否使用
#define QH_GetUnitIdx(x) (gQHeapRecord[x].UnitIdx&QH_UnitIdxMask)//x为记录表序号，返回内存块的第一个单元索引
#define QH_SetUnitIdx(x,Index) {gQHeapRecord[x].UnitIdx=((gQHeapRecord[x].UnitIdx&(~QH_UnitIdxMask))|((Index)&QH_UnitIdxMask));}
#define QH_GetUnitSize(x) (QH_GetUnitIdx(QH_GetNext((x)))-QH_GetUnitIdx((x)))//x为记录表序号，返回内存块占用单元数
//#define UnitIdxEnd QH_UNIT_MAX_NUM

//操作指定块的读标志，入口参数x为记录表序号
#define QH_UsedMask 0x8000
#define QH_UNUSE 0x0000
#define QH_USED 0x8000
#define QH_GetUsedFlag(x) (gQHeapRecord[x].UnitIdx&(QH_UsedMask))//返回非0表示被使用，返回0表示空闲
#define QH_SetUsedFlag(x,v) {gQHeapRecord[x].UnitIdx=((gQHeapRecord[x].UnitIdx&(~QH_UsedMask))|(v?QH_USED:QH_UNUSE));}

//操作链表，根据记录表序号获取下一个块对应记录表的序号
#define QH_GetNext(x) (gQHeapRecord[x].Next)
#define QH_SetNext(x,v) {gQHeapRecord[x].Next=(v);}
#define QH_GetPrev(x) (gQHeapRecord[x].Prev)
#define QH_SetPrev(x,v) {gQHeapRecord[x].Prev=(v);}

//清除指定块的记录
#define QH_CleanRecord(x) {gQHeapRecord[x].Prev=gQHeapRecord[x].Next=gQHeapRecord[x].UnitIdx=0;}

//For Track
#if Q_HEAP_TRACK_DEBUG
#define QH_SetRecdName(x,p) gQHeapRecord[x].pFuncName=p
#define QH_SetRecdLines(x,s) gQHeapRecord[x].Lines=s
#define QH_GetRecdName(x) gQHeapRecord[x].pFuncName
#define QH_GetRecdLines(x) gQHeapRecord[x].Lines
#else
#define QH_SetRecdName(x,p)
#define QH_SetRecdLines(x,s)
#define QH_GetRecdName(x)
#define QH_GetRecdLines(x)
#endif

//单元和字节的转换
#define QH_Unit2Byte(U) ((U)<<Q_HEAP_UNIT_OFFSET) //转换单元数到字节数,用位移代替乘法
#define QH_Byte2Unit(B) ((B)?((((B)-1)>>Q_HEAP_UNIT_OFFSET)+1):0) //转换字节数到占用单元数，用位移代替除法

typedef struct
{
	u32 Align8Rsv;//用于8字节对齐
	u32 Header;//for align and sign
	u16 Heap[ Q_HEAP_SIZE_BYTE / 2 ];//mem
	u32 Footer;//
}QHS_STRUCT;
static QHS_STRUCT QHS;

typedef struct {
	u8 Prev;//记录上一个内存块的记录表序号
	u8 Next;//记录下一个内存块的记录表序号
	u16 UnitIdx;//记录内存块起始单元位置
	u16 MagicCode;//用来区分分配者的数字
#if Q_HEAP_TRACK_DEBUG
	u8 *pFuncName;
	u16 Lines;
#endif
}Q_HEAP_RECORD;
#define Q_HEAP_MAX_RECORD_NUM 256
#define QH_RECORD_START 0
#define QH_RECORD_END (Q_HEAP_MAX_RECORD_NUM-1)
static Q_HEAP_RECORD gQHeapRecord[Q_HEAP_MAX_RECORD_NUM];//分配记录表

//用于调试
void DebugHeap(void)
{
	u16 Index;//表示块的起始单元
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;
	
	Debug("\n\r--------------Heap Record--0x%08x----R:0x%08x-0x%08x--------\n\r",(u32)QHS.Heap,(u32)gQHeapRecord,(u32)gQHeapRecord+Q_HEAP_MAX_RECORD_NUM*sizeof(Q_HEAP_RECORD));
	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUsedFlag(Index))
		{
			DebugCol(" %3d ",Index);
		}
		else
		{
			Debug("[%3d]",Index);
		}
		
		Debug(" Unit %4u-%4u, Addr %04x - %04x,%5uB, %3u-> Me->%3u",
			QH_GetUnitIdx(Index),QH_GetUnitIdx(QH_GetNext(Index)),
			LBit16((u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(Index))),LBit16((u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(QH_GetNext(Index)))),
			QH_Unit2Byte(QH_GetUnitSize(Index)),
			QH_GetPrev(Index),QH_GetNext(Index));
#if Q_HEAP_TRACK_DEBUG
		if(QH_GetUsedFlag(Index)) Debug(", %s:%u",QH_GetRecdName(Index),QH_GetRecdLines(Index));
#endif
		Debug("\n\r");

		if(QH_GetNext(Index)==QH_RECORD_END) break;
	}
	Q_HEAP_EXIT_CRIT;
	Debug("--------------Heap Record End--0x%08x---------\n\r",(u32)QHS.Heap+Q_HEAP_SIZE_BYTE);
}

void QS_HeapInit(void)
{
	u16 Index;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if( (QH_Unit2Byte(1)!=Q_HEAP_MINI_UNIT)||(QH_Byte2Unit(Q_HEAP_MINI_UNIT)!=1) )
	{
		Debug("Q_HEAP_MINI_UNIT & Q_HEAP_UNIT_OFFSET Marco Define Error!!!\n\r");
		while(1);
	}
		
	Q_HEAP_ENTER_CRIT;
	for(Index=0;Index<(sizeof(gQHeapRecord)/sizeof(Q_HEAP_RECORD));Index++)
	{
		QH_CleanRecord(Index);
	}

	//建立头记录
	QH_SetPrev(QH_RECORD_START,QH_RECORD_END);
	QH_SetNext(QH_RECORD_START,QH_RECORD_END);
	QH_SetUsedFlag(QH_RECORD_START,QH_UNUSE);
	QH_SetUnitIdx(QH_RECORD_START,0);
#if Q_HEAP_TRACK_DEBUG
	QH_SetRecdName(QH_RECORD_START,"<Idle>");
	QH_SetRecdLines(QH_RECORD_START,0);
#endif

	//建立尾记录
	QH_SetPrev(QH_RECORD_END,QH_RECORD_START);
	QH_SetNext(QH_RECORD_END,QH_RECORD_START);
	QH_SetUsedFlag(QH_RECORD_END,QH_USED);
	QH_SetUnitIdx(QH_RECORD_END,QH_UNIT_MAX_NUM);
#if Q_HEAP_TRACK_DEBUG
	QH_SetRecdName(QH_RECORD_END,"<Idle>");
	QH_SetRecdLines(QH_RECORD_END,0);
#endif

	//建立冲内存检查的头标记
	QHS.Header=Q_HEAP_REWRITE_ID;
	QHS.Footer=Q_HEAP_REWRITE_ID;
	
	//DebugHeap();
	Q_HEAP_EXIT_CRIT;
}

//插入一个存储块记录到记录表中
//Index:将插入到此记录,
//Index必须是个记录空内存的记录块
static bool InsertRecord(u16 Index,u16 NeedUnit)
{	
	u16 i;

	//QH_Debug("  ##Insert UnitNum %d @ Idx %d\n\r",NeedUnit,Index);

	if(NeedUnit<QH_GetUnitSize(Index))//需要的空间小于空闲空间
	{
		//重新建立一个内存记录块
		for(i=(QH_RECORD_START+1);i<QH_RECORD_END;i++)
		{
			if(QH_GetUnitIdx(i)==0)
			{
				//设置新内存块，这是一个空闲块
				QH_SetPrev(i,Index);//prev设置
				QH_SetNext(i,QH_GetNext(Index));//next转移
				QH_SetUnitIdx(i,QH_GetUnitIdx(Index)+NeedUnit);
				QH_SetUsedFlag(i,QH_UNUSE);

				//处理原本空闲的内存块之后的内存块
				QH_SetPrev(QH_GetNext(Index),i);
				
				//处理要使用的内存块
				QH_SetUsedFlag(Index,QH_USED);//标记使用标志
				QH_SetNext(Index,i);//next转移
				
#if Need_Debug
				//DebugHeap();
#endif
				return TRUE;
			}
		}
		
		if(i==QH_RECORD_END)
		{
			Debug("!!!Record Num is not enough!!!\n\r");
		}
		return FALSE;
	}
	else //如果需要插入的内存块和空闲内存块刚好相等
	{
		QH_SetUsedFlag(Index,QH_USED);//标记被使用
		return TRUE;
	}
}

//UnitIdx为起始单元索引，以HEAP_MINI_UNIT为单位
//返回0表示错误，否则返回free的内存块大小
#if Q_HEAP_TRACK_DEBUG
static u16 DeleteRecord(u16 UnitIdx,u8 *pFuncName,u32 Lines)
#else
static u16 DeleteRecord(u16 UnitIdx)
#endif
{
	u16 Index,Size=0;
	u16 i;

	//Debug("DeleteRecord Unit 0x%x\n\r",UnitIdx);

	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUnitIdx(Index)==UnitIdx)//找到匹配项了
		{
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d (For %s %d)\n\r",pFuncName,Lines,QH_GetRecdName(Index),QH_GetRecdLines(Index));
#endif

#if Q_HEAP_TRACK_DEBUG
			QH_SetRecdName(Index,"<Idle>");
			QH_SetRecdLines(Index,0);
#endif

			if(QH_GetUsedFlag(Index)==QH_UNUSE) break;//未使用的块，不能被free
			
			Size=QH_Unit2Byte(QH_GetUnitSize(Index));//获取内存块大小
			
			QH_SetUsedFlag(Index,QH_UNUSE);//设置当前为未使用
			
			//先看前面一个是不是空的
			i=QH_GetPrev(Index);
			if(QH_GetUsedFlag(i)==QH_UNUSE)//前面一个是空的，合并前面一个
			{
				QH_SetNext(i,QH_GetNext(Index));
				QH_SetPrev(QH_GetNext(Index),i);
				QH_CleanRecord(Index);		
				Index=i;
			}

			//再看后面一个是不是空的
			i=QH_GetNext(Index);
			if(QH_GetUsedFlag(i)==QH_UNUSE)
			{
				QH_SetNext(Index,QH_GetNext(i));
				QH_SetPrev(QH_GetNext(i),Index);
				QH_CleanRecord(i);
			}

#if Need_Debug
			//DebugHeap();
#endif
			return Size;
		}
		
		if(QH_GetNext(Index)==QH_RECORD_END) break;//到了最后一个块了，退出。
	}

	return 0;	
}

#if Q_HEAP_TRACK_DEBUG
void *_Q_Malloc(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *_Q_Malloc(u16 Size)
#endif
{
	u16 UnitNum,Index,AddSize;
	u16 *Mem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(QHS.Header!=Q_HEAP_REWRITE_ID||QHS.Footer!=Q_HEAP_REWRITE_ID)
	{
		Debug("Heap Be Rewrite!\n\r");
		while(1);
	}
	
	if(Size==0) return NULL;

	UnitNum=QH_Byte2Unit(Size);//计算要几个单元
	
	//多加一个单元，用来验证内存头尾是否被冲毁。
	//如果Size本来就比最小单元的整数倍小2或者小3，就不用多余的一个单元了。
	AddSize=Size&Q_HEAP_UNIT_MASK;
	if(AddSize==0||(Q_HEAP_MINI_UNIT-AddSize)<2) UnitNum++;

#if Q_HEAP_USE_MALLCO_ALIGN8 && Q_HEAP_MINI_UNIT<8 //是否启用分配内存8字节对齐
	//由于第一块内存已经被设置为8字节对齐地址，所以为了8字节对齐，每块分出去的内存大小都必须是8的倍数
	if((UnitNum<<Q_HEAP_UNIT_OFFSET)&0x07)//筛选出未8字节对齐的
	{
		UnitNum++;//此处有个前提，就是必须内存单元的大小是4
	}	
#endif

	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUsedFlag(Index)==QH_UNUSE)//找到一个空的单元
		{
			if(QH_GetUnitSize(Index)>=UnitNum)
			{
				if(InsertRecord(Index,UnitNum)==TRUE)
				{
					Mem=(void *)((u32)QHS.Heap+QH_Unit2Byte(QH_GetUnitIdx(Index)));
					Mem[(QH_Unit2Byte(UnitNum)>>1)-1]=Q_HEAP_UNIT_REWRITE_ID;
					QH_Debug("##Malloc Size %d, Addr 0x%x,Unit 0x%x\n\r",QH_Unit2Byte(UnitNum),(u32)Mem,QH_GetUnitIdx(Index));
#if Q_HEAP_TRACK_DEBUG
					QH_SetRecdName(Index,pFuncName);
					QH_SetRecdLines(Index,Lines);
#endif

#if Q_HEAP_TRACK_DEBUG > 1				
					Debug("##Malloc Func:%s Line:%d\n\n\r",pFuncName,Lines);
#endif
					Q_HEAP_EXIT_CRIT;	
					//Debug("  M[%x]%d\n\r\n\r",Mem,Size);

					MemSet(Mem,0,Size);	//初始化归零				
					return (void *)Mem;
				}
				else 
					break;
			}
		}			
		
		if(QH_GetNext(Index)==QH_RECORD_END) 
			break;
	}
	Q_HEAP_EXIT_CRIT;
	
	DebugHeap();
	QS_MonitorFragment();
	Debug("!!!No Get Heap,Size:%d!!!\n\r",Size);
	return 0;
//	OS_SchedLock();
//	while(1);
}

#if Q_HEAP_TRACK_DEBUG
bool _Q_Free(void *Ptr,u8 *pFuncName,u32 Lines)
#else
bool _Q_Free(void *Ptr)
#endif
{	
	u16 *Mem=Ptr;
	u16 Size;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Ptr==NULL) return TRUE;
	
	if(QHS.Header!=Q_HEAP_REWRITE_ID||QHS.Footer!=Q_HEAP_REWRITE_ID)
	{
		Debug("Heap Be Rewrite!\n\r");
		while(1);
	}
	//Debug("F[%x]\n\r",(u32)Ptr);
	
	if(Ptr)
	{
		Q_HEAP_ENTER_CRIT;

#if Q_HEAP_TRACK_DEBUG
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap),pFuncName,Lines);
#else
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap));
#endif

		//Debug("  F[%x]%d\n\r",Ptr,Size);
		
		if(Size==0)
		{
			Debug("##Free Error!!!Can not find the match memory!!!0x%x\n\r",(u32)Ptr);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			//while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}
		else 
		{
			QH_Debug("##Free   Size %d, Addr 0x%x,Unit 0x%x\n\n\r",Size,(u32)Ptr,QH_Byte2Unit((u32)Ptr-(u32)QHS.Heap));
		}
		
		//检查内存冲毁情况
		if(Mem[(Size>>1)-1]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory[%x] end be rewrited!!!Flag %x, Size %d\n\r",Ptr,Mem[(Size>>1)-1],Size);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			DebugHeap();
			while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}

		Mem--;
		if(Mem[0]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory header be rewrited!!![%x]=%x\n\r",(u32)Mem,Mem[0]);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##FuncName:%s Lines:%d\n\r",pFuncName,Lines);
#endif
			DebugHeap();
			while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}
				
		Q_HEAP_EXIT_CRIT;
		return TRUE;
	}
	else
		return FALSE;
}

#if 0
#if Q_HEAP_TRACK_DEBUG
void *_Q_Calloc(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *_Q_Calloc(u16 Size)
#endif
{
	void *p;
#if Q_HEAP_TRACK_DEBUG
	p = _Q_Malloc(Size,pFuncName,Lines);
#else
	p = _Q_Malloc(Size);
#endif
	if (p) 
	{
		MemSet(p, 0, Size);
	}
	return p;
}
#endif

//用于内存碎片监控
/*为了描述内存碎片有多少，引入了一个浮点数fragindices来描述它：
fragindices被定义为：
fragindices = (float)MaxSize/(float)TotalSize
其中MaxSize表示堆中最大可分配的连续空闲内存大小
TotalSize表示堆中所有空闲内存的总和
可见当系统无碎片时fragindices = 1
而当系统无内存可分配时fragindices = 0 
fragindices越小表示碎片越多*/
void QS_MonitorFragment(void)
{
	u16 Index,Size;
	u32 MaxSize=0,TotalSize=0;
	
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{ 
		if(QH_GetUsedFlag(Index)==QH_UNUSE)
		{
			Size=QH_Unit2Byte(QH_GetUnitSize(Index));//获取内存块大小
			TotalSize+=Size;
			if(Size>MaxSize)
				MaxSize=Size;
		}
		if(QH_GetNext(Index)==QH_RECORD_END) break;
	}

	Debug("********** Heap Monitor **********\n\r");
	Debug(" TotalFreeMem=%d Byte\n\r",TotalSize);
	Debug(" MaxFreeMem=%d Byte\n\r",MaxSize);
	Debug(" Fragindices=%u.%02uf\n\r",MaxSize/TotalSize,MaxSize*100/TotalSize%100);
	Debug("**********************************\n\r");
}

//用于整理内存碎片
bool QS_HeapArrange(void)
{//unfinish
	return TRUE;
}

//检查该地址是否属于堆空间
bool IsHeapRam(void *p)
{
	if(p == NULL) return FALSE;
	
	if(((u32)p >= (u32)QHS.Heap)&&((u32)p < ((u32)QHS.Heap+Q_HEAP_SIZE_BYTE)))
		return TRUE;

	return FALSE;
}

