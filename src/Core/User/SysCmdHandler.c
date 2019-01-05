#include "SysDefines.h"
#include "Product.h"


static bool __inline SysCmdHandler_A(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_B(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{


	return FALSE;
}

static bool __inline SysCmdHandler_C(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{

	
	return FALSE;
}

static bool __inline SysCmdHandler_D(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd,"debug")==0)
	{
		if(IsNullStr(pParam[0]))//help
		{
			RFS_Debug();
		}
		else if(NotNullStr(pParam[0]))
		{
			//INFO_RECORD_NAME Name;
			//u16 Idx=1,Num=0xffff;
			
			//if(NotNullStr(pParam[1])) Idx=StrToUint(pParam[1]);
			//if(NotNullStr(pParam[2])) Num=StrToUint(pParam[2]);

			if(strcmp((void *)pParam[0],"sys")==0) {RFS_Debug();}
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

static bool __inline SysCmdHandler_E(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_F(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd,"flash")==0)
	{
		if(strcmp((void *)pParam[0],"wen")==0)	
		{
			if(StrToUint(pParam[1]))	W25Q_Write_Enable();
			else W25Q_Write_Disable();
		}
		else if(strcmp((void *)pParam[0],"wp")==0)
		{
			if(IsNullStr(pParam[1]))
			{
				Debug("Now Wp=%u\n\r",IOOUT_ReadIoStatus(IOOUT_FLASH_WP));
			}
			else
			{
				if(StrToUint(pParam[1]))	IOOUT_SetIoStatus(IOOUT_FLASH_WP,TRUE);
				else IOOUT_SetIoStatus(IOOUT_FLASH_WP,FALSE);
			}
		}
		else if(strcmp((void *)pParam[0],"protect")==0)
		{
			if(IsNullStr(pParam[1]))
			{
				Debug("Now protect=%u\n\r",W25Q_IsProtect());
			}
			else
			{
				if(StrToUint(pParam[1]))	W25Q_Protect(TRUE);
				else W25Q_Protect(FALSE);
			}
		}		
		else if(strcmp((void *)pParam[0],"status")==0)	
		{
			Debug("StateReg:0x%x\n\r",W25Q_Read_Status_Reg());
		}
		else if(strcmp((void *)pParam[0],"wtsta")==0)	
		{
			W25Q_Set_Status_Reg((W25Q_MEM_PROTE)StrToUint(pParam[1]),(W25Q_STA_REG_PROTE)StrToUint(pParam[2]));
		}
		else if(strcmp((void *)pParam[0],"read_page")==0)
		{
			u32 Page=StrToUint(pParam[1]);
			u16 Num=StrToUint(pParam[2]);
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
		else if(strcmp((void *)pParam[0],"read")==0)
		{
			u32 Addr=StrToUint(pParam[1]);
			u16 Num=StrToUint(pParam[2]);
			char *pPageData=Q_Malloc(Num);
			u32 Now=GetSysStartMs();
			W25Q_Read_Data(Addr,Num?Num:1,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,Num,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pParam[0],"read_fast")==0)
		{
			u32 Addr=StrToUint(pParam[1]);
			u16 Num=StrToUint(pParam[2]);
			char *pPageData=Q_Malloc(Num);
			u32 Now=GetSysStartMs();
			W25Q_Fast_Read_Data(Addr,Num?Num:1,pPageData);Debug("Read %dMs\n\r",GetSysStartMs()-Now);
			Debug("Addr 0x%x(%u) Data:\r\n",Addr,Addr);
			DisplayBuf(pPageData,Num,16);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pParam[0],"read_sec")==0)
		{
			u32 Addr=StrToUint(pParam[1])*FLASH_SECTOR_BYTES;
			u16 Page=StrToUint(pParam[2])*FLASH_PAGE_SIZE;
			u16 PageNum=StrToUint(pParam[3]);
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
		else if(strcmp((void *)pParam[0],"read_minsec")==0)
		{
			u32 Addr=StrToUint(pParam[1])*FLASH_MIN_SEC_BYTES;
			u16 Page=StrToUint(pParam[2])*FLASH_PAGE_SIZE;
			u16 PageNum=StrToUint(pParam[3]);
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
		else if(strcmp((void *)pParam[0],"write")==0)
		{
			u32 Addr=StrToUint(pParam[1]);
			u16 Len=StrToUint(pParam[2]);
			char Data=StrToUint(pParam[3]);
			char *pPageData=Q_Malloc(Len);
			u32 Now=GetSysStartMs();
			
			MemSet(pPageData,Data,Len);
			W25Q_Program(Addr,Len,pPageData);

			Debug("Write %dmS\n\r",GetSysStartMs()-Now);
			Q_Free(pPageData);
		}
		else if(strcmp((void *)pParam[0],"erase")==0)
		{
			u32 Addr=StrToUint(pParam[1]);
			u8 Type=StrToUint(pParam[2]);
			u32 Now;

			if(NotNullStr(pParam[1]))
			{
				Debug("Start Erase...\n\r");
				Now=GetSysStartMs();
				W25Q_Erase(Addr,Type);
				Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
			}
		}
		else if(strcmp((void *)pParam[0],"erase_sec")==0)
		{
			u32 Sec=StrToUint(pParam[1]);
			u32 Num=StrToUint(pParam[2]);
			u32 Now;

			if(Sec && Num)
			{
				Debug("Start Erase Sec...\n\r");
				Now=GetSysStartMs();
				SpiFlsEraseSector(Sec,Num);
				Debug("Finish %dmS\n\r",GetSysStartMs()-Now);
			}
		}
		else if(strcmp((void *)pParam[0],"erase_minsec")==0)
		{
			u32 Sec=StrToUint(pParam[1]);
			u32 Num=StrToUint(pParam[2]);
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
		else if(strcmp((void *)pParam[0],"erase_chip")==0)
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

static bool __inline SysCmdHandler_G(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_H(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd,"heap")==0)
	{
		DebugHeap();
		QS_MonitorFragment();
		
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_I(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd,"ioset")==0)
	{
		IOIN_SetIoStatus((IO_IN_DEFS)(StrToUint(pParam[0])+IOIN_PIO0),StrToUint(pParam[1])?TRUE:FALSE);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_K(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_M(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_N(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	return FALSE;
}

static bool __inline SysCmdHandler_P(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_Q(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{

	return FALSE;
}


static bool __inline SysCmdHandler_R(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd,"reset")==0)
	{
		RebootBoard();//等死
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_S(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_T(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_U(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{

	return FALSE;
}

static bool __inline SysCmdHandler_V(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
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

static bool __inline SysCmdHandler_W(char *pCmd,char **pParam,char *StrCopyBuf,char *pOutStream)
{

	return FALSE;
}

//通用的串口输入处理
//Len 字符串个数
//pStr 字符串
//将处理如下命令:
#define UART_CMD_MAX_PARAM_NUM 6//最长参数
#define COM_CMD_STR_LEN 128
bool SysCmdHandler(u16 Len,char *pStr,char *pOutStream)
{
	u16 i,n;
	char *pCmd=NULL;
	char *pParam[UART_CMD_MAX_PARAM_NUM]={NULL,NULL,NULL,NULL,NULL,NULL};
	char *pStrCopyBuf=NULL;
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

