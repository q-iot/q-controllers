#ifndef WDEV_ONLINE_HANDLER_H
#define WDEV_ONLINE_HANDLER_H

void HostSyncFinishHook(bool Online);
bool HostIsOnline(void);
void HostActive(u32 HostWAddr);
void DevOnlinePoll(int a,void *p);


#endif

