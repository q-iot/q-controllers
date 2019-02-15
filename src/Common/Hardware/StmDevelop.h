#ifndef __STM_DEVELOP_H__
#define __STM_DEVELOP_H__

void IWDG_Configuration(void);
void IWDG_PeriodCB(void);
void RebootBoard(void);
u32 GetHwID(u8 *pID);
u32 SaveCpuStatus(void);
void RestoreCpuStatus(u32);

#if 1
#define IntSaveInit() u32 cpu_sr=0;
#define EnterCritical() cpu_sr=SaveCpuStatus()
#define LeaveCritical() RestoreCpuStatus(cpu_sr)
#else
#define IntSaveInit() 
#define EnterCritical() __set_PRIMASK(1);
#define LeaveCritical() __set_PRIMASK(0);
#endif

#endif
