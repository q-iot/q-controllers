#ifndef ROM_FLASH_SAVE_H
#define ROM_FLASH_SAVE_H

#include "Product.h"

typedef enum{
	RFSI_NULL=0,
	RFSI_NUM,

	
}RFS_ITEM;


void RFS_Debug(void);
void RFS_Init(void);
void RFS_BurnDefaultToRom(void);
void RFS_BurnToRom(void);
u32 RFS_GetNum(u16 Item);
bool RFS_SetNum(u16 Item,u32 Value,void *p);




#endif

