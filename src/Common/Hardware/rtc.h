#ifndef __RTC_H__
#define __RTC_H__
#include "stm32f10x.h"

typedef struct {
	u16 year;
	u8 mon;
	u8 day;
	u8 week;//取值0-6，0代表星期天
	u8 hour;
	u8 min;
	u8 sec;
}RTC_TIME;

typedef enum {
	RtcOp_CheckVal,
	RtcOp_SetTime,
	RtcOp_SetAlarm,
	RtcOp_GetTime,
	RtcOp_GetAlarm,
}RTC_OPERATE;

u32 RtcTime2Cnt(RTC_TIME *pTime);
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime);
void RtcSetUp(void);
void RtcReadTime(RTC_TIME *pTime,RTC_OPERATE Op);
bool RtcAdjustTime(RTC_TIME *pTime,RTC_OPERATE Op);

#endif
