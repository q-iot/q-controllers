#include "SysDefines.h"

static volatile u32 gNowAlarmID=0;//记录当前闹钟id

//比较两个时间大小，
//time1大，返回正值
//time2大，返回负值
//一样大，返回0
s32 AlarmCompare(RTC_TIME *pTime1,RTC_TIME *pTime2)
{
	u32 Time1;
	u32 Time2;

	//必须忽略秒数，所以不能用RtcTime2Cnt进行比较
	Time1=(pTime1->year<<20)+(pTime1->mon<<16)+(pTime1->day<<11)+(pTime1->hour<<6)+(pTime1->min);
	Time2=(pTime2->year<<20)+(pTime2->mon<<16)+(pTime2->day<<11)+(pTime2->hour<<6)+(pTime2->min);
	
	return Time1-Time2;
}

//根据星期掩码得出真实的触发时间
//输入闹钟记录，返回触发时间
bool AlarmRecord2Time(RTC_TIME *pTime,ALARM_IN_DEV *pRcd)
{
	if(pRcd->Enable == FALSE) return FALSE;
	if(pRcd->AlarmDays==0 && pRcd->OnceAlarm==0) return FALSE;
	
	if(pRcd->OnceAlarm)//直接日期
	{
		pTime->year=pRcd->Years;
		pTime->mon=pRcd->Mon;
		pTime->day=pRcd->Day;
		pTime->hour=pRcd->Hour;
		pTime->min=pRcd->Min;
		pTime->sec=0;
		return TRUE;
	}
	else if(pRcd->AlarmDays)//循环日期
	{
		RTC_TIME NowTime;
		u8 Day;
		u8 i;
		bool Flag=FALSE;
		
		RtcReadTime(&NowTime,RtcOp_GetTime);

		for(i=NowTime.week;i<(NowTime.week+7);i++)//从当天的星期数目开始
		{
			if(i >= 7) Day=i-7;
			else Day=i;
			
			if(ReadBit(pRcd->AlarmDays,Day)==0) continue;//此日不触发
			
			if(Day == NowTime.week)//判断当天
			{
				if(((NowTime.hour<<16)+(NowTime.min<<8)+NowTime.sec) < ((pRcd->Hour<<16)+(pRcd->Min<<8))) //时间还没到
				{//设置为当天
					pTime->year=NowTime.year;
					pTime->mon=NowTime.mon;
					pTime->day=NowTime.day;
					pTime->hour=pRcd->Hour;
					pTime->min=pRcd->Min;
					pTime->sec=0;
					return TRUE;					
				}
				else//时间已经过了，放到最后设置
				{
					Flag=TRUE;
				}
			}
			else//非当天
			{
				u32 TimeCnt=0;

				if(Day<NowTime.week) Day+=7;//判断下周
				
				pTime->year=NowTime.year;
				pTime->mon=NowTime.mon;
				pTime->day=NowTime.day;
				pTime->hour=pRcd->Hour;
				pTime->min=pRcd->Min;
				pTime->sec=0;

				TimeCnt=RtcTime2Cnt(pTime);//得到今天的计数
				if(TimeCnt == 0) return FALSE;//不符合要求
				TimeCnt+=(Day-NowTime.week)*86400;//一天的秒数86400

				RtcCnt2Time(TimeCnt,pTime);//转换回定时时间
				return TRUE;	
			}			
		}	

		if(Flag) //放到下周的今天，并且时间早于现在
		{
			u32 TimeCnt=0;

			//得到今天的闹钟时间(已过)
			pTime->year=NowTime.year;
			pTime->mon=NowTime.mon;
			pTime->day=NowTime.day;
			pTime->hour=pRcd->Hour;
			pTime->min=pRcd->Min;
			pTime->sec=0;

			//把今天的闹钟时间+7天
			TimeCnt=RtcTime2Cnt(pTime);//得到今天的计数
			if(TimeCnt == 0) return FALSE;//不符合要求
			TimeCnt+=7*86400;//一天的秒数86400

			RtcCnt2Time(TimeCnt,pTime);//转换回定时时间
			return TRUE;	
		}
	}	

	return FALSE;
}

