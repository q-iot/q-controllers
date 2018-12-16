#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

void IWDG_Configuration(void);
void IWDG_PeriodCB(void);
void RebootBoard(void);
void DefaultConfig(void);

bool SysCmdHandler(u16 Len,u8 *pStr,u8 *pOutStream);



#endif

