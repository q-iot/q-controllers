//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了一套基于spi flash的数据缓存机制，可被开发者用于其他项目，减少代码开发量
本机制的主要目的，是在最大数目的约束下，记录最新的数据，当超过最大约束数目时，
最旧的记录会被删除。缓存的数据，可以以id来获取。
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

#define GetAddr(Site) (gInfoStartAddr[Name]+((Site)-1)*gInfoAttrib[Name].BlockBytes)
#define GetSite(Addr) (((Addr)-gInfoStartAddr[Name])/gInfoAttrib[Name].BlockBytes+1)

typedef struct{
	u8 StartSector;//起始扇区
	u8 BackupSector;//备份扇区
	u8 SectorNum;//占用扇区数
	u16 BlockBytes;//单元字节数，含8个头字节
	u16 MaxItem;//应用程序定义的能容纳的最大记录数，比实际容量小或相等即可
	u16 ChkSizeof;//用于大小检查
}INFO_TEMP_ATTRIB;

//用户定义
#if PRODUCT_IS_JUMPER
static const INFO_TEMP_ATTRIB gInfoAttrib[ITN_MAX]={
{1,5,4,FLASH_PAGE_SIZE,IR_TEMP_MAX_NUM,sizeof(IR_RECORD)},//ir signal buf
{9,13,4,FLASH_PAGE_SIZE,RF_TEMP_MAX_NUM,sizeof(RF_RECORD)},//rf signal buf
};
#endif

//本页使用
#define INFO_NULL 0xffffffff
static INFO_ADDR gInfoStartAddr[ITN_MAX]={0};//存储区起始地址
static INFO_ADDR gInfoEndAddr[ITN_MAX]={0};//存储区结束地址
static u16 gInfoOccupy[ITN_MAX]={0};//通过遍历产生的占有量
static INFO_SITE gIdleSite[ITN_MAX]={1};//空闲位置索引表


//遍历存储体中所有的信息
//并产生实际有效的总数
u16 InfoTempStatistics(INFO_TEMP_NAME Name)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;
	u16 Count=0;

	if(Name>=ITN_MAX) return 0;
	
	gInfoOccupy[Name]=0;

	for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		Count++;
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		
		if(InfoID!=INFO_NULL)//找到有效信息
		{
			gInfoOccupy[Name]=Count;
		}
	}

	return gInfoOccupy[Name];
}


//初始化info数据库
void InfoTempInit(void)
{
	INFO_TEMP_NAME Name;
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	for(Name=(INFO_TEMP_NAME)0;Name<ITN_MAX;Name++)
	{
		if(AlignTo4(gInfoAttrib[Name].ChkSizeof)!=gInfoAttrib[Name].ChkSizeof)
		{
			Debug("[%s]InfoSize is not align 4! sizeof()=%d\n\r",gNameInfoName[Name],gInfoAttrib[Name].ChkSizeof);
			while(1);
		}
	
		if((gInfoAttrib[Name].ChkSizeof)>gInfoAttrib[Name].BlockBytes)
		{
			Debug("[%s]BlockSize is small,pls check it! %d>%d\n\r",gNameInfoName[Name],(gInfoAttrib[Name].ChkSizeof),gInfoAttrib[Name].BlockBytes);
			while(1);
		}

		if((gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes)<gInfoAttrib[Name].MaxItem)
		{
			Debug("[%s]MaxItem is too big.%d<%d\n\r",gNameInfoName[Name],(gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes),gInfoAttrib[Name].MaxItem);
			while(1);
		}
	}

	Debug("Name     Sector(Backup)  Vaild  Deleted  Remainder  Occupation  IdelSite  sizeof\n\r");

	for(Name=(INFO_TEMP_NAME)0;Name<ITN_MAX;Name++)
	{
		u16 Remainder=0,Vaild=0,Deleted=0;
		
		//先找地址
		FlashAddr=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;//读备用区头字节
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID == INFO_NULL)//标志为空，则数据在正常区
		{
			Debug("            %4d        ",gInfoAttrib[Name].StartSector);
			gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
			gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
		}
		else//数据在备用区
		{
			Debug("            %4d(*)     ",gInfoAttrib[Name].BackupSector);
			gInfoStartAddr[Name]=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
			gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
		}
	
		//先检测是不是合法
		for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
		{
			SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);

			if(InfoID == INFO_NULL) Remainder++;
			else if(InfoID == 0) Deleted++;
			else Vaild++;
		}

		//检测空间剩余
		Debug("%4d      %4d      %4d      ",Vaild,Deleted,Remainder);

		if(Remainder*4 < Vaild)
		{
			Debug("Remainder info is too little!Tidy Info System!\n\r");	
		}

		//遍历，建立内容首字节(应用id)和实际存储位置的关系
		gInfoOccupy[Name]=0;
		gIdleSite[Name]=1;
		InfoTempStatistics(Name);
		FindIdleSite(Name);//建立空位置索引表
		Debug("%4d       %4d      %4d",gInfoOccupy[Name],gIdleSite[Name],gInfoAttrib[Name].ChkSizeof);
		
		Debug("\r[%s]\n\r",gNameInfoName[Name]);
	}

	Debug("\n\r");
}

