//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了一个字符串解析函数库
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "StrParse.h"

//处理类似&typ=1&exp=1539865800&cnt=5的参数字符串
//name解析到pParam
//值解析到pVal
//返回解析的参数个数
u8 StrParamParse(char *pParamStr,char **pName,char **pVal)
{
	u16 StrLen=strlen(pParamStr);
	u16 Num=0,i,n,m;
	
	if(pParamStr==NULL || pParamStr[0]==0 || StrLen==0) return 0;

	//Debug("Str[%u]:%s\n\r",StrLen,pParamStr);

	//分离参数
	pName[Num++]=pParamStr;
	for(i=0;i<StrLen;i++)
	{
		if(pParamStr[i]=='&' && pParamStr[i+1])
		{
			pParamStr[i]=0;
			if(Num>=STR_PARAM_MAX_NUM) break;
			pName[Num++]=&pParamStr[i+1];
		}
	}

	//分离参数值
	for(i=0;i<Num;i++)
	{
		char *pChr=strchr(pName[i],'=');
		if(pChr!=NULL)
		{
			pChr[0]=0;
			pChr++;
			pVal[i]=pChr;
		}
	}

	//参数值转码
	for(i=0;i<Num;i++)
	{
		char *pValTmp=pVal[i];
		char AsciiChr=0;
		u16 ValLen=strlen(pValTmp);
		
		n=0;m=0;
		while(pValTmp[n])
		{
			if(pValTmp[n]=='%')
			{
				char NumStr[6];
				NumStr[0]='0';
				NumStr[1]='x';
				NumStr[2]=pValTmp[n+1];
				NumStr[3]=pValTmp[n+2];
				NumStr[4]=0;

				AsciiChr=Str2Uint(NumStr)&0xff;
				if(AsciiChr<0x80)//ascii
				{
					pValTmp[m]=AsciiChr;
					n+=2;	
				}
				else //utf-8 code
				{
					pValTmp[m]=0xA8;
					pValTmp[m+1]=0x81;
					m+=1;
					n+=8;
				}								
			}
			else
			{
				if(pValTmp[n]=='+') pValTmp[m]=' ';//加号转空格
				else pValTmp[m]=pValTmp[n];
			}
			
			n++;
			m++;

			if(n >= ValLen) break;
		}

		pValTmp[m]=0;
	}

	return Num;
}

//通用的串口输入处理，处理以空格间隔的字符串命令
//pStr 字符串
//pRet 返回的指针数组，[0]指向cmd，[1]指向第一个参数，以此类推
//pBuf 外部自行申请的buf，函数调用完成后，需自行free
//CmdToLower 命令名转小写
//返回处理的参数个数，包括命令
u8 StrCmdParse(const char *pCmdStr,char **pRet,char *pBuf,bool CmdToLower)
{
	u16 StrLen=strlen(pCmdStr);
	u16 Num=0,i;
	
	if(StrLen==0 || pCmdStr==NULL || pRet==NULL || pBuf==NULL)//控制字符
	{
		return 0;
	}

	strcpy(pBuf,pCmdStr);//拷贝到buf中，用于参数分解

	if(CmdToLower)
	{
		for(i=0;i<StrLen;i++)//命令字符串全部转小写
		{
			if(pBuf[i]>='A' && pBuf[i]<='Z')
			{
				pBuf[i]=pBuf[i]+32;
			}
			else if(pBuf[i]==' ')
			{
				break;
			}
		}
	}
	
	pRet[Num++]=pBuf;
	for(i=0;pBuf[i];i++)//取参数
	{
		if(pBuf[i]==' ')
		{
			pBuf[i]=0;
			if(pBuf[i+1]&&pBuf[i+1]!=' ')
			{
				if(Num>=STR_CMD_MAX_PARAM_NUM) break;
				pRet[Num++]=&pBuf[i+1];
			}
		}
	}
	
	return Num;
}


