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
#include "FuncType.h"
#include "Drivers.h"
#include "SpiFlashApi.h"
#include "RomFlashSave.h"
#include "SomeFunc.h"
#include "Q_Heap.h"
#include "Q_Queue.h"
#include "MsFunc.h"
#include "SecFunc.h"
#include "SysTimer.h"
#include "EventInHandler.h"
#include "NextLoopFunc.h"
#include "ControllerHandler.h"
#include "QComFunc.h"
#include "NameDebug.h"

#include "WNetNameDebug.h"
#include "WNetPktTypes.h"
#include "WNetTransHandler.h"
#include "WNetPktSend.h"
#include "WNetPktAttrib.h"
#include "WNetRecvQ.h"

extern const u32 __gBinSoftVer;
extern const u32 __gBinSize;
extern const u32 __gBinSum;


//功能开关，注意功能函数要引用此头文件
#define ENABLE_IR_FUNC 1//使用ir功能

 
#endif

