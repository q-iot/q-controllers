#ifndef Q_QUEUE_H
#define Q_QUEUE_H


void *Q_NewQueue(u16 ItemSize,u16 MaxNum);
bool Q_DeleteQueue(void **ppHandler);
bool Q_QueueClean(void *pHandler);
bool Q_QueueAddItem(void *pHandler,void *New,bool Force);
bool Q_QueueAddItemToFirst(void *pHandler,void *New,bool Force);
bool Q_FetchQueueFirst(void *pHandler,void *pRead,bool NeedDelete);
bool Q_FetchQueueLast(void *pHandler,void *pRead,bool NeedDelete);
bool Q_FetchQueueItem(void *pHandler,u16 Idx,void *pRead,bool NeedDelete);
u16 Q_GetQueueItemTotal(void *pHandler);
bool Q_QueueEmpty(void *pHandler);
bool Q_QueueNotEmpty(void *pHandler);
bool Q_QueueFull(void *pHandler);
bool Q_QueueNotFull(void *pHandler);

#endif
