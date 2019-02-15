#ifndef __ROM_FLASH_SAVE_H__
#define __ROM_FLASH_SAVE_H__

#include "Product.h"

typedef struct{//定义数据结构
	u16 Ver;//修改后会导致恢复默认

	//用户自己定义的数据存储，注意总长度不要大于ROM_FLASH_PAGE_SIZE
	u32 Num;//示例

	
	u32 ChkSum;//系统内部使用，不可修改
}RFS_BLOCK;//存储块，最大1k


void RFS_Debug(void);
void RFS_Init(void);
void RFS_BurnDefaultToRom(void);
void RFS_BurnToRom(void);
const RFS_BLOCK *RFS_DefDB(void);
RFS_BLOCK *RFS_DB(void);


#endif

