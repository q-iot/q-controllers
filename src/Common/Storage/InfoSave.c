//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了一套基于spi flash的数据存储机制，可被开发者用于其他项目，减少代码开发量
本存储机制主要用来记录相同类别的数据，非常像数据库中的数据表
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"



//info存储块不应从实际flash的0地址开始，这样是为了避免本页程序中的FlashAddr可能为0

//Addr:指信息存储块的绝对地址
//Site:指信息存储相对位置，从1开始，存储的第1个信息块地址为1，存储的第2个信息块为2
//ID:指信息内部的应用程序指定的id，必须非0
//Idx:指信息存储中有效信息的顺序索引，从1开始，比如第1,3个信息块有效，第2个信息块为废弃块，那么第三个信息块Idx为2，而不是3

typedef enum{
	IBF_Removed=0xe0,//此块无用，信息也已经被删除
	IBF_Vaild=0xf0,//此块信息有效
	IBF_Null=0xfe,//此块空白未使用
}INFO_BLOCK_FLAG;

typedef struct{
	INFO_BLOCK_FLAG Flag;///用来表示此信息块存在还是被删除
	INFO_TYPE Type;//类型
	u16 Resv;
}INFO_ITEM_HEADER;

typedef struct{
	INFO_BLOCK_FLAG Flag;///用来表示此信息块存在还是被删除
	INFO_TYPE Type;//类型
	u16 Resv;
	INFO_ID AppID;//由用户定义，与info结构无关，必须放到用户数据的最前面
}INFO_HEADER_AND_ID;

typedef struct{
	u32 StartAddr;//起始存储地址
	u32 StartSec;//起始扇区
	u32 SectorNum;//占用扇区数
	u32 UnitSize;//单元字节数，含4个头字节
	u32 UnitTotal;//单元总数
}INFO_BLOCK_ATTRIB;//存储块信息

typedef struct{
	INFO_TYPE Type;
	INFO_BLOCK Block;
	u16 ItemBytes;//必须小于等于INFO_BLOCK_ATTRIB.ItemBytes-4
}INFO_TYPE_ATTRIB;

//存储区，每个存储区保有最少1024个可供存储区域
//每个扇区64kB，即256页
static const INFO_BLOCK_ATTRIB gBlockAttrib[IBN_MAX]={ //每个存储块信息
//StartAddr			StartSec		SectorNum			UnitSize		UnitTotal
{(FM_INFOSAVE_BASE_SECTOR)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR,		1,	FLASH_PAGE_SIZE/8	,	2048},//IBN_32B
{(FM_INFOSAVE_BASE_SECTOR+1)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+1,	2,	FLASH_PAGE_SIZE/4	,	2048},//IBN_64B
{(FM_INFOSAVE_BASE_SECTOR+3)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+3,	4,	FLASH_PAGE_SIZE/2	,	2048},//IBN_128B
{(FM_INFOSAVE_BASE_SECTOR+7)*FLASH_SECTOR_BYTES,		FM_INFOSAVE_BASE_SECTOR+7,	8,	FLASH_PAGE_SIZE,		2048},//IBN_256B
{(FM_INFOSAVE_BASE_SECTOR+15)*FLASH_SECTOR_BYTES,	FM_INFOSAVE_BASE_SECTOR+15,	16,	FLASH_PAGE_SIZE*2	,	2048},//IBN_512B
};

static const INFO_TYPE_ATTRIB gTypeAttrib[IFT_MAX]={ //每个类型的信息
//Type		Block		ItemBytes
{IFT_STR,				IBN_64B,		sizeof(STR_RECORD)},
{IFT_VARIABLE,	IBN_128B,	sizeof(VARIABLE_RECORD)},
{IFT_RF_DATA,		IBN_256B,	sizeof(RF_RECORD)},
{IFT_IR_DATA,		IBN_256B,	sizeof(IR_RECORD)},
{IFT_KEYS,			IBN_32B,		sizeof(KEYS_RECORD)},
{IFT_DEV,				IBN_128B,	sizeof(DEVICE_RECORD)},
{IFT_TRIGGER,		IBN_128B,	sizeof(TRIGGER_RECORD)},
{IFT_SCENE,			IBN_512B,	sizeof(SCENE_RECORD)},
};


