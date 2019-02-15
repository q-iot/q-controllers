#ifndef __Q_HEAP_H__
#define __Q_HEAP_H__

#define Q_HEAP_DEBUG 2//置1打开打印，置2会打开记录Malloc申请者的功能，但会增加额外内存开销

void QHeapDebug(void);
u32 QHeapGetIdleSize(void);
bool IsHeapRam(void *p);

#if Q_HEAP_DEBUG > 1
void *_Q_Malloc(u16 Size,const char *pFuncCaller,u32 Line);
void _Q_Free(void *Ptr,const char *pFuncCaller,u32 Line);
#else
void *_Q_Malloc(u16 Size);
void _Q_Free(void *Ptr);
#endif

#if Q_HEAP_DEBUG > 1
#define Q_Malloc(n) _Q_Malloc(n,(void *)__func__,__LINE__)
#define Q_Zalloc(n) _Q_Malloc(n,(void *)__func__,__LINE__)
#define Q_Calloc(n,s) _Q_Malloc((n)*(s),(void *)__func__,__LINE__)
#define Q_Free(p) _Q_Free(p,(void *)__func__,__LINE__)
#else
#define Q_Malloc _Q_Malloc
#define Q_Zalloc _Q_Malloc
#define Q_Calloc(n,s) _Q_Malloc((n)*(s))
#define Q_Free _Q_Free
#endif





#endif

