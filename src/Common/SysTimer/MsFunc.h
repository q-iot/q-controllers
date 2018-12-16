#ifndef TIMING_FUNC_H
#define TIMING_FUNC_H

#include "stm32f10x.h"
#include "EventInHandler.h"

#define ALWAY_LOOP -1

void MsFuncRcdDisp(void);
void MsFuncInit(void);
bool AddMsFunc(u32 TimingMs,u32 IntervalMs,pExpFunc pCallBack,int IntParam,void *pParam);
bool AddOnceMsFunc(u32 TimingMs,pStdFunc pCallBack,int IntParam,void *pParam);
void MsFuncExpired(void);
bool MsFuncAlready(void *pCB);
void DeleteMsFuncByCB(void *pCB);


#endif

