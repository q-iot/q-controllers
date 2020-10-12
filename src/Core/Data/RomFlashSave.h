#ifndef __ROM_FLASH_SAVE_H__
#define __ROM_FLASH_SAVE_H__

#include "Product.h"

typedef struct{//定义数据结构
	u16 Ver;//修改后会导致恢复默认

	//用户自己定义的数据存储，注意总长度不要大于ROM_FLASH_PAGE_SIZE
	u32 Num;//示例
	u32 DebugFlags;//bit flag
	u32 Com2Baud;//us2波特率
	
	u16 RFSI_RSSI_THRD;//rssi阈值
	u32 RFSI_FLY_ADDR;//主机地址
	u32 RFSI_BROTHER_ADDR;//透传兄弟地址

	bool SnAuth;//序列认证

	u32 ChkSum;//系统内部使用，不可修改
}RFS_BLOCK;//存储块，最大1k

typedef enum{
	DFT_WNET=0,
	DFT_WPKT,
	DFT_WDEV,

	DFT_MAX	
}DEBUG_FLAG_TYPE;

void RFS_Debug(void);
void RFS_Init(void);
void RFS_BurnDefaultToRom(void);
void RFS_BurnToRom(void);
const RFS_BLOCK *RFS_DefDB(void);
RFS_BLOCK *RFS_DB(void);

bool RFS_GetDebugBits(DEBUG_FLAG_TYPE Flag);
void RFS_SetDebugBits(DEBUG_FLAG_TYPE Flag);
#define NeedDebug(x) RFS_GetDebugBits(x)

#endif

