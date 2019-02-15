#ifndef __CONTROLLER_HANDLER_H__
#define __CONTROLLER_HANDLER_H__

void ControllerDebug(void);
void ControllerRegister(const EVENT_FUNC_ITEM *pItemArray,const char *pName);
void ControllerEvtPost(EVENT_BIT_FLAG Event,int Param,void *p);

#endif
