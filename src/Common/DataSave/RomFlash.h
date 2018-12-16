#ifndef ROM_FLASH_H
#define ROM_FLASH_H

void DebugRomPage(u32 PageNum);
bool Rom_ReadPage(void *Buf,u32 Address,u32 Len);
bool Rom_WritePage(void *Buf,u32 Address,u32 Len);
bool Rom_ErasePage(u32 PageNum);

#endif