//寻址缓存表，将每个item的类型存放此处，0表示已删除，0xff表示未用
static INFO_TYPE gpTypeMapB32[2048];//数组大小请对应gBlockAttrib修改
static INFO_TYPE gpTypeMapB64[2048];//数组大小请对应gBlockAttrib修改
static INFO_TYPE gpTypeMapB128[2048];//数组大小请对应gBlockAttrib修改
static INFO_TYPE gpTypeMapB256[2048];//数组大小请对应gBlockAttrib修改
static INFO_TYPE gpTypeMapB512[2048];//数组大小请对应gBlockAttrib修改
static INFO_TYPE *gpTypeMap[IBN_MAX]={gpTypeMapB32,gpTypeMapB64,gpTypeMapB128,gpTypeMapB256,gpTypeMapB512};

#if 1 //索引表相关，必须放到map建立之后使用
//根据读出的存储内容，建立map
static void BuildTypeMap(INFO_BLOCK Block)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_ITEM_HEADER Header;
	u32 Unit;

	MemSet(pMap,IFT_IDLE,gBlockAttrib[Block].UnitTotal);

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			pMap[Unit]=IFT_IDLE;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			pMap[Unit]=Header.Type;
		}
		else if(Header.Flag==IBF_Removed)
		{
			pMap[Unit]=IFT_NOVALID;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}
}

//修改索引标记
static void SetItemMapFlag(INFO_BLOCK Block,INFO_SITE Site,INFO_TYPE Type)
{
	gpTypeMap[Block][Site]=Type;
}

//得到真实地址
static INFO_ADDR SiteToAddr(INFO_BLOCK Block,INFO_SITE Site)
{
	return gBlockAttrib[Block].StartAddr+Site*gBlockAttrib[Block].UnitSize;
}

//根据索引查找位置
static INFO_SITE FindItemByIdx(INFO_BLOCK Block,INFO_TYPE Type,INFO_IDX Idx)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	u16 Site;
	u16 Cnt=0;
	
	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site]==Type)
		{
			if(++Cnt==Idx)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}

//根据app id找寻项目，要读取flash，所以比较费时间
static INFO_SITE FindItemByAppID(INFO_BLOCK Block,INFO_TYPE Type,INFO_ID AppID)
{
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_HEADER_AND_ID Header;
	u16 Site;
	
	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site]==Type)//找到类型了
		{
			u32 Addr=SiteToAddr(Block,Site);

			SpiFlsReadData(Addr,sizeof(Header),(void *)&Header);
			if(Header.AppID==AppID)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}
#endif


#if 1 //内部函数
//统计各种类型info个数
static void StatsInfoItemNum(INFO_BLOCK Block,u16 *pIdleNum,u16 *pValidNum,u16 *pRemovedNum)
{
	INFO_ITEM_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			(*pIdleNum)++;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			(*pValidNum)++;
		}
		else if(Header.Flag==IBF_Removed)
		{
			(*pRemovedNum)++;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}

	//Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u[%u/%u]\n\r",Block,*pIdleNum,*pValidNum,*pRemovedNum,MainAddr/FLASH_SECTOR_BYTES,gBlockAttrib[Block].SectorNum,(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,FALSE)),(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,TRUE)));
}

//检查存储块
//如果存储块未被格式化，返回FALSE
//如果存储块已被格式化，返回TRUE
static bool CheckBlockFat(INFO_BLOCK Block)
{
	INFO_ITEM_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag!=IBF_Removed && Header.Flag!=IBF_Vaild && Header.Flag!=IBF_Null)
		{
			return FALSE;
		}		
	}

	return TRUE;
}

