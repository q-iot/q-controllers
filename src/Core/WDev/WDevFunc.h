#ifndef _WDEV_FUNC_H_
#define _WDEV_FUNC_H_

#include "WorkMode.h"
#include "LedsMode.h"

void WDevPairDev(WNET_ADDR WAddr,pStdFunc pSentCallBack);
void WDevSyncToDev(WNET_ADDR WAddr);
void WDevUnbindDev(WNET_ADDR WAddr);
void WDevPassData(WNET_ADDR WAddr,u16 Len,void *pData,pStdFunc pCb);

#define WDevSearchDev() WDevPairDev(WNetMyAddr()|0xffff,NULL)


#endif