//清除所有闹钟
bool AlarmClean(void)
{
	RTC_TIME Alarm;

	gNowAlarmID=0;

	//设置到若干年以后
	Alarm.year=2047;
	Alarm.mon=1;
	Alarm.day=1;
	Alarm.hour=Alarm.min=Alarm.sec=0;
	
	if(RtcAdjustTime(&Alarm,RtcOp_SetAlarm)==FALSE)
	{
		Debug("Alarm Clean Failed!\r\n");
		return FALSE;
	}
	
	return TRUE;
}

//根据数据库设置闹钟
bool AlarmTidy(INFO_SITE *pSiteList,u16 Num)
{
	IN_DEVICE_RECORD InDev;
	ALARM_IN_DEV *pAlaram=&(InDev.Record.Alarm);
	RTC_TIME Now;
	RTC_TIME Alarm;
	bool ChangeFlag=FALSE;
	s16 Res,i;

	if(pSiteList==NULL) return TRUE;
	if(Num==0) return TRUE;

	RtcReadTime(&Now,RtcOp_GetTime);//获取当前时间
	RtcReadTime(&Alarm,RtcOp_GetAlarm);//读取当前闹钟

	if(AlarmCompare(&Now,&Alarm)>=0)//闹钟时间早于当前，表示闹钟无意义
	{
		AlarmClean();		
		RtcReadTime(&Alarm,RtcOp_GetAlarm);
	}

	Debug("NowTim:%4d-%02d-%02d %02d:%02d:%02d\n\r",Now.year,Now.mon,Now.day,
			Now.hour,Now.min,Now.sec);
	Debug("NowAlm:%4d-%02d-%02d %02d:%02d:%02d\n\r\n\r",Alarm.year,Alarm.mon,Alarm.day,
			Alarm.hour,Alarm.min,Alarm.sec);
			
	for(i=0;i<Num;i++)//逐个读取闹钟
	{
		Res=ReadInfoBySite(IRN_IN_DEV,pSiteList[i],&InDev,sizeof(IN_DEVICE_RECORD));
		if(Res<0) break;
		if(InDev.DevType != IDT_ALARM) continue;

		if(pAlaram->Enable)//列出每个有效的闹钟
		{
			RTC_TIME New;//存放闹钟记录转换成标准RTC后的数据

			//转换格式
			if(AlarmRecord2Time(&New,pAlaram)==FALSE) continue;
			Debug(" Rcd:%4d-%02d-%02d %02d:%02d [%02x]\n\r",
				pAlaram->Years,pAlaram->Mon,pAlaram->Day,pAlaram->Hour,pAlaram->Min,pAlaram->OnceAlarm?0x80:pAlaram->AlarmDays);
			Debug(" Rtc:%4d-%02d-%02d %02d:%02d:%02d",New.year,New.mon,New.day,
			New.hour,New.min,New.sec);

			if(AlarmCompare(&Now,&New)>=0)//新时间早于当前，表示定时无意义
			{
				Debug(" ----\r\n\r\n");
				pAlaram->Enable=FALSE;//修改使能
				CoverInfoBySite(IRN_IN_DEV,0xffff,Res,&InDev,sizeof(IN_DEVICE_RECORD));//覆盖原信息
				continue;
			}

			//比较最小值
			if(AlarmCompare(&Alarm,&New)>0)//新时间比较靠前
			{
				Debug(" <<>>");
				ChangeFlag=TRUE;
				MemCpy(&Alarm,&New,sizeof(RTC_TIME));
			}		

			Debug("\r\n\r\n");
		}	
	}

	if(ChangeFlag)
	{
		Debug("ChaAlm:%4d-%02d-%02d %02d:%02d:%02d\n\r",Alarm.year,Alarm.mon,Alarm.day,
			Alarm.hour,Alarm.min,Alarm.sec);
			
		if(RtcAdjustTime(&Alarm,RtcOp_SetAlarm)==FALSE)
			return FALSE;
	}

	return TRUE;
}

//闹钟到期处理
void AlarmExpireHandler(void)
{
	RTC_TIME Now;

	//IOOUT_SetIoStatus(IOOUT_BEEP,TRUE);//需实现定时器频率
	
	RtcReadTime(&Now,RtcOp_GetTime);
	Now.sec=0;
	AlarmTriggerIn(RtcTime2Cnt(&Now));
}






