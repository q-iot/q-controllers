#ifndef SYS_DEFINES_H
#define SYS_DEFINES_H

#define SYS_TICK_PERIOD_MS  1  // 定义节拍毫秒数
#define SYS_SCHEDULE_PERIOS_MS  4//定义调度时间，必须为2的n次方
#define SYS_SCHEDULE_PERIOD_MASK 0x03//定义调度掩码
#define SYS_SCHEDULE_PERIOD_OFFSET 2//相当于乘除的移位
#define WDG_CB_MS 500//喂狗周期
#define TRIGGER_IN_POLL_MS 200//轮询信号输入的时间

#define Ms2Sch(Ms) ((Ms)>>SYS_SCHEDULE_PERIOD_OFFSET)
#define Sch2Ms(Period) ((Period)<<SYS_SCHEDULE_PERIOD_OFFSET)

#define MAX_VAR(a,b)  (a)>(b)?(a):(b)
#define MIN_VAR(a,b)  (a)<(b)?(a):(b)

#define Frame() Debug("                                                                    |\r");

#include "LimitMarco.h"
#include "Drivers.h"
#include "Q_Heap.h"
#include "Q_Queue.h"
#include "EventInHandler.h"
#include "NextLoopFunc.h"
#include "MsFunc.h"
#include "SecFunc.h"
#include "SysTimer.h"
#include "SpiFlashApi.h"
#include "RomFlashSave.h"
#include "SomeFunc.h"

extern const u32 __gBinSoftVer;
extern const u32 __gBinSize;
extern const u32 __gBinSum;

 
#endif

