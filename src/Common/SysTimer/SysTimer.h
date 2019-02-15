#ifndef __SYS_TIMER_H__
#define __SYS_TIMER_H__

#include "FuncType.h"
#include "Product.h"

typedef enum{
	STT_NULL=0,
	STT_AUTO,//全自动定时器，加入后即开始定时响应，一直到系统停止
	STT_MANUAL,//手动定时器，加入后，不自动开始，需调用StartSysTimer才开始,到期后停止，需再次调用StartSysTimer开始
}SYS_TIMER_TYPE;

typedef struct{
	u32 Id;//返回的定时器识别号，为0表示未使用
	bool ExcInISR;
	SYS_TIMER_TYPE Type;
	EVENT_BIT_FLAG Event;
	u32 Count;//计数
	u32 ValueTick;//最终定时值
	u32 DefVal;//增加定时器时保存的定时值
	pVoidFunc pCB;//到期后的callback，auto和manual类型的在中断中执行
	pStdFunc pTaskTimoutCB;//仅供task
	u8 TaskId;//仅供task
}SYS_TIMER_RCD;

void DebugSysTimer(void);
void SysTimerInit(void);
u32 AddSysTimer(SYS_TIMER_TYPE Type,u32 Value,EVENT_BIT_FLAG Event,pVoidFunc pCallBackFunc,bool ExcInISR);
u32 AddSysTimerForTask(u32 Value,pStdFunc pCallBackFunc,u8 TaskId);
bool ChangeSysTimerVal(u32 Id,u32 Value);
bool StartSysTimer(u32 Id,u32 Value);
bool StopSysTimer(u32 Id);
bool SysTimerWorking(u32 Id);
u32 GetSysTimerRemain(u32 Id);
u32 GetSysTimerCount(u32 Id);
bool DeleteSysTimer(u32 Id);

#define SYS_TIMER_MAX_NUM 20//允许定义的系统定时器数目


extern u32 gWdgTimer;
extern u32 gTimingFuncTimer;	



#endif

