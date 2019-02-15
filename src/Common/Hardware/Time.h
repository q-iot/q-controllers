#ifndef __TIME_H__
#define __TIME_H__

void Tim1_Init(void);
void Tim2_Init(void);
void Tim3_Init(void);
void Tim4_Init(void);
void Tim5_Init(void);
void Tim1_Set(u16 Val,u16 uS_Base,bool AutoReload);
void Tim2_Set(u16 Val,u16 uS_Base,bool AutoReload);
void Tim3_Set(u16 Val,u16 uS_Base,bool AutoReload);
void Tim4_Set(u16 Val,u16 uS_Base,bool AutoReload);
void IO7_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse);
void IO8_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse);


#endif
