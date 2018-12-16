#include "SysDefines.h"
#include "Product.h"

typedef struct{//定义数据结构
	u16 Ver;//修改后会导致恢复默认
	u32 Num;//示例


	
	u32 ChkSum;
}RFS_BLOCK;//存储块，最大1k

static const RFS_BLOCK gDefBlock={//默认配置
1,//Ver
0,//Num



};

static volatile RFS_BLOCK gSysBlock;//配置缓存

void RFS_Debug(void)
{
	Debug("  -------------------------------------------------------------------\n\r");
	Frame();Debug("  |SnHash:%u\n\r",GetHwID(NULL));
#if ADMIN_DEBUG
	Frame();Debug("  |SoftVer:%u.%u(*)\n\r",__gBinSoftVer,RELEASE_DAY);
#else
	Frame();Debug("  |SoftVer:%u.%u\n\r",__gBinSoftVer,RELEASE_DAY);
#endif
	Frame();Debug("  |Num:%u\n\r",gSysBlock.Num);

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
	Rom_ReadPage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));

	if(MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4)!=gSysBlock.ChkSum)
	{
		Debug("RFS ChkSum error!\n\r");
		RFS_BurnDefaultToRom();
	}

	if(gSysBlock.Ver!=gDefBlock.Ver)
	{
		Debug("Ver Error,Reset Database!\n\r");
		RFS_BurnDefaultToRom();
	}
}

//存储默认数据到rom
void RFS_BurnDefaultToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);
	
	MemCpy((void *)&gSysBlock,&gDefBlock,sizeof(RFS_BLOCK));
	gSysBlock.ChkSum=MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gSysBlock.ChkSum)		
		Rom_WritePage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));
}

//存储当前数据到rom
void RFS_BurnToRom(void)
{
	__IO u32 *pChkSum=(void *)(IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE+sizeof(RFS_BLOCK)-4);

	gSysBlock.ChkSum=MakeHash33((void *)&gSysBlock,sizeof(RFS_BLOCK)-4);

	if(*pChkSum!=gSysBlock.ChkSum)	
		Rom_WritePage((void *)&gSysBlock,IAP_ROM1_ADDR+ROM_INFO_SAVE_PAGE*ROM_FLASH_PAGE_SIZE,sizeof(gSysBlock));
}

//读取值
u32 RFS_GetNum(u16 Item)
{
	switch(Item)
	{
		case RFSI_NUM:
			return gSysBlock.Num;









	}

    return 0;
}

//设置值
bool RFS_SetNum(u16 Item,u32 Value,void *p)
{
	switch(Item)
	{
		case RFSI_NUM:
			gSysBlock.Num=Value;
			break;








		default:
			return FALSE;
	}

	return TRUE;
}


