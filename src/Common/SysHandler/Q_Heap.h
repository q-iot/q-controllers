#ifndef QSYS_Q_HEAP_H
#define QSYS_Q_HEAP_H

#define Q_HEAP_DEBUG 0//置1会打开记录Malloc申请者的功能，但会增加额外内存开销

void DebugHeap(void);
void QS_HeapInit(void);
void QS_MonitorFragment(void);
bool IsHeapRam(void *p);

#if Q_HEAP_DEBUG
void *_Q_Malloc(u16 Size,u8 *pFuncName,u32 Lines);
bool _Q_Free(void *Ptr,u8 *pFuncName,u32 Lines);
void *_Q_Calloc(u16 Size,u8 *pFuncName,u32 Lines);
#else
void *_Q_Malloc(u16 Size);
bool _Q_Free(void *Ptr);
void *_Q_Calloc(u16 Size);
#endif

#if Q_HEAP_DEBUG
#define Q_Malloc(n) _Q_Malloc(n,(void *)__func__,__LINE__)
#define Q_Free(p) _Q_Free(p,(void *)__func__,__LINE__)
//#define Q_Calloc(n) _Q_Calloc(n,(void *)__func__,__LINE__)
#else
#define Q_Malloc _Q_Malloc
#define Q_Free _Q_Free
//#define Q_Calloc _Q_Calloc
#endif





#endif

