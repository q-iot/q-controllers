#include "SysDefines.h"
#include "SpiFlashApi.h"

#define DB_Debug Debug

//外部申明
extern const SYS_DB_STRUCT gDefSysDb;
extern SYS_DB_STRUCT gSysDb;
void SysSvnInit(void);
u32 Sys_GetValue(u16 Item,u32 IntParam,void *Val);
bool Sys_SetValue(u16 Item,u32 IntParam,void *pParam,u8 ByteLen);
void Sys_Default(void);

typedef u32 (*GetValueCallBackFunc)(u16 Item,u32 IntParam,void *Val);
typedef bool (*SetValueCallBackFunc)(u16 Item,u32 IntParam,void *pParam,u8 ByteLen);
typedef void (*BurnDefaultCallBackFunc)(void);

#define DBF_REMOVED 0x55550000//废弃
#define DBF_USED 0x55555555//使用中
#define DBF_IDLE 0x5555ffff//空闲

typedef struct{	
	u32 Flag;//读取标志，无需用户干预
	u32 Ver;//版本，无需用户干预
	u32 ChkSum;//校验和，无需用户干预
	
	u8 Data[4];//用户数据区域
}DB_STRUCT;	//数据库

typedef struct{
	bool ForceDef;//一旦改变此值，则启动后恢复默认值
	u8 StartSector;//起始扇区号
	u8 SectorNum;//占用扇区个数
	u8 UnitPageNum;//每单元占用页面个数
	const DB_STRUCT *pDefaultData;//默认值存储空间
	DB_STRUCT *pData;//当前值存储空间
	u32 DataBytes;//存储结构体大小，含ver check等前体部分
	GetValueCallBackFunc GetValue;
	SetValueCallBackFunc SetValue;
	BurnDefaultCallBackFunc BurnDefault;
}SUB_DB_STRUCT;

const SUB_DB_STRUCT gSubDbs[SDN_MAX]={
{FALSE,0,1,1,(void *)&gDefSysDb,(void *)&gSysDb,sizeof(SYS_DB_STRUCT),Sys_GetValue,Sys_SetValue,Sys_Default},
};

#define GetStartAddr(n) (gSubDbs[n].StartSector*FLASH_SECTOR_BYTES)

//DB 版本
#if PRODUCT_IS_JUMPER
#define QDB_VER 2
#endif
#define GetDbVer(n) (QDB_VER+gSubDbs[n].ForceDef+gSubDbs[n].StartSector+gSubDbs[n].SectorNum+gSubDbs[n].UnitPageNum  \
									+gSubDbs[n].DataBytes+MakeHash33((void *)gSubDbs[n].pDefaultData,gSubDbs[n].DataBytes))

static u32 gNowDbAddr[SDN_MAX]={0};//当前db有效位置

#if 0 //for debug
static void DB_BufDisp(void)
{
	u8 buf[FLASH_PAGE_SIZE];
	int i,j;

	DB_Debug("Spi Flash:\n\r");
	for(j=0;j<16;j++)
	{
		Q_SpiFlashSync(FlashRead,GetStartAddr(Name)+j*256,sizeof(buf),buf);
		DB_Debug("page %d context:",j);
		for(i=0;i<FLASH_PAGE_SIZE;i++)
		{
			if(buf[i]!=0xff)
			{
				DB_Debug("0x%02x ",buf[i]);
			}
		}
		DB_Debug("\n\r");
	}
}
#endif

