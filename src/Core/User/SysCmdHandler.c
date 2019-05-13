//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。

Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。

所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于
酷物联视频（q-iot.cn）进行传播或有偿售卖，相应所得收入扣除税费及维护费用后，
均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件用来定义处理串口指令的解析函数，是一套非常完整的串口指令解析机制
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "StrParse.h"
#include "Product.h"

static bool __inline SysCmdHandler_A(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"adc")==0)
	{
		u32 Idx=Str2Uint(pCmd[1]);

		Debug("Adc[%u]=%u\n\r",Idx,Adc1_GetVal(Idx));
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"adc_temp")==0)
	{
		Debug("Cpu temp=%u\n\r",GetCpuTemp());
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"adc_rand")==0)
	{
		Debug("Adc=%x\n\r",GetAdcRand());
		
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_B(char **pCmd,const char *pStrCopy,char *pOutStream)
{


	return FALSE;
}

static bool __inline SysCmdHandler_C(char **pCmd,const char *pStrCopy,char *pOutStream)
{

	
	return FALSE;
}

static bool __inline SysCmdHandler_D(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"debug")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			RFS_Debug();
		}
		else if(NotNullStr(pCmd[1]))
		{
			//INFO_RECORD_NAME Name;
			//u16 Idx=1,Num=0xffff;
			
			//if(NotNullStr(pCmd[2])) Idx=Str2Uint(pCmd[2]);
			//if(NotNullStr(pCmd[3])) Num=Str2Uint(pCmd[3]);

			if(strcmp((void *)pCmd[1],"sys")==0) {RFS_Debug();}
			else if(strcmp((void *)pCmd[1],"tim")==0) {DebugSysTimer();}
			//else if(strcmp((void *)pCmd[1],"task")==0) {DebugTask();Debug("\r\n");DebugSysTimer();}
			else if(strcmp((void *)pCmd[1],"ms")==0) {MsFuncRcdDisp();}	
			else if(strcmp((void *)pCmd[1],"sec")==0) {SecFuncRcdDisp();}	
			else if(strcmp((void *)pCmd[1],"event")==0) {EventDebug();}
			else if(strcmp((void *)pCmd[1],"ctrl")==0) {ControllerDebug();}
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"def")==0)
	{
		DefaultConfig();
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_E(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"erase")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			Debug("----------------------------------------------\n\r");
			Debug("Erase bulk \n\r");
			Debug("Erase sector Start_Sector Sector_Num\n\r");
			Debug("----------------------------------------------\n\r");
		}
		else if(NotNullStr(pCmd[1]))
		{
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"eee")==0)
	{
		u32 a=Str2Uint(pCmd[1]);
		u32 b=Str2Uint(pCmd[2]);

		Debug("ieee2f %f = %x\n\r",110.80,Float2Ieee(110.80));
		Debug("ieee2f %f = %x\n\r",0.124,Float2Ieee(0.124));
		Debug("float2i %x = %f\n\r",0x3DFDF3B6,Ieee2Float(0x3DFDF3B6));
		Debug("float2i %x = %f\n\r",0x42DDCC80,Ieee2Float(0x42DDCC80));
		
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_F(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"flash")==0)
	{
		if(strcmp((void *)pCmd[1],"wen")==0)	
		{
			if(Str2Uint(pCmd[2]))	W25Q_Write_Enable();
			else W25Q_Write_Disable();
		}
		else if(strcmp((void *)pCmd[1],"wp")==0)
		{
			if(IsNullStr(pCmd[2]))
			{
				Debug("Now Wp=%u\n\r",IOOUT_ReadIoStatus(IOOUT_FLASH_WP));
			}
			else
			{
				if(Str2Uint(pCmd[2]))	IOOUT_SetIoStatus(IOOUT_FLASH_WP,TRUE);
				else IOOUT_SetIoStatus(IOOUT_FLASH_WP,FALSE);
			}
		}
		else if(strcmp((void *)pCmd[1],"protect")==0)
		{
			if(IsNullStr(pCmd[2]))
			{
				Debug("Now protect=%u\n\r",W25Q_IsProtect());
			}
			else
			{
				if(Str2Uint(pCmd[2]))	W25Q_Protect(TRUE);
				else W25Q_Protect(FALSE);
			}
		}		
		else if(strcmp((void *)pCmd[1],"status")==0)	
		{
			Debug("StateReg:0x%x\n\r",W25Q_Read_Status_Reg());
		}
		else if(strcmp((void *)pCmd[1],"wtsta")==0)	
		{
			W25Q_Set_Status_Reg((W25Q_MEM_PROTE)Str2Uint(pCmd[2]),(W25Q_STA_REG_PROTE)Str2Uint(pCmd[3]));
		}
		else if(strcmp((void *)pCmd[1],"read_page")==0)
		{
			u32 Page=Str2Uint(pCmd[2]);
			u16 Num=Str2Uint(pCmd[3]);
			char *pPageData=Q_Malloc(W25Q_PAGE_SIZE);
			u16 i=0;

			if(Num==0) Num=1;

			for(i=0;i<Num;i++,Page++)
			{
				W25Q_Read_Data(Page*W25Q_PAGE_SIZE,W25Q_PAGE_SIZE,pPageData);
				Debug("Page %d Data:\r\n",Page);
				DisplayBuf(pPageData,W25Q_PAGE_SIZE,16);
			}
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"read")==0)
		{
			u32 Addr=Str2Uint(pCmd[2]);
			u16 Num=Str2Uint(pCmd[3]);
			char *pPageData=Q_Malloc(Num);
			u32 Now=GetSysStartMs();
			W25Q_Read_Data(Addr,Num?Num:1,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,Num,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"read_fast")==0)
		{
			u32 Addr=Str2Uint(pCmd[2]);
			u16 Num=Str2Uint(pCmd[3]);
			char *pPageData=Q_Malloc(Num);
			u32 Now=GetSysStartMs();
			W25Q_Fast_Read_Data(Addr,Num?Num:1,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,Num,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"read_sec")==0)
		{
			u32 Addr=Str2Uint(pCmd[2])*FLASH_SECTOR_BYTES;
			u16 Page=Str2Uint(pCmd[3])*FLASH_PAGE_SIZE;
			u16 PageNum=Str2Uint(pCmd[4]);
			char *pPageData=NULL;
			u32 Now=GetSysStartMs();

			if(PageNum==0) PageNum=1;
			if(PageNum>16) PageNum=16;
			pPageData=Q_Malloc(PageNum*FLASH_PAGE_SIZE);
			W25Q_Read_Data(Addr+Page,PageNum*FLASH_PAGE_SIZE,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,PageNum*FLASH_PAGE_SIZE,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"read_minsec")==0)
		{
			u32 Addr=Str2Uint(pCmd[2])*FLASH_MIN_SEC_BYTES;
			u16 Page=Str2Uint(pCmd[3])*FLASH_PAGE_SIZE;
			u16 PageNum=Str2Uint(pCmd[4]);
			char *pPageData=NULL;
			u32 Now=GetSysStartMs();

			if(PageNum==0) PageNum=1;
			if(PageNum>16) PageNum=16;
			pPageData=Q_Malloc(PageNum*FLASH_PAGE_SIZE);
			W25Q_Read_Data(Addr+Page,PageNum*FLASH_PAGE_SIZE,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,PageNum*FLASH_PAGE_SIZE,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"write")==0)
		{
			u32 Addr=Str2Uint(pCmd[2]);
			u16 Len=Str2Uint(pCmd[3]);
			char Data=Str2Uint(pCmd[4]);
			char *pPageData=Q_Malloc(Len);
			u32 Now=GetSysStartMs();
			
			MemSet(pPageData,Data,Len);
			W25Q_Program(Addr,Len,pPageData);

			Debug("Write %dmS\n\r",GetSysStartMs()-Now);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pCmd[1],"erase")==0)
		{
			u32 Addr=Str2Uint(pCmd[2]);
			u8 Type=Str2Uint(pCmd[3]);
			u32 Now;

			if(NotNullStr(pCmd[2]))
			{
				Debug("Start Erase...\n\r");
				Now=GetSysStartMs();
				W25Q_Erase(Addr,Type);
				Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
			}
		}
		else if(strcmp((void *)pCmd[1],"erase_sec")==0)
		{
			u32 Sec=Str2Uint(pCmd[2]);
			u32 Num=Str2Uint(pCmd[3]);
			u32 Now;

			if(Sec && Num)
			{
				Debug("Start Erase Sec...\n\r");
				Now=GetSysStartMs();
				SpiFlsEraseSector(Sec,Num);
				Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
			}
		}
		else if(strcmp((void *)pCmd[1],"erase_minsec")==0)
		{
			u32 Sec=Str2Uint(pCmd[2]);
			u32 Num=Str2Uint(pCmd[3]);
			u32 Now;

			if(Sec && Num)
			{
				Debug("Start Erase Sec...\n\r");
				Now=GetSysStartMs();

				for(;Num;Num--,Sec++)
				{
					SpiFlsEraseMinSec(Sec*FLASH_MIN_SEC_BYTES);
				}
				
				Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
			}
		}
		else if(strcmp((void *)pCmd[1],"erase_chip")==0)
		{
			u32 Now;

			Debug("Start Erase...\n\r");
			Now=GetSysStartMs();
			W25Q_Bulk_Erase();
			Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
		}
		
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_G(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_H(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"heap")==0)
	{
		QHeapDebug();
		
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_I(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"ioset")==0)
	{
		IOIN_SetIoStatus((IO_IN_DEFS)(Str2Uint(pCmd[1])+IOIN_PIO0),Str2Uint(pCmd[2])?TRUE:FALSE);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ir")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			
		}
		else if(strcmp((void *)pCmd[1],"start")==0)
		{
			Debug("Start Recv Ir\r\n");
			StartRecvIr();		
		}
		else if(strcmp((void *)pCmd[1],"stop")==0)
		{
			Debug("Stop Recv Ir\r\n");
			StopRecvIr();
		}
		else if(strcmp((void *)pCmd[1],"send")==0)
		{
			Debug("Start Send Ir\r\n");
			StartSendIr(NULL);
		}
		else if(strcmp((void *)pCmd[1],"capture")==0)
		{

		}

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_J(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_K(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_L(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_M(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_N(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_O(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_P(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"pwm1")==0)
	{
		u32 Val=Str2Uint(pCmd[1]);
		u32 uS_Base=Str2Uint(pCmd[2]);
		u32 Pluse=Str2Uint(pCmd[3]);

		PWM1_CONFIG(Val,uS_Base,Pluse);

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"pwm2")==0)
	{
		u32 Val=Str2Uint(pCmd[1]);
		u32 uS_Base=Str2Uint(pCmd[2]);
		u32 Pluse=Str2Uint(pCmd[3]);

		PWM2_CONFIG(Val,uS_Base,Pluse);

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"pin")==0)
	{
		u32 IO=Str2Uint(pCmd[1]);
		Debug("PIN[%u]=%u\n\r",IO,IOIN_ReadIoStatus((IO_IN_DEFS)(IO-1+IOIN_PIO0)));

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_Q(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"qcom")==0)
	{
		if(strcmp((void *)pCmd[1],"last")==0)
		{
			Debug("LastCmd:%s\n\r",QCom_GetLastCmd());
			return TRUE;
		}
		else if(strcmp((void *)pCmd[1],"var")==0)
		{
			if(NotNullStr(pCmd[2]) && NotNullStr(pCmd[3]))//set
			{
				QCom_SetVarValue(pCmd[2],Str2Sint(pCmd[3]),TRUE);
				return TRUE;
			}
			else if(NotNullStr(pCmd[2]))//read
			{
				QCom_GetVarValue(pCmd[2]);
				return TRUE;
			}
		}
		else if(strcmp((void *)pCmd[1],"str")==0)
		{
			if(NotNullStr(pCmd[2]) && NotNullStr(pCmd[3])) QCom_SendStr(Str2Uint(pCmd[2]),&pStrCopy[10+strlen(pCmd[2])]);
			else if(NotNullStr(pCmd[2])) QCom_SendStr(0,pCmd[2]);
			return TRUE;
		}
		else if(strcmp((void *)pCmd[1],"msg")==0)
		{
			if(NotNullStr(pCmd[2]) && NotNullStr(pCmd[3])) QCom_SendMsg(Str2Uint(pCmd[2]),&pStrCopy[10+strlen(pCmd[2])]);
			else if(NotNullStr(pCmd[2])) QCom_SendMsg(1<<SMF_PUSH,pCmd[2]);
			return TRUE;
		}
		else if(strcmp((void *)pCmd[1],"sta")==0)
		{
			QCom_SendSta();
			return TRUE;
		}
		else if(strcmp((void *)pCmd[1],"reset")==0)
		{
			QCom_ResetQwifi();
			return TRUE;
		}		
	}
	return FALSE;
}


static bool __inline SysCmdHandler_R(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"reset")==0)
	{
		RebootBoard();//等死
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_S(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"save")==0)
	{
		Debug("Save System Parameter\n\r");
		//RFS_BurnToRom();
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"setio")==0)
	{
		IOIN_SetIoStatus((IO_IN_DEFS)(IOIN_PIO0+Str2Uint(pCmd[1])),Str2Uint(pCmd[2])?TRUE:FALSE);
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_T(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"test")==0)
	{
		if(IsNullStr(pCmd[1]))//help
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

static bool __inline SysCmdHandler_U(char **pCmd,const char *pStrCopy,char *pOutStream)
{}

static bool __inline SysCmdHandler_V(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"version")==0)
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

