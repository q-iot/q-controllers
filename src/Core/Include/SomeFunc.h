#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

void IWDG_Configuration(void);
void IWDG_PeriodCB(void);
void RebootBoard(void);
void DefaultConfig(void);

bool SysCmdHandler(u16 Len,char *pStr,char *pOutStream);



#endif