//添加新info信息到flash，返回相对存储位置
//返回INFO_RES_SPACE_FULL表示没空间
INFO_SITE SaveInfoTemp(INFO_TEMP_NAME Name,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(pData==NULL) return INFO_PARAM_ERROR;
	if(*(u32 *)pData==INFO_NULL) return INFO_PARAM_ERROR;
	
	if(Byte > gInfoAttrib[Name].BlockBytes)
	{
		Debug("Info Size Is Too Big!\n\r");
		while(1);
	}
	
	FlashAddr=GetAddr(gIdleSite[Name]);
	if(FlashAddr<gInfoEndAddr[Name]) 
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL)
		{
			SpiFlsWriteData(FlashAddr,Byte,pData);
			if(INFO_SPACE_FULL == FindIdleSite(Name)) return INFO_SPACE_FULL; //更新空位置索引
			return GetSite(FlashAddr);
		}
		else
		{
			Debug("FindIdleSite is not null!S %d %d\n\r",gIdleSite[Name],FlashAddr);
			while(1);
		}
	}
	else
	{
		//Debug("Info Space Full!Rebuild it!S\n\r");
		return INFO_SPACE_FULL;
	}

//	return INFO_SPACE_FULL;
}

//返回INFO_RES_ERROR表示读不到
//返回有效值表示实际存储地址
//Site超过有效部分，会返回错误码
INFO_SITE ReadInfoBySite(INFO_TEMP_NAME Name,INFO_SITE Site,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr=GetAddr(Site);
	
	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(pData==NULL) return INFO_PARAM_ERROR;
	if(Site==0) return INFO_PARAM_ERROR;
	if(Site > gInfoOccupy[Name]) return INFO_SPACE_FULL;
	
	if((FlashAddr < gInfoStartAddr[Name])||(FlashAddr >= gInfoEndAddr[Name])||(FlashAddr%gInfoAttrib[Name].BlockBytes)) return INFO_PARAM_ERROR;
	
	SpiFlsReadData(FlashAddr,Byte,pData);
	return GetSite(FlashAddr);
}

//在指定的类下找appid
//从最后往前部读
INFO_SITE ReadInfoTemp(INFO_TEMP_NAME Name,INFO_ID AppID,void *pData,u16 Byte)
{
	INFO_ADDR FlashAddr=gInfoStartAddr[Name]+(gInfoOccupy[Name]-1)*gInfoAttrib[Name].BlockBytes;
	INFO_ADDR EndAddr=gInfoStartAddr[Name];
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	if(AppID==0) return INFO_PARAM_ERROR;
	
	for(;FlashAddr>=EndAddr;FlashAddr-=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==AppID)
		{
			if(pData!=NULL)SpiFlsReadData(FlashAddr,Byte,pData);
			return GetSite(FlashAddr);
		}
	}

	return INFO_RES_ERROR;
}

u16 InfoTempTotalInc(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	
	gInfoOccupy[Name]++;
	return gInfoOccupy[Name];
}