static bool __inline SysCmdHandler_W(char **pCmd,const char *pStrCopy,char *pOutStream)
{

	return FALSE;
}

static bool __inline SysCmdHandler_X(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_Y(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_Z(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

bool CustomCmdHandler(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

typedef bool (*pCmdHandler)(char **,const char *,char *);
const pCmdHandler gpCmdHandlers[]={
SysCmdHandler_A,SysCmdHandler_B,SysCmdHandler_C,SysCmdHandler_D,SysCmdHandler_E,SysCmdHandler_F,
SysCmdHandler_G,SysCmdHandler_H,SysCmdHandler_I,SysCmdHandler_J,SysCmdHandler_K,SysCmdHandler_L,
SysCmdHandler_M,SysCmdHandler_N,SysCmdHandler_O,SysCmdHandler_P,SysCmdHandler_Q,SysCmdHandler_R,
SysCmdHandler_S,SysCmdHandler_T,SysCmdHandler_U,SysCmdHandler_V,SysCmdHandler_W,SysCmdHandler_X,
SysCmdHandler_Y,SysCmdHandler_Z
};

#define UART_CMD_MAX_PARAM_NUM 6//最长参数

//通用的串口输入处理
//Len 字符串个数
//pStr 字符串
//将处理如下命令:
bool SysCmdHandler(u16 Len,const char *pStr,char *pOutStream)
{
	char *pParam[UART_CMD_MAX_PARAM_NUM+1];
	char *pBuf=NULL;
	u16 i,n;
	bool Res=FALSE;
	char FirstByts=0;
	
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
			Debug("\r\n");
		}
		else
		{
			Debug("\r\n");
			//CDebug("CtrlCode:%x\r\n",((u16 *)pStr)[0]);
		}	
		return Res;
	}

	for(i=0;i<(UART_CMD_MAX_PARAM_NUM+1);i++) pParam[i]=NULL;

	pBuf=Q_Malloc(Len+2);
	StrCmdParse(pStr,pParam,pBuf,TRUE);//解析指令和参数
	Debug("\r\n");
	
	FirstByts=pStr[0];
	if(FirstByts=='#') Res=CustomCmdHandler(pParam,pStr,pOutStream);
	else if(FirstByts>='a' && FirstByts<='z') Res=gpCmdHandlers[FirstByts-'a'](pParam,pStr,pOutStream);
	else Res=FALSE;
	
	if(Res==FALSE) 
	{
		Debug("No Such Cmd[%u]:%s\r\n",Len,pBuf);
	}
	
	Q_Free(pBuf);
	
	return Res;
}

