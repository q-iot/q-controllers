#ifndef WDEV_FUNC_H
#define WDEV_FUNC_H

#include "Product.h"

#define OPEN_WDEV_ATTACK 0//是否开启wdev攻击功能



#if OPEN_WDEV_ATTACK
extern u32 gWNetMyAddr;
void AttackReplyAddClientToHost(u32 FlyAddr,u16 Group,u16 TotalCnt);
void SendSyncAttack(u16 Group);
void SendWDevConfigAttack(u32 WDevAddr);
void SendWaverConfigAttack(u16 WaverAddr);
#endif


bool MyAddrInclude(WCON_PACKET *pPkt);

#if PRODUCT_IS_JUMPER
void SendRfToHost(RF_RECORD *pRf);
void SendIrToHost(IR_RECORD *pIr);
void SendRfOppoToHost(u32 Code,u16 BasePeriod,bool IsCrf);
#endif

#if PRODUCT_IS_WAVER || PRODUCT_IS_JUMPER
void RespAddClientToHost(u16 PktCnt,u32 FlyAddr);
void SyncToHost(int a,void *p);
void ReplySyncToHost(u16 PktCnt,u32 FlyAddr);
void SendVarsToHost(int VarIdx,void *p);
void SendStrToHost(int StrID,void *pStr,bool NeedFreeStr);
void SendCmdToHost(u32 FlyAddr,void *pStr);
void SendOppoToHost(int OppoID,void *p);
void UnbindToHost(bool NeedSendMainPkt,void *a);
#endif


#endif

