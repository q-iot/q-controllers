//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了一个控制器，本控制器用来解析qwifi通过com3返回的指令结果，
并定义了一系列函数，用来给其他代码调用，以方便的向qwifi发送指令
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"

#define VAR_TAG_LEN 8

#if 1
static char CmdBuf[32]={0};

//读取发送指令缓存，即最后一条指令
const char *QCom_GetLastCmd(void)
{
	return CmdBuf;
}

//向qwifi发送获取变量值的指令
void QCom_GetVarValue(const char *pTag)
{
	if(strlen(pTag)==VAR_TAG_LEN)
	{
		CmdBuf[0]=0;
		sprintf(CmdBuf,"#var %s\r",pTag);
		Com3_Send(strlen(CmdBuf),CmdBuf);
	}
	else
	{
		Debug("Var tag len error!\n\r");
	}
}

//向qwifi发送设置变量值的指令
void QCom_SetVarValue(const char *pTag,int Value,bool Signed)
{
	if(strlen(pTag)==VAR_TAG_LEN)
	{
		CmdBuf[0]=0;
		if(Signed) sprintf(CmdBuf,"#var %s %d\r",pTag,Value);
		else sprintf(CmdBuf,"#var %s %u\r",pTag,Value);
		Com3_Send(strlen(CmdBuf),CmdBuf);
	}
	else
	{
		Debug("Var tag len error!\n\r");
	}
}

//向qwifi发送字符串指令
void QCom_SendStr(u32 StrId,const char *pStr)
{
	char *p=Q_Malloc(strlen(pStr)+16);

	sprintf(p,"#str %u %s\r",StrId,pStr);
	StrnCpy(CmdBuf,p,32);
	Com3_Send(strlen(p),p);
	
	Q_Free(p);
}

//向qwifi发送系统消息指令
void QCom_SendMsg(u8 Flag,const char *pMsg)
{
	char *p=Q_Malloc(strlen(pMsg)+16);

	sprintf(p,"#msg %u %s\r",Flag,pMsg);
	StrnCpy(CmdBuf,p,32);
	Com3_Send(strlen(p),p);
	
	Q_Free(p);
}

//向qwifi发送状态获取指令
void QCom_SendSta(void)
{
	CmdBuf[0]=0;
	sprintf(CmdBuf,"#sta\r");
	Com3_Send(strlen(CmdBuf),CmdBuf);
}

//向qwifi发送重启指令
void QCom_ResetQwifi(void)
{
	CmdBuf[0]=0;
	sprintf(CmdBuf,"#rst\r");
	Com3_Send(strlen(CmdBuf),CmdBuf);
}
#endif

//qwifi回复字符串的解析函数
static void QCom_Res_Handler(u16 Num,const char **pCmd,const char *pStr)
{
	if(strcmp((void *)pCmd[0],"#rvar")==0)//回复了变量指令
	{
		if(Num==3)//set return
		{
			if(pCmd[1][0]=='0')
			{
				SendEvent(EBF_QWIFI_SET_VAR_RET,0,(void *)pCmd[2]);
			}
			else
			{
				Debug("VAR SET RET ERROR %s\n\r",pCmd[2]);
				SendEvent(EBF_QWIFI_SET_VAR_ERROR,0,(void *)pCmd[2]);
			}
		}
		else if(Num==4)//read return
		{
			if(pCmd[1][0]=='0')
			{
				if(pCmd[3][0]=='-')
				{
					s32 VarValueS32=Str2Sint(pCmd[3]);
					SendEvent(EBF_QWIFI_READ_VAR_RET,VarValueS32,(void *)pCmd[2]);
				}
				else
				{
					u32 VarValueU32=Str2Uint(pCmd[3]);
					SendEvent(EBF_QWIFI_READ_VAR_RET,VarValueU32,(void *)pCmd[2]);
				}
			}
			else
			{
				Debug("VAR READ RET ERROR %s = x\n\r",pCmd[2]);
				SendEvent(EBF_QWIFI_READ_VAR_ERROR,0,(void *)pCmd[2]);
			}
		}
		else
		{
			Debug("VAR RET ERROR NUM = %u\n\r",Num);
		}
	}
	else if(strcmp((void *)pCmd[0],"#rstr")==0)//回复了字符串指令
	{
		if(Num==2 && pCmd[1][0]=='0') SendEvent(EBF_QWIFI_STR_RET,0,NULL);
		else if(Num==2 && pCmd[1][0]!='0') SendEvent(EBF_QWIFI_STR_RET,1,NULL);
		else Debug("STR RET ERROR NUM = %u\n\r",Num);
	}
	else if(strcmp((void *)pCmd[0],"#rmsg")==0)//回复了系统消息指令
	{
		if(Num==2 && pCmd[1][0]=='0') SendEvent(EBF_QWIFI_MSG_RET,0,NULL);
		else if(Num==2 && pCmd[1][0]!='0') SendEvent(EBF_QWIFI_MSG_RET,1,NULL);
		else Debug("MSG RET ERROR NUM = %u\n\r",Num);
	}
	else if(strcmp((void *)pCmd[0],"#rsta")==0)//回复了状态指令
	{
		if(Num==3 && pCmd[1][0]=='0')
		{
			if(strcmp((void *)pCmd[2],"rdy")==0) SendEvent(EBF_QWIFI_STATE,QSE_READY,"rdy");
			else if(strcmp((void *)pCmd[2],"con")==0) SendEvent(EBF_QWIFI_STATE,QSE_CONNECTING,"con");
			else if(strcmp((void *)pCmd[2],"dis")==0) SendEvent(EBF_QWIFI_STATE,QSE_DISCONNECT,"dis");
			else Debug("STA RET ERROR\n\r");
		}
		else 
		{
			Debug("STA RET ERROR NUM = %u\n\r",Num);
		}		
	}
	else if(strcmp((void *)pCmd[0],"#rrst")==0)//回复了重启指令
	{
		if(Num==2 && pCmd[1][0]=='0') SendEvent(EBF_QWIFI_STATE,QSE_RESET,"rst");
		else Debug("RST RET ERROR\n\r");
	}
	else
	{
		Debug("QCOM RET ERROR %s\n\r",pCmd[0]);
	}
}

