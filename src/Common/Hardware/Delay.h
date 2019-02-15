#ifndef __DELAY_H__
#define __DELAY_H__

void DelayMs(uint32_t nTime);
u32 GetSysTick(void);
u32 GetSysStartMs(void);
u32 GetRtcCount(void);
void SysTick_Stop(void);
void SysTick_Init(void);

#endif