//写默认值到flash和系统
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name)
{
	DB_Debug("Now burn default database to spi flash!\n\r");
	SpiFlsEraseSector(gSubDbs[Name].StartSector,gSubDbs[Name].SectorNum);//擦除整个db存放扇区

	//将默认值写到系统和flash
	MemCpy((void *)gSubDbs[Name].pData,(void *)gSubDbs[Name].pDefaultData,gSubDbs[Name].DataBytes);//将默认数据拷贝到系统
	if(gSubDbs[Name].BurnDefault != NULL) gSubDbs[Name].BurnDefault();
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(GetStartAddr(Name),gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//将默认数据拷贝到flash db扇区

	gNowDbAddr[Name]=GetStartAddr(Name);
}

//将缓存烧写到flash
void QDB_BurnToSpiFlash(SUB_DB_NAME Name)
{
	u32 NewChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);//记录旧校验和
	if(gSubDbs[Name].pData->ChkSum==NewChkSum) return;//校验和一样，不予处理		
		
	if(gNowDbAddr[Name] == GetStartAddr(Name)+(gSubDbs[Name].SectorNum*FLASH_SECTOR_PAGE_NUM-gSubDbs[Name].UnitPageNum)*FLASH_PAGE_SIZE)//最后一页，擦除整个扇区
	{
		SpiFlsEraseSector(gSubDbs[Name].StartSector,gSubDbs[Name].SectorNum);//擦除整个db存放扇区
		gSubDbs[Name].pData->Flag=DBF_USED;
		gSubDbs[Name].pData->Ver=GetDbVer(Name);
		gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
		gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
		DB_Debug("Database Chk:%x,P:%x,Len:%u\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		SpiFlsWriteData(GetStartAddr(Name),gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//将数据拷贝到flash db扇区
		gNowDbAddr[Name]=GetStartAddr(Name);
		Debug("Burn Addr:%d\n\r",gNowDbAddr[Name]);
	}
	else //非最后一页，直接写入
	{
		u32 DbFlag=DBF_REMOVED;
		SpiFlsWriteData(gNowDbAddr[Name],sizeof(u32),(void *)&DbFlag);//擦除标志
		Debug("Set Remove Flag At Page:%d\n\r",gNowDbAddr[Name]/FLASH_PAGE_SIZE);
		
		gNowDbAddr[Name]+=(gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
		gSubDbs[Name].pData->Flag=DBF_USED;
		gSubDbs[Name].pData->Ver=GetDbVer(Name);
		gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
		gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
		SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//将数据拷贝到flash db扇区
		Debug("Burn At Page:%d\n\r",gNowDbAddr[Name]/FLASH_PAGE_SIZE);
	}
}

//从存储体读出数据库到内存，通过指针返回当前存储页
static void QDB_ReadFromSpiFlash(SUB_DB_NAME Name)
{
	u32 i;
	u32 DbFlag;
	u32 ChkSum;

	for(i=0;i<(gSubDbs[Name].SectorNum*FLASH_SECTOR_PAGE_NUM/gSubDbs[Name].UnitPageNum);i++) //遍历存储区
	{
		SpiFlsReadData(GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,sizeof(u32),(void *)&DbFlag);//读标志位
		
		if(DbFlag == DBF_USED)//读到正确数据
		{
			Debug("Read DB Idx:%d,Addr:%d\n\r",i,GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
			SpiFlsReadData(GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);
			gNowDbAddr[Name]=GetStartAddr(Name)+i*gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE;
			if(gSubDbs[Name].pData->Ver!=GetDbVer(Name))//检查版本是否正确
			{
				DB_Debug("Database version 0x%x is not right(!=0x%x)!\n\rBurn default database to flash\n\r",gSubDbs[Name].pData->Ver,GetDbVer(Name));
				for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++) QDB_BurnDefaultToSpiFlash(Name);
				return;
			}

			ChkSum=gSubDbs[Name].pData->ChkSum;
			gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
			if(ChkSum!= MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes))//检查校验和是否正确
			{
				DB_Debug("Database chksum is not right!(%x != %x) %x-%x\n\rsys halt!\n\r",ChkSum, MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes),gSubDbs[Name].pData->Data,gSubDbs[Name].DataBytes);
				QDB_BurnDefaultToSpiFlash(Name);
			}

			return;
		}
	}

	DB_Debug("Not read database for DB[%d]!\n\rBurn default database!\n\r",Name);
	QDB_BurnDefaultToSpiFlash(Name);
}

//初始化函数 作用如下
// 1.从flash读取数据库内容
// 2.如果没有读到数据库，则将默认值数据库烧写到flash
// 3.如果有数据库，则读取数据库到缓存
void QDB_Init(void)
{	
	SUB_DB_NAME Name;
	
	Debug("Database init\n\r");

	for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++)
	{
		if((gSubDbs[Name].DataBytes) > gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE)
		{
			Debug("!!!DB[%d] init error!\n\r***Flash size[%d] for database[%d] is too small!\n\r",Name,gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE,gSubDbs[Name].DataBytes);
			while(1);
		}
		
		QDB_ReadFromSpiFlash(Name);
		
		Debug("DB[%d] Burn Sector:%d\n\r",Name,gSubDbs[Name].StartSector);
		Debug("DB[%d] Version:%x-%x\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
		Debug("DB[%d] ChkSum:%x\n\r",Name,gSubDbs[Name].pData->ChkSum);
		Debug("DB[%d] Size:%d Byte < UnitPageNum x FLASH_PAGE_SIZE (=%d Byte)\n\r\n\r",Name,gSubDbs[Name].DataBytes,gSubDbs[Name].UnitPageNum*FLASH_PAGE_SIZE);
	}

	SysSvnInit();	
}

//从数据库缓存读取值，只能使用pDB_Setting获取
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val)
{
	if(gSubDbs[Name].GetValue != NULL)	
		return gSubDbs[Name].GetValue(Item,IntParam,Val);

	return 0;
}

//写值到数据库缓存，只能使用pDB_Setting存储
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u8 ByteLen)
{
	if(gSubDbs[Name].SetValue != NULL)
		return gSubDbs[Name].SetValue(Item,IntParam,pParam,ByteLen);	

	return FALSE;
}