//整理info temp
//删掉前四分之一的数据，转换到新存储区
void FlushTempInfo(INFO_TEMP_NAME Name)
{
	u16 TotalNum=gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes;
	INFO_ADDR BackupAddr;
	INFO_ADDR FlashAddr;
	u16 NullNum=0;
	INFO_ID InfoID;
	INFO_ID *pInfoID;

	if(Name>=ITN_MAX) return;

	//调查有没有整理必要
	for(FlashAddr=gInfoStartAddr[Name];FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes,BackupAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL) NullNum++;
	}

	//如果空白位置不足8分之一,整理
	if(NullNum<(TotalNum/8))
	{
		Debug("Need Flush!Null %d,Total %d\n\r",NullNum,TotalNum);
	}
	else
	{
		Debug("Not Need Flush!Null %d,Total %d\n\r",NullNum,TotalNum);
		return;//无需整理
	}	
	
	//搞清楚拷贝方向
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
		BackupAddr=gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
	else
		BackupAddr=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;

	//开始拷贝
	pInfoID=Q_Malloc(gInfoAttrib[Name].BlockBytes);
	for(FlashAddr=gInfoStartAddr[Name]+((gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES)>>2);//从四分之一处开始
			FlashAddr<gInfoEndAddr[Name];
			FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)pInfoID);
		if(*pInfoID!=0 && *pInfoID!=INFO_NULL)//有效的直接拷贝
		{
			//Debug("Cp %d >> %d [%d]\n\r",GetSite(FlashAddr),GetSite(BackupAddr),gInfoAttrib[Name].BlockBytes);
			SpiFlsReadData(FlashAddr,gInfoAttrib[Name].BlockBytes,(void *)pInfoID);
			SpiFlsWriteData(BackupAddr,gInfoAttrib[Name].BlockBytes,(void *)pInfoID);
			BackupAddr+=gInfoAttrib[Name].BlockBytes;
		}
	}

	//擦除原先存储
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name] = gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
	}
	else
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;		
	}
	
	//重新统计
	gInfoOccupy[Name]=0;
    gIdleSite[Name]=1;
	InfoTempStatistics(Name);
	FindIdleSite(Name);
	
	Q_Free(pInfoID);	
}

//清除所有temp
void CleanTempInfo(INFO_TEMP_NAME Name)
{
	//u16 TotalNum=gInfoAttrib[Name].SectorNum*FLASH_SECTOR_BYTES/gInfoAttrib[Name].BlockBytes;
	//INFO_ADDR BackupAddr;
	//INFO_ADDR FlashAddr;
	//u16 NullNum=0;
	//INFO_ID InfoID;
	//INFO_ID *pInfoID;

	if(Name>=ITN_MAX) return;
	if(gInfoOccupy[Name]==0) return;

	//擦除原先存储
	if(gInfoStartAddr[Name] == gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES)
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name] = gInfoAttrib[Name].BackupSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].BackupSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;
	}
	else
	{
		SpiFlsEraseSector(gInfoStartAddr[Name]/FLASH_SECTOR_BYTES,gInfoAttrib[Name].SectorNum);
		gInfoStartAddr[Name]=gInfoAttrib[Name].StartSector*FLASH_SECTOR_BYTES;
		gInfoEndAddr[Name]=(gInfoAttrib[Name].StartSector+gInfoAttrib[Name].SectorNum)*FLASH_SECTOR_BYTES;		
	}
	
	//重新统计
	gInfoOccupy[Name]=0;
    gIdleSite[Name]=1;
}

//查找一个空闲位置
//更新空闲位置索引表
//返回INFO_RES_SPACE_FULL表示没空间
INFO_SITE FindIdleSite(INFO_TEMP_NAME Name)
{
	INFO_ADDR FlashAddr;
	INFO_ID InfoID;

	if(Name>=ITN_MAX) return INFO_PARAM_ERROR;
	
	for(FlashAddr=GetAddr(gIdleSite[Name]);FlashAddr<gInfoEndAddr[Name];FlashAddr+=gInfoAttrib[Name].BlockBytes)
	{
		SpiFlsReadData(FlashAddr,sizeof(INFO_ID),(void *)&InfoID);
		if(InfoID==INFO_NULL)
		{
			gIdleSite[Name]=GetSite(FlashAddr);
			return gIdleSite[Name];
		}
	}

	//Debug("Info Space Full!Rebuild it!F\n\r");
	gIdleSite[Name]=0x7fff;//等于7fff表示满了
	
	return INFO_SPACE_FULL;
}

//返回单位块大小
//注意是块大小，一般是256的整数分子或者整数倍
u16 GetInfoBlockSize(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoAttrib[Name].BlockBytes;
}

//返回信息大小,即实际使用大小
u16 GetInfoSize(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoAttrib[Name].ChkSizeof;
}

//返回用户设定的允许最大总数
u16 GetInfoMaxAllowNum(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return  gInfoAttrib[Name].MaxItem;
}

//返回统计的总数
u16 GetInfoTotal(INFO_TEMP_NAME Name)
{
	if(Name>=ITN_MAX) return 0;
	return gInfoOccupy[Name];
}
