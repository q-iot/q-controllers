#ifndef __Q_COM_FUNC_H__
#define __Q_COM_FUNC_H__

//本头文件声明了与qwifi通信的函数
//如果要使用串口与qwifi通信，需包含此头文件

typedef enum{
	QSE_NULL=0,
	QSE_READY,
	QSE_CONNECTING,
	QSE_DISCONNECT,
	QSE_RESET,
}QWIFI_STATE;

enum{
	SMF_SYS=0,//系统信息，服务器需将信息广播给所有主机
	SMF_GSM,//短信，服务器需将信息gsm给指定用户
	SMF_DEV_STR,//设备字符串，被dut推送给app，由微信消息修改
	SMF_PUSH,//推送信息，服务器需将信息由推送发给指定用户
};
typedef u8 SRV_MSG_FLAG;

const char *QCom_GetLastCmd(void);
void QCom_GetVarValue(const char *pTag);
void QCom_SetVarValue(const char *pTag,int Value,bool Signed);
void QCom_SendStr(u32 StrId,const char *pStr);
void QCom_SendMsg(u8 Flag,const char *pMsg);
void QCom_SendSta(void);
void QCom_ResetQwifi(void);

#endif