//检查存储块是否全为0xff
static bool CheckBlockIdle(INFO_BLOCK Block)
{
	u32 j,Addr,EndAddr=(gBlockAttrib[Block].StartSec+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 *pBuf=Q_Malloc(FLASH_PAGE_SIZE);

	for(Addr=gBlockAttrib[Block].StartAddr;Addr<EndAddr;Addr+=FLASH_PAGE_SIZE)//逐页读取
	{
		SpiFlsReadData(Addr,FLASH_PAGE_SIZE,(void *)pBuf);

		for(j=0;j<(FLASH_PAGE_SIZE>>2);j++)
		{
			if(pBuf[j]!=0xffffffff)
			{
				Q_Free(pBuf);
				return FALSE;
			}		
		}
	}

	Q_Free(pBuf);
	return TRUE;
}

//擦除并格式化某区
static void FromatBlock(INFO_BLOCK Block)
{
	if(CheckBlockIdle(Block)!=TRUE)//非全空
	{
		u32 Now;
		Debug("FromatBlock[%u] EraseSec:%u-%u,",Block,gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
		Now=GetNowMs();
		SpiFlsEraseSector(gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
		Debug("Finish %dmS\n\r",GetNowMs()-Now);
	}

	//开始格式化
	{
		INFO_BLOCK_FLAG Flag=IBF_Null;
		u32 Unit;

		for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条设置信息头
		{
			SpiFlsWriteData(gBlockAttrib[Block].StartAddr+Unit*gBlockAttrib[Block].UnitSize,sizeof(INFO_BLOCK_FLAG),&Flag);
		}
	}

	//标记map
	MemSet(gpTypeMap[Block],IFT_IDLE,gBlockAttrib[Block].UnitTotal);
}

//挤掉空隙，重新存储
//利用flash可以只擦除4k的特点
//此过程不能断电
//完成后会建立map
static void FlushBlock(INFO_BLOCK Block)
{
	u32 EndAddr=(gBlockAttrib[Block].StartSec+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 UnitSize=gBlockAttrib[Block].UnitSize;
	u32 UintNum=(FLASH_MIN_SEC_BYTES/gBlockAttrib[Block].UnitSize);
	u32 DstAddr=gBlockAttrib[Block].StartAddr;
	u8 *pSecBuf=Q_Malloc(FLASH_MIN_SEC_BYTES);
	INFO_TYPE *pMap=gpTypeMap[Block];
	INFO_ITEM_HEADER *pBuf;
	u32 MapIdx=0,Unit,Addr;

	MemSet(pMap,IFT_IDLE,gBlockAttrib[Block].UnitTotal);//清空map
			
	for(Addr=gBlockAttrib[Block].StartAddr;Addr<EndAddr;Addr+=FLASH_MIN_SEC_BYTES)
	{
		SpiFlsReadData(Addr,FLASH_MIN_SEC_BYTES,(void *)pSecBuf);//每4k为一个单位，全部拷贝到buf中
		SpiFlsEraseMinSec(Addr);//擦除4k扇区

		for(Unit=0,pBuf=(void *)pSecBuf;Unit<UintNum;Unit++,pBuf=(void *)(UnitSize+(u32)pBuf))//逐个unit检查，拷贝有效unit到扇区中
		{
			if(pBuf->Flag==IBF_Vaild)//仅拷贝有效块
			{
				SpiFlsWriteData(DstAddr,UnitSize,(void *)pBuf);
				DstAddr+=UnitSize;
				pMap[MapIdx++]=pBuf->Type;//标记map
			}		
		}		
	}

	//剩余的空间全部格式化
	{
		INFO_BLOCK_FLAG Flag=IBF_Null;

		for(;DstAddr<EndAddr;DstAddr+=UnitSize)//逐条设置信息头
		{
			SpiFlsWriteData(DstAddr,sizeof(INFO_BLOCK_FLAG),&Flag);
		}
	}	

	Q_Free(pSecBuf);
}

//Force是否强制整理
//如果占用废弃块太多，就重新整理存储区
//如果发生整理，返回true
//完成后，会建立map
static bool TidyBlock(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	u16 Total=gBlockAttrib[Block].UnitTotal;

	//统计个数
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);

	if((IdleNum+ValidNum+RemovedNum)!=Total)
	{
		Debug("Block Flag Num Is Error!\n\r");
	}

	//检查是否需要整理
	if(IdleNum<(Total>>2) && RemovedNum>(Total>>3))//空白单元数目小于总体四分之一，并且删除单元数大于总体八分之一，就整理
	{
		u32 Now;
		Debug("Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u ... ",Block,IdleNum,ValidNum,RemovedNum);
		Now=GetNowMs();
		FlushBlock(Block);
		Debug("Finish by %umS\n\r",GetNowMs()-Now);
		return TRUE;
	}
	else
	{
		Debug("No Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u\n\r",Block,IdleNum,ValidNum,RemovedNum);
		BuildTypeMap(Block);//建立索引表，方便后期操作
		return FALSE;
	}
}
#endif

//展示所有信息
void DebugInfoSave(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	INFO_TYPE *pMap;
	u16 Unit;

	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	pMap=gpTypeMap[Block];
	
	//展示占用数，剩余数等等
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);
	
	Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u\n\r",Block,IdleNum,ValidNum,RemovedNum,gBlockAttrib[Block].StartSec,gBlockAttrib[Block].SectorNum);
	
	//展示每个区域map
	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)
	{
		switch(pMap[Unit])
		{
			case IFT_IDLE:
				Debug("_");
				break;
			case IFT_NOVALID:
				Debug("*");
				break;
			default:
				Debug("%x",pMap[Unit]);
		}

		if(Unit%64==(64-1)) Debug("\n\r");
	}

	OS_ExitCritical();
}

//遍历存储区，看是否要格式化，即全部flag全部变为0xfe
//检查每个存储块不符合就格式化
//检查存储区冗余情况，必要时整理存储区，建立存储区type映射表
//ForceClean==TRUE恢复出厂设置
//删除块过多时，自动整理
//任何时候都可以调用此函数用来缩减存储区大小
void InfoBuildBlock(INFO_BLOCK Block,bool ForceClean)
{
	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	if(ForceClean)//强制恢复出厂设置
	{
		FromatBlock(Block);//擦除并格式化
	}
	else //自检
	{
		if(CheckBlockFat(Block)==TRUE)//正常情况下，应该是格式化状态(有效区)
		{
			TidyBlock(Block);//整理存储区
		}
		else
		{
			FromatBlock(Block);//擦除并格式化1区
		}	
	}

	OS_ExitCritical();
}

//初始化info数据库
//ForceClean==TRUE恢复出厂设置
//删除块过多时，自动整理
//任何时候都可以调用此函数用来缩减所有存储区大小
void InfoSaveInit(bool ForceClean)
{
	INFO_BLOCK Block;
	INFO_TYPE Type;
	
#if 1	//检查参数
	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		if(gBlockAttrib[Block].UnitTotal > gBlockAttrib[Block].SectorNum*FLASH_SECTOR_BYTES/gBlockAttrib[Block].UnitSize)
		{
			Debug("INFO_BLOCK %u UnitTotal is too big!\n\r",Block);
			while(1);
		}

		if(gBlockAttrib[Block].StartAddr!=gBlockAttrib[Block].StartSec*FLASH_SECTOR_BYTES)
		{
			Debug("INFO_BLOCK %u StartAddr or StartSec error!\n\r",Block);
			while(1);
		}

		if(Block!=(INFO_BLOCK)0)
		{
			if(gBlockAttrib[Block].StartSec!=gBlockAttrib[Block-1].StartSec+gBlockAttrib[Block-1].SectorNum)
			{
				Debug("INFO_BLOCK %u StartSec error!\n\r",Block);
				while(1);
			}
		}		
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		if(gTypeAttrib[Type].Type!=Type)
		{
			Debug("INFO_TYPE %u Type is error!\n\r",Type);
			while(1);
		}
		
		if((gTypeAttrib[Type].ItemBytes+sizeof(INFO_ITEM_HEADER))>gBlockAttrib[gTypeAttrib[Type].Block].UnitSize)
		{
			Debug("INFO_TYPE %u Param is error! %u\n\r",Type,gTypeAttrib[Type].ItemBytes+sizeof(INFO_ITEM_HEADER));
			while(1);
		}

		if(gTypeAttrib[Type].ItemBytes % 4)
		{
			Debug("INFO_TYPE %u ItemBytes is error!\n\r",Type);
			while(1);
		}
	}		
#endif 

	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		InfoBuildBlock(Block,ForceClean);
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		Debug("[%s]:%u\n\r",gNameInfoName[Type],GetTypeInfoTotal(Type));	
	}

	Debug("\n\r");
}

