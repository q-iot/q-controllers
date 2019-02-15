#ifndef __SEC_FUNC_H__
#define __SEC_FUNC_H__


void SecFuncRcdDisp(void);
void SecFuncInit(void);
bool AddSecFunc(u32 TimCnt,u32 Interval,pExpFunc pCallBack,int IntParam,void *pParam);
bool AddOnceSecFunc(u32 TimCnt,pStdFunc pCallBack,int IntParam,void *pParam);
void SecFuncExpired(void);
bool SecFuncAlready(void *pCB);
void DeleteSecFuncByCB(void *pCB);




#endif

