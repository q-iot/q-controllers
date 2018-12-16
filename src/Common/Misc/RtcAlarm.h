#ifndef RTC_ALARM_H
#define RTC_ALARM_H


bool AlarmTidy(INFO_SITE *pSiteList,u16 Num);
bool AlarmClean(void);
void AlarmExpireHandler(void);
s32 AlarmCompare(RTC_TIME *pTime1,RTC_TIME *pTime2);


#endif