//qwifi串口字符串解析函数
static EVENT_HANDLER_RESUTL QCom_Cmd_EF(EVENT_BIT_FLAG Event,int Len,const char *pStr)
{
	char *pCmd[6];
	char *pBuf=NULL;
	u16 i,Num=0;
	
	pBuf=Q_Malloc(Len+2);
	Num=StrCmdParse(pStr,pCmd,pBuf,TRUE);//解析指令和参数

	for(i=0;i<Num;i++)//打印收取到的参数
	{
		Debug("[%u]%s\n\r",i,pCmd[i]);
	}

	if(pCmd[0][0]!='#') goto Finish;

	if(strlen(pCmd[0])==5)//回包的命令都是5个字节，主包的是4个字节
	{
		QCom_Res_Handler(Num,pCmd,pStr);
		goto Finish;
	}
	
	if(strlen(pCmd[0])!=4) goto Finish;
	
	if(strcmp((void *)pCmd[0],"####")==0)
	{
		//do nothing
	}
	else if(strcmp((void *)pCmd[0],"#rdy")==0)//qwifi启动
	{
		SendEvent(EBF_QWIFI_STATE,QSE_READY,"rdy");
	}
	else if(strcmp((void *)pCmd[0],"#con")==0)//qwifi连接ok
	{
		SendEvent(EBF_QWIFI_STATE,QSE_CONNECTING,"con");
	}
	else if(strcmp((void *)pCmd[0],"#dis")==0)//qwifi断开连接
	{
		SendEvent(EBF_QWIFI_STATE,QSE_DISCONNECT,"dis");
	}
	else if(strcmp((void *)pCmd[0],"#key")==0)//qwifi上app的按钮被按下
	{
		if(Num==3) 
		{
			SendEvent(EBF_QWIFI_KEY,Str2Uint(pCmd[2]),(void *)Str2Uint(pCmd[1]));//强制转换类型
		}
		else
		{
			Debug("QCOM KEY CMD ERROR %s\n\r",pCmd[0]);
		}
	}
	else if(strcmp((void *)pCmd[0],"#msg")==0)//qwifi收到系统消息，转发给qcom
	{
		const char *pMsg=&pStr[5];
		SendEvent(EBF_QWIFI_MSG,strlen(pMsg),(void *)pMsg);
	}
	else if(strcmp((void *)pCmd[0],"#var")==0)//qwifi改变了变量，发给qcom
	{
		if(Num==3 && strlen(pCmd[1])==VAR_TAG_LEN)
		{
			if(pCmd[2][0]=='-')
			{
				s32 VarValueS32=Str2Sint(pCmd[2]);
				SendEvent(EBF_QWIFI_VAR,VarValueS32,pCmd[1]);
			}
			else
			{
				u32 VarValueU32=Str2Uint(pCmd[2]);
				SendEvent(EBF_QWIFI_VAR,VarValueU32,pCmd[1]);
			}
		}
		else
		{
			Debug("QCOM VAR CMD ERROR %s\n\r",pCmd[0]);
		}
	}
	else
	{
		Debug("QCOM CMD ERROR %s\n\r",pCmd[0]);
	}

Finish:	
	Q_Free(pBuf);
	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gQComController[]={
{EBF_Q_COM_CMD,QCom_Cmd_EF},


{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void QComControllerReg(void)
{
	ControllerRegister(gQComController,"QCom Controller");
}

