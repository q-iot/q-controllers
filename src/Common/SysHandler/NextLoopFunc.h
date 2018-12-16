#ifndef NEXT_LOOP_FUNC
#define  NEXT_LOOP_FUNC

#include "FuncType.h"

void NextLoopFuncInit(void);
bool AddNextVoidFunc(bool IsQuick,pVoidFunc pCallBack);
bool AddNextStdFunc(bool IsQuick,pStdFunc pCallBack,int IntParam,void *pParam);
bool AddNextExpFunc(bool IsQuick,pExpFunc pCallBack,int IntParam,int IntParam2,void *pParam);
bool NextFuncExcute(bool IsQuick);

#endif
