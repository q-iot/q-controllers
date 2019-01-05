#include "SysDefines.h"
#include "Product.h"

static const RFS_BLOCK gDefBlock={//默认配置
1,//Ver
0,//Num



};

static RFS_BLOCK gDataBlock;//配置缓存

void RFS_Debug(void)
{
	Debug("  -------------------------------------------------------------------\n\r");
	Frame();Debug("  |SnHash:%u\n\r",GetHwID(NULL));
#if ADMIN_DEBUG
	Frame();Debug("  |SoftVer:%u.%u(*)\n\r",__gBinSoftVer,RELEASE_DAY);
#else
	Frame();Debug("  |SoftVer:%u.%u\n\r",__gBinSoftVer,RELEASE_DAY);
#endif
	Frame();Debug("  |Num:%u\n\r",gDataBlock.Num);

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


