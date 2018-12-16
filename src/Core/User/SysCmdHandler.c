#include "SysDefines.h"
#include "Product.h"


static bool __inline SysCmdHandler_A(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"adc")==0)
	{
		u32 Idx=StrToUint(pParam[0]);

		Debug("Adc[%u]=%u\n\r",Idx,Adc1_GetVal(Idx));
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd,"adc_temp")==0)
	{
		Debug("Cpu temp=%u\n\r",GetCpuTemp());
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd,"adc_rand")==0)
	{
		Debug("Adc=%x\n\r",GetAdcRand());
		
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_B(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{


	return FALSE;
}

static bool __inline SysCmdHandler_C(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{

	
	return FALSE;
}

static bool __inline SysCmdHandler_D(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"debug")==0)
	{
		if(IsNullStr(pParam[0]))//help
		{
			//RFS_Debug();
		}
		else if(NotNullStr(pParam[0]))
		{
			//INFO_RECORD_NAME Name;
			//u16 Idx=1,Num=0xffff;
			
			//if(NotNullStr(pParam[1])) Idx=StrToUint(pParam[1]);
			//if(NotNullStr(pParam[2])) Num=StrToUint(pParam[2]);

			if(strcmp((void *)pParam[0],"sys")==0) {}//RFS_Debug();}
			else if(strcmp((void *)pParam[0],"tim")==0) {DebugSysTimer();}
			//else if(strcmp((void *)pParam[0],"task")==0) {DebugTask();Debug("\r\n");DebugSysTimer();}
			else if(strcmp((void *)pParam[0],"ms")==0) {MsFuncRcdDisp();}	
			else if(strcmp((void *)pParam[0],"sec")==0) {SecFuncRcdDisp();}	
			else if(strcmp((void *)pParam[0],"event")==0) {EventDebug();}
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd,"def")==0)
	{
		DefaultConfig();
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_E(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"erase")==0)
	{
		if(IsNullStr(pParam[0]))//help
		{
			Debug("----------------------------------------------\n\r");
			Debug("Erase bulk \n\r");
			Debug("Erase sector Start_Sector Sector_Num\n\r");
			Debug("----------------------------------------------\n\r");
		}
		else if(NotNullStr(pParam[0]))
		{
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd,"eee")==0)
	{
		u32 a=StrToUint(pParam[0]);
		u32 b=StrToUint(pParam[1]);

		Debug("ieee2f %f = %x\n\r",110.80,Float2Ieee(110.80));
		Debug("ieee2f %f = %x\n\r",0.124,Float2Ieee(0.124));
		Debug("float2i %x = %f\n\r",0x3DFDF3B6,Ieee2Float(0x3DFDF3B6));
		Debug("float2i %x = %f\n\r",0x42DDCC80,Ieee2Float(0x42DDCC80));
		
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_F(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	
	
	return FALSE;
}

static bool __inline SysCmdHandler_G(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_H(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"heap")==0)
	{
		DebugHeap();
		QS_MonitorFragment();
		
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_I(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"ioset")==0)
	{
		IOIN_SetIoStatus((IO_IN_DEFS)(StrToUint(pParam[0])+IOIN_PIO0),StrToUint(pParam[1])?TRUE:FALSE);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_K(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_M(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_N(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_P(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"pwm1")==0)
	{
		u32 Val=StrToUint(pParam[0]);
		u32 uS_Base=StrToUint(pParam[1]);
		u32 Pluse=StrToUint(pParam[2]);

		IO7_PWM_CONFIG(Val,uS_Base,Pluse);

		return TRUE;
	}
	else if(strcmp((void *)pCmd,"pwm2")==0)
	{
		u32 Val=StrToUint(pParam[0]);
		u32 uS_Base=StrToUint(pParam[1]);
		u32 Pluse=StrToUint(pParam[2]);

		IO8_PWM_CONFIG(Val,uS_Base,Pluse);

		return TRUE;
	}
	else if(strcmp((void *)pCmd,"pin")==0)
	{
		u32 IO=StrToUint(pParam[0]);

		if(IO>0 && IO<=8)
			Debug("PIN[%u]=%u\n\r",IO,IOIN_ReadIoStatus((IO_IN_DEFS)(IO-1+IOIN_PIO0)));

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_Q(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{

	return FALSE;
}


static bool __inline SysCmdHandler_R(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"reset")==0)
	{
		RebootBoard();//等死
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_S(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"save")==0)
	{
		Debug("Save System Parameter\n\r");
		//RFS_BurnToRom();
		return TRUE;
	}
	else if(strcmp((void *)pCmd,"setio")==0)
	{
		IOIN_SetIoStatus((IO_IN_DEFS)(IOIN_PIO0+StrToUint(pParam[0])),StrToUint(pParam[1])?TRUE:FALSE);
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_T(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"test")==0)
	{
		if(IsNullStr(pParam[0]))//help
		{
			Debug("----------------------------------------------\n\r");
			Debug("Test xxx \n\r");
			Debug("----------------------------------------------\n\r");
		}
		else
		{ 
			//StartFeedColor(1);
		}

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_U(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{

	return FALSE;
}

static bool __inline SysCmdHandler_V(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{
	if(strcmp((void *)pCmd,"version")==0)
	{
#if ADMIN_DEBUG
			Debug("Firmware:%u.%u(*)\r\n",__gBinSoftVer,RELEASE_DAY);
#else
			Debug("Firmware:%u.%u\r\n",__gBinSoftVer,RELEASE_DAY);
#endif
		
		return TRUE;
	}

    return FALSE;
}

static bool __inline SysCmdHandler_W(u8 *pCmd,u8 **pParam,u8 *StrCopyBuf,u8 *pOutStream)
{

	return FALSE;
}

//通用的串口输入处理
//Len 字符串个数
//pStr 字符串
//将处理如下命令:
#define UART_CMD_MAX_PARAM_NUM 6//最长参数
#define COM_CMD_STR_LEN 128
bool SysCmdHandler(u16 Len,u8 *pStr,u8 *pOutStream)
{
	u16 i,n;
	u8 *pCmd=NULL;
	u8 *pParam[UART_CMD_MAX_PARAM_NUM]={NULL,NULL,NULL,NULL,NULL,NULL};
	u8 *pStrCopyBuf=NULL;
	bool Res=FALSE;
	
	if(Len==0)//控制字符
	{
		if(((u16 *)pStr)[0]==0x445b)
		{

		}
		else if(((u16 *)pStr)[0]==0x435b)
		{

		}
		else if((((u16 *)pStr)[0]==0x5f00)||(((u16 *)pStr)[0]==0x0))
		{
			Debug("\n\r");
		}
		else
		{
			Debug("CtrlCode:%x\n\r",((u16 *)pStr)[0]);
		}	
		return Res;
	}

	for(n=0;n<UART_CMD_MAX_PARAM_NUM;n++)	pParam[n]=NULL;//清空参数

	pStrCopyBuf=Q_Malloc(COM_CMD_STR_LEN);
	MemCpy(pStrCopyBuf,pStr,Len+1);//for uip shell
		
	pCmd=pStr;
	for(i=0,n=0;pStr[i];i++)//取参数
	{
		if(pStr[i]==' ')
		{
			pStr[i]=0;
			if(pStr[i+1]&&pStr[i+1]!=' ')
			{
				if(n>=UART_CMD_MAX_PARAM_NUM) break;
				pParam[n++]=&pStr[i+1];
			}
		}
	}

	Len=strlen((void *)pCmd);
	for(i=0;i<=Len;i++)//命令字符串全部转小写
	{
		if(pCmd[i]>='A' && pCmd[i]<='Z')
			pCmd[i]=pCmd[i]+32;
	}

	Debug("\n\r");
	switch(pCmd[0])
	{
		case 'a': Res=SysCmdHandler_A(pCmd,pParam,pStrCopyBuf,pOutStream);break;			
		case 'b': Res=SysCmdHandler_B(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'c': Res=SysCmdHandler_C(pCmd,pParam,pStrCopyBuf,pOutStream);break;		
		case 'd': Res=SysCmdHandler_D(pCmd,pParam,pStrCopyBuf,pOutStream);break;			
		case 'e': Res=SysCmdHandler_E(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'f': Res=SysCmdHandler_F(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'g': Res=SysCmdHandler_G(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'h': Res=SysCmdHandler_H(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'i': Res=SysCmdHandler_I(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'k': Res=SysCmdHandler_K(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'm': Res=SysCmdHandler_M(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'n': Res=SysCmdHandler_N(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'p': Res=SysCmdHandler_P(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'q': Res=SysCmdHandler_Q(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'r': Res=SysCmdHandler_R(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 's': Res=SysCmdHandler_S(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 't': Res=SysCmdHandler_T(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'u': Res=SysCmdHandler_U(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'v': Res=SysCmdHandler_V(pCmd,pParam,pStrCopyBuf,pOutStream);break;
		case 'w': Res=SysCmdHandler_W(pCmd,pParam,pStrCopyBuf,pOutStream);break;
			
		default:
			goto NoSuchCmd;
	}
	Q_Free(pStrCopyBuf);
	
	if(Res==FALSE) 
	{
NoSuchCmd:
		Debug("No Such Cmd\n\r");
	}

	return Res;
}

