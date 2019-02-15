#ifndef __EVENT_IN_HANDLER_H__
#define __EVENT_IN_HANDLER_H__


typedef enum{
	EBF_NULL=0,

	//调度事件，用户控制器不可用
	EBF_NEXT_QUICK_FUNC,
	
	//硬件中断事件
	EBF_KEY,//key input
	EBF_PIO_IN,//io input
	EBF_SEC_FUNC,
	EBF_30SEC,
	EBF_TIM2,
	EBF_TIM4,
	EBF_IR,

	//系统事件
	EBF_IDLE,
	EBF_INIT,
	
	//数据传入
	EBF_Q_COM_CMD,//与qwifi连接的串口有命令进来
	EBF_QWIFI_STATE,//
	EBF_QWIFI_KEY,//
	EBF_QWIFI_VAR,//
	EBF_QWIFI_MSG,//
	EBF_QWIFI_READ_VAR_RET,
	EBF_QWIFI_READ_VAR_ERROR,
	EBF_QWIFI_SET_VAR_RET,
	EBF_QWIFI_SET_VAR_ERROR,
	EBF_QWIFI_MSG_RET,//
	EBF_QWIFI_STR_RET,//


	
	//内部任务
	EBF_SYS_CMD,//系统串口命令

	//用户自定义事件
	EBF_USER_EVT1,
	EBF_USER_EVT2,
	EBF_USER_EVT3,
	EBF_USER_EVT4,









	
	//系统调度
	EBF_NEXT_LOOP_FUNC,//下个循环函数
	
	EBF_MAX
}EVENT_BIT_FLAG;

typedef enum{
	EFR_OK=0,
	EFR_STOP,//回调如果返回此结果，则事件不再分配给后续控制器

	EFR_MAX
}EVENT_HANDLER_RESUTL;

typedef EVENT_HANDLER_RESUTL (*pEvtFunc)(EVENT_BIT_FLAG,int,void *);
typedef struct{
	EVENT_BIT_FLAG Event;
	pEvtFunc EvtFunc;
}EVENT_FUNC_ITEM;


void EventDebug(void);
void SetEventFlag(EVENT_BIT_FLAG BitFlag);
void SendEvent(EVENT_BIT_FLAG BitFlag,s32 S32Param,void *pParam);
void CleanAllEvent(void);
void *WaitEvent(EVENT_BIT_FLAG *pEvent,s32 *pS32);
bool CheckEventFinished(EVENT_BIT_FLAG Event);


#endif