//添加新info信息到flash，返回绝对存储位置
//返回INFO_RES_SPACE_FULL表示没空间
INFO_ADDR SaveInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;

	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,IFT_IDLE,1);
	if(Site<0)
	{
		OS_ExitCritical();
		return INFO_SPACE_FULL;	//资源不足
	}
	
	SetItemMapFlag(Block,Site,Type);//修改索引标记
	Addr=SiteToAddr(Block,Site);//得到真实地址

	Header.Flag=IBF_Vaild;
	Header.Type=Type;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	if(pData!=NULL) SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return Addr;
}

//删除flash里指定位置的info信息
//返回原来的绝对存储位置
INFO_ADDR DeleteInfo(INFO_TYPE Type,INFO_ID AppID)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	if(AppID==0) return INFO_PARAM_ERROR;

	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return 0;
	}
	
	SetItemMapFlag(Block,Site,IFT_NOVALID);//修改索引标记
	Addr=SiteToAddr(Block,Site);//得到真实地址

	Header.Flag=IBF_Removed;
	Header.Type=IFT_IDLE;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	
	OS_ExitCritical();
	return Addr;
}

//覆盖相同appid的info信息到flash，返回绝对存储位置
//不做数据对比和校验
//由于flash只能由1变0的特性，如果数据不对，可能存储结果并非程序预期
INFO_ADDR CoverInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_ITEM_HEADER Header;
	INFO_ID AppID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&AppID,pData,sizeof(INFO_ID));//拷贝id

	if(AppID==0) return INFO_PARAM_ERROR;
		
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return INFO_PARAM_ERROR;
	}	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return Addr;
}

