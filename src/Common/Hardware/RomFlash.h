#ifndef __ROM_FLASH_H__
#define __ROM_FLASH_H__

void DebugRomPage(u32 PageNum);
bool Rom_ReadPage(void *Buf,u32 Address,u32 Len);
bool Rom_WritePage(void *Buf,u32 Address,u32 Len);
bool Rom_ErasePage(u32 PageNum);

#endif

