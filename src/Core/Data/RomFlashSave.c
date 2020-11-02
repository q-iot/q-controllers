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

本文件定义了一个存储与芯片flash内部的数据存储机制
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "Product.h"

static const RFS_BLOCK gDefBlock={//默认配置
1,//Ver
0,//Num
0,//DebugFlags
9600,//u32 Com2Baud;//us2波特率

WRF_RSSI_THRESHOLD,//RFSI_RSSI_THRD
0,//u32 RFSI_FLY_ADDR
0,//u32 RFSI_BROTHER_ADDR;//透传兄弟地址

FALSE,//bool SnAuth;//序列认证
};

static RFS_BLOCK gDataBlock;//配置缓存

void RFS_Debug(char *pOutStream)
{
	Debug("  -------------------------------------------------------------------\n\r");

	if(pOutStream!=NULL)
	{
		pOutStream[0]=0;
		sprintf((void *)&pOutStream[strlen(pOutStream)]," HwId:%u \n\r",GetHwID(NULL));
		sprintf((void *)&pOutStream[strlen(pOutStream)]," SoftVer:%u.%u \n\r",__gBinSoftVer,RELEASE_DAY);
		sprintf((void *)&pOutStream[strlen(pOutStream)]," Com2_Baud:%u \n\r",gDataBlock.Com2Baud);
		sprintf((void *)&pOutStream[strlen(pOutStream)]," My WAddr:0x%x \n\r",WNetMyAddr());
		if(RFS_DB()->SnAuth) sprintf((void *)&pOutStream[strlen(pOutStream)]," Fly WAddr:0x%x \n\r",gDataBlock.RFSI_FLY_ADDR);
		sprintf((void *)&pOutStream[strlen(pOutStream)]," Broterh:0x%x \n\r",gDataBlock.RFSI_BROTHER_ADDR);
	}

	//串口打印
	Frame();Debug("  |HwId:%u\n\r",GetHwID(NULL));
	Frame();Debug("  |SoftVer:%u.%u\n\r",__gBinSoftVer,RELEASE_DAY);
	Frame();Debug("  |Com2 Baudrate:%u\n\r",gDataBlock.Com2Baud);
	Frame();Debug("  |MY_WNET_ADDR:0x%x\n\r",WNetMyAddr());
	Frame();Debug("  |RFSI_RSSI_THRD:%u\n\r",gDataBlock.RFSI_RSSI_THRD);
	Frame();Debug("  |RFSI_FLY_ADDR:0x%x\n\r",gDataBlock.RFSI_FLY_ADDR);
	Frame();Debug("  |RFSI_BROTHER_ADDR:0x%x\n\r",gDataBlock.RFSI_BROTHER_ADDR);
	Frame();Debug("  |Debug:",gDataBlock.RFSI_BROTHER_ADDR);
		if(NeedDebug(DFT_WNET))Debug("wnet ");
		if(NeedDebug(DFT_WPKT))Debug("wpkt ");
		if(NeedDebug(DFT_WDEV))Debug("wdev ");
		Debug("\n\r");
	Debug("  -------------------------------------------------------------------\n\r");
}

//数据存储初始化
void RFS_Init(void)
{
	if(sizeof(RFS_BLOCK)>ROM_FLASH_PAGE_SIZE)
	{
		Debug("InfoSaveBlock %u too big!\n\r",sizeof(RFS_BLOCK));
		while(1);
	}

	//读出配置
	Rom_ReadPage((void *)&gDataBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gDataBlock));

	if(MakeHash33((void *)&gDataBlock,sizeof(RFS_BLOCK)-4)!=gDataBlock.ChkSum)
	{
		Debug("RFS ChkSum error!\n\r");
		RFS_BurnDefaultToRom();
	}

	if(gDataBlock.Ver!=gDefBlock.Ver)
	{
		Debug("Ver Error,Reset Database!\n\r");
		RFS_BurnDefaultToRom();
	}
}

//存储默认数据到rom
void RFS_BurnDefaultToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);
	
	MemCpy((void *)&gDataBlock,&gDefBlock,sizeof(RFS_BLOCK));
	gDataBlock.ChkSum=MakeHash33((void *)&gDataBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gDataBlock.ChkSum)		
		Rom_WritePage((void *)&gDataBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gDataBlock));
}

//存储当前数据到rom
void RFS_BurnToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);

	gDataBlock.ChkSum=MakeHash33((void *)&gDataBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gDataBlock.ChkSum)	
		Rom_WritePage((void *)&gDataBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gDataBlock));
}

const RFS_BLOCK *RFS_DefDB(void)
{
	return &gDefBlock;
}

RFS_BLOCK *RFS_DB(void)
{
	return &gDataBlock;
}

bool RFS_GetDebugBits(DEBUG_FLAG_TYPE Flag)
{
	return ReadBit(gDataBlock.DebugFlags,Flag)?TRUE:FALSE;
}

void RFS_SetDebugBits(DEBUG_FLAG_TYPE Flag)
{
	if(Flag==DFT_MAX) gDataBlock.DebugFlags=0xff;
	else SetBit(gDataBlock.DebugFlags,Flag);
}

void RFS_ClrDebugBits(DEBUG_FLAG_TYPE Flag)
{
	if(Flag==DFT_MAX) gDataBlock.DebugFlags=0;
	else ClrBit(gDataBlock.DebugFlags,Flag);
}