//将实际数据的头4字节作为appid，轮询存储块，匹配时返回信息
//返回读取大小
u16 ReadInfoByAppID(INFO_TYPE Type,INFO_ID AppID,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;
	if(AppID==0) return 0;
	
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByAppID(Block,Type,AppID);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_ITEM_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//根据索引顺序读取info信息
//返回读取大小
u16 ReadInfoByIdx(INFO_TYPE Type,INFO_IDX Idx,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;

	Block=gTypeAttrib[Type].Block;

	if(Idx==0 || Idx>gBlockAttrib[Block].UnitTotal) return 0;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,Type,Idx);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_ITEM_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//获取信息总数
u16 GetTypeInfoTotal(INFO_TYPE Type)
{
	INFO_TYPE *pMap;
	INFO_BLOCK Block=gTypeAttrib[Type].Block;
	s32 i,Num=0;

	if(Type>=IFT_MAX) return 0;

	OS_EnterCritical();
	pMap=gpTypeMap[Block];
	
	for(i=0;i<gBlockAttrib[Block].UnitTotal;i++)
	{
		if(pMap[i]==IFT_IDLE) break;
		if(pMap[i]==Type) Num++;
	}	

	OS_ExitCritical();
	return Num;
}

//获取信息大小
u16 GetTypeItemSize(INFO_TYPE Type)
{
	if(Type>=IFT_MAX) return 0;
	
	return gTypeAttrib[Type].ItemBytes;
}

//获取空白单元数
u16 GetBlockFreeUnit(INFO_BLOCK Block)
{
	INFO_TYPE *pMap;
	s32 i,Num=0;

	if(Block>=IBN_MAX) return 0;

	OS_EnterCritical();
	pMap=gpTypeMap[Block];

	for(i=gBlockAttrib[Block].UnitTotal-1;i>=0;i--,Num++)//从屁股往前数空白格即可
	{
		if(pMap[i]!=IFT_IDLE) break;
	}	

	OS_ExitCritical();
	return Num;
}

//获取指定类型的空闲数目
u16 GetTypeFreeUnit(INFO_TYPE Type)
{
	return GetBlockFreeUnit(gTypeAttrib[Type].Block);
}

//获取指定类型的块大小
u16 GetTypeBlockSize(INFO_TYPE Type)
{
	return 1<<(5+gTypeAttrib[Type].Block);
}

