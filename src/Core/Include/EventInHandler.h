#ifndef __EVENT_IN_HANDLER_H__
#define __EVENT_IN_HANDLER_H__


typedef enum{
	EBF_NULL=0,

	//调度事件，用户控制器不可用
	EBF_NEXT_QUICK_FUNC,
	
	//硬件中断事件
	EBF_KEY,//按键事件
	EBF_PIO_IN,//io事件
	EBF_SEC_FUNC,//每秒事件
	EBF_30SEC,//30秒事件
	EBF_TIM2,//定时器2到期事件
	EBF_TIM4,//定时器2到期事件
	EBF_IR,//红外接收事件

	//系统事件
	EBF_IDLE,//系统闲循环
	EBF_INIT,//系统初始化完成
	
	//数据传入
	EBF_Q_COM_CMD,//与qwifi连接的串口有字符串指令进来
	EBF_QWIFI_STATE,//qwifi的状态返回
	EBF_QWIFI_KEY,//qwifi的app上的按钮被按下
	EBF_QWIFI_VAR,//qwifi的变量改变了
	EBF_QWIFI_MSG,//qwifi收到系统消息
	EBF_QWIFI_READ_VAR_RET,//读取qwifi的变量返回结果
	EBF_QWIFI_READ_VAR_ERROR,//读取qwifi的变量返回错误
	EBF_QWIFI_SET_VAR_RET,//设置qwifi的变量返回结果
	EBF_QWIFI_SET_VAR_ERROR,//设置qwifi的变量返回结果
	EBF_QWIFI_MSG_RET,//发送系统消息给qwifi返回结果
	EBF_QWIFI_STR_RET,//发送字符串给qwifi返回结果


	
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

