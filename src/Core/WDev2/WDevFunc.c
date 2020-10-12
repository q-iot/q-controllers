#include "SysDefines.h"

//检查地址是否在内
bool MyAddrInclude(WCON_PACKET *pPkt)
{
	u16 i;

	if(pPkt->Num==0 || pPkt->DataLen==0)
	{
		pPkt->Num=pPkt->DataLen=0;
		return FALSE;
	}

	for(i=0;i<pPkt->Num;i++)
	{
		u16 Addr=0;

		if(WPktFindAttrib_Array(pPkt,WAA_SET,WDA_DEV_LIST_ADDR,i,&Addr)==0) break;
		if(Addr==LBit16(WNetMyAddr())) return TRUE;	
	}

	return FALSE;
}

#if PRODUCT_IS_JUMPER
void SendRfToHost(RF_RECORD *pRf)
{
	if(pRf->Type==SIT_RF_OTHERS)
	{
		if(SysVars()->RfStdOnly==TRUE) return;
	}
	else if(pRf->Type==SIT_RF_STD)
	{
	}
	else
	{
		return;
	}

	if(RFS_DB()->RFSI_FLY_ADDR)
	{
		WSIG_PACKET *pSig=Q_Malloc(320);

		pSig->Type=WPT_CAPTURE;
		pSig->Res=WPR_MAIN;
		pSig->PktCnt=GetWNetPktCnt();
		pSig->SigType=WST_RF;
		pSig->Vendor=0;
		pSig->Num=1;
		pSig->DataLen=SigRecordVaildSize(pRf);

		if(pSig->DataLen)
		{
			MemCpy(pSig->Data,pRf,pSig->DataLen);
			WNetSendPkt(RFS_DB()->RFSI_FLY_ADDR,(void *)pSig,NULL);
		}
		
		Q_Free(pSig);
	}	
}

void SendIrToHost(IR_RECORD *pIr)
{
	if(pIr->Type==SIT_IR)
	{
	}
	else
	{
		return;
	}

	if(RFS_DB()->RFSI_FLY_ADDR)
	{
		WSIG_PACKET *pSig=Q_Malloc(320);

		pSig->Type=WPT_CAPTURE;
		pSig->Res=WPR_MAIN;
		pSig->PktCnt=GetWNetPktCnt();
		pSig->SigType=WST_IR;
		pSig->Vendor=0;
		pSig->Num=1;
		pSig->DataLen=SigRecordVaildSize(pIr);

		if(pSig->DataLen)
		{
			MemCpy(pSig->Data,pIr,pSig->DataLen);
			WNetSendPkt(RFS_DB()->RFSI_FLY_ADDR,(void *)pSig,NULL);
		}
		
		Q_Free(pSig);
	}	
}

//通知主机，上报信号
void SendRfOppoToHost(u32 Code,u16 BasePeriod,bool IsCrf)
{
	WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
	u32 FlyAddr=RFS_DB()->RFSI_FLY_ADDR;
	u16 PktSn;
	
	if(FlyAddr)
	{
		//Debug("SendRfOppoToHost:%d[0x%x]\n\r",FlyAddr,FlyAddr);

		pConPkt->Type=WPT_CONFIG;
		pConPkt->Res=WPR_MAIN;
		pConPkt->PktCnt=GetWNetPktCnt();

		pConPkt->Num=pConPkt->DataLen=0;
		{
			u16 Val[4];
			MemCpy(Val,&Code,4);
			Val[2]=BasePeriod;
			if(IsCrf)
			{
				Debug("   INFORM CRF 0x%x@%duS\n\r",Code,BasePeriod);
				WPktSetAttrib_Array(pConPkt,WAA_SET,WDA_CRF_CODE_U32_U16_U16,8,Val);
			}
			else
			{
				Debug("   INFORM RF 0x%x@%duS\n\r",Code,BasePeriod);
				WPktSetAttrib_Array(pConPkt,WAA_SET,WDA_RF_CODE_U32_U16,6,Val);
			}
		}		
					
		PktSn=WNetSendPkt(FlyAddr,(void *)pConPkt,NULL);
		if(PktSn==0)
		{
			Debug("Inform Faild!\n\r");
		}
	}

	Q_Free(pConPkt);
}
#endif

#if OPEN_WDEV_ATTACK
static u16 AttackHostCnt=0;
static void AttackHost_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(--AttackHostCnt)
	{
		AttackReplyAddClientToHost(pBlock->DstAddr,pBlock->SrcAddr>>16,0);
	}
	else
	{
		gWNetMyAddr=FlyAddr(Rand(0xffff));//回到fly身份
	}
}

//攻击主机发出大量的的搜索包
void AttackReplyAddClientToHost(u32 FlyAddr,u16 Group,u16 TotalCnt)
{
	WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
	u16 PktSn;

	if(GroupAddr(FlyAddr)==WDG_FLY)
	{
		Debug("AttackReplyAddClientToHost[%u]\n\r",FlyAddr&0xffff);
		if(TotalCnt) AttackHostCnt=TotalCnt;
		
		pConPkt->Type=WPT_ADD_CLIENT;
		pConPkt->Res=WPR_SUCESS;
		pConPkt->PktCnt=Rand(0xffff);

		//告知版本号，配置参数
		pConPkt->Num=pConPkt->DataLen=0;
		WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
		WPktSetAttrib(pConPkt,WAA_SET,WDA_HOST_ADDR,0);

		gWNetMyAddr=(Group<<16)|Rand(0xffff);
		PktSn=WNetSendPkt(FlyAddr,(void *)pConPkt,(pStdFunc)AttackHost_CB);
		if(PktSn==0)
		{
			Debug("AddClient Faild!\n\r");
		}
	}

	Q_Free(pConPkt);
}

//发送主机才能发送的同步包，对回包的设备进行解绑攻击
void SendSyncAttack(u16 Group)
{
	WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
	u16 PktSn;

	Debug("To Attack Group[%u]\n\r",Group);

	pConPkt->Type=WPT_ADD_CLIENT;
	pConPkt->Res=WPR_MAIN;
	pConPkt->PktCnt=GetWNetPktCnt();
	pConPkt->Num=pConPkt->DataLen=0;

	WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);

	PktSn=WNetSendPkt((Group<<16)|0xffff,(void *)pConPkt,NULL);
	if(PktSn==0)
	{
		Debug("Attack Faild!\n\r");
	}

	Q_Free(pConPkt);
}

//发送主机才能发送的强制恢复命令
void SendWDevConfigAttack(u32 WDevAddr)
{
	WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
	u16 PktSn;

	Debug("To Attack WDev[%u.%u]\n\r",WDevAddr>>16,WDevAddr&0xffff);

	pConPkt->Type=WPT_CONFIG;
	pConPkt->Res=WPR_MAIN;
	pConPkt->PktCnt=GetWNetPktCnt();
	pConPkt->Num=pConPkt->DataLen=0;

	WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
	WPktSetAttrib(pConPkt,WAA_SET,WDA_PASSWORD,LBit16(WDevAddr)^0xa5a5);

	PktSn=WNetSendPkt(WDevAddr,(void *)pConPkt,NULL);
	if(PktSn==0)
	{
		Debug("Attack Faild!\n\r");
	}

	Q_Free(pConPkt);
}

//发送主机才能发送的量产包
void SendWaverConfigAttack(u16 WaverAddr)
{
	WWAV_PACKET *pWavPkt=Q_Malloc(sizeof(WWAV_PACKET));
	WAVER_PARAM_TAB *pTab=pWavPkt->Tab;
	u16 PktSn;

	Debug("To Attack Waver[%u]\n\r",WaverAddr);

	MemSet(pWavPkt,0,sizeof(WWAV_PACKET));

	pWavPkt->Type=WPT_WAVER_CONFIG;
	pWavPkt->Res=WPR_MAIN;
	pWavPkt->PktCnt=GetWNetPktCnt();

	pTab->ProdID=Rand(0xffff);
	pTab->TabVer=1;
	pTab->DutType=WDT_WDEV_IO;
	pTab->WNetGroup=Rand(0xffff);
	
	PktSn=WNetSendPkt(WavAddr(WaverAddr),(void *)pWavPkt,NULL);
	if(PktSn==0)
	{
		Debug("Attack Faild!\n\r");
	}

	Q_Free(pWavPkt);
}
#endif

#if PRODUCT_IS_WAVER || PRODUCT_IS_JUMPER
void RespAddClientToHost(u16 PktCnt,u32 FlyAddr)
{
	WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
	u16 PktSn;

	if(GroupAddr(FlyAddr)==WDG_FLY)
	{
		//Debug("RespAddClientToHost[0x%x]\n\r",FlyAddr);

		pConPkt->Type=WPT_ADD_CLIENT;
		pConPkt->Res=WPR_SUCESS;
		pConPkt->PktCnt=PktCnt;

		//告知版本号，配置参数
		pConPkt->Num=pConPkt->DataLen=0;
		WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
		WPktSetAttrib(pConPkt,WAA_SET,WDA_HOST_ADDR,RFS_DB()->RFSI_FLY_ADDR);
		
		PktSn=WNetSendPkt(FlyAddr,(void *)pConPkt,NULL);
		if(PktSn==0)
		{
			Debug("AddClient Faild!\n\r");
		}
	}

	Q_Free(pConPkt);
}

static void SyncToHost_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(TransRes==WBR_OK)
	{
		HostSyncFinishHook(TRUE);
	}
	else
	{
		HostSyncFinishHook(FALSE);
	}
}

//主动带过去自己的变量
void SyncToHost(int a,void *p)
{
	WCON_PACKET *pConPkt=NULL;

	if(RFS_GetConfig()->ProdID==0) return;
	if(RFS_DB()->RFSI_FLY_ADDR==0) return;

	Debug("SyncToHost\n\r");
	
	pConPkt=(void *)Q_Malloc(128);
	
	pConPkt->Type=WPT_SYNC;
	pConPkt->Res=WPR_MAIN;
	pConPkt->PktCnt=GetWNetPktCnt();

	pConPkt->Num=pConPkt->DataLen=0;
	WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
	{
		VAR_OBJ *pVarObjs=Q_Malloc(sizeof(VAR_OBJ));
		u8 i;
		
		for(i=0;i<RFS_GetConfig()->VarNum;i++)
		{
			MemSet(pVarObjs,0,sizeof(VAR_OBJ));
			pVarObjs->VarOpt=VOT_SET;
			pVarObjs->VarState=SelfVarGetState(i);
			pVarObjs->VarIdx=i+1;
			pVarObjs->VarIdxType=VIT_SELF;
			pVarObjs->VarValue=SelfVarRead(i);								

			WPktSetAttrib_Array(pConPkt,WAA_SET,WDA_VAR_OBJ_U32,4,(void *)pVarObjs);//告知自身属性值
		}				

		Q_Free(pVarObjs);
	}
	
	WNetSendPkt(RFS_DB()->RFSI_FLY_ADDR,(void *)pConPkt,(pStdFunc)SyncToHost_CB);

	Q_Free(pConPkt);
}

//通过同步包将自己的变量状态发给主机
void ReplySyncToHost(u16 PktCnt,u32 FlyAddr)
{
	WCON_PACKET *pConPkt=NULL;

	if(RFS_GetConfig()->ProdID==0) return;

	Debug("Reply Sync To Fly\n\r");

	pConPkt=(void *)Q_Malloc(128);
	
	if(RFS_DB()->RFSI_FLY_ADDR==FlyAddr)
	{
		pConPkt->Type=WPT_SYNC;
		pConPkt->Res=WPR_SUCESS;
		pConPkt->PktCnt=PktCnt;

		pConPkt->Num=pConPkt->DataLen=0;
		//WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);//此处是违反协议规定的，但是为了屏蔽之前的版本的漏洞固件，故加上。
		{
			VAR_OBJ *pVarObjs=Q_Malloc(sizeof(VAR_OBJ));
			u8 i;
			
			for(i=0;i<RFS_GetConfig()->VarNum;i++)
			{
				MemSet(pVarObjs,0,sizeof(VAR_OBJ));
				pVarObjs->VarOpt=VOT_SET;
				pVarObjs->VarState=SelfVarGetState(i);
				pVarObjs->VarIdx=i+1;
				pVarObjs->VarIdxType=VIT_SELF;
				pVarObjs->VarValue=SelfVarRead(i);	
				WPktSetAttrib_Array(pConPkt,WAA_SET,WDA_VAR_OBJ_U32,4,(void *)pVarObjs);//告知自身属性值							
			}					
			
			Q_Free(pVarObjs);
		}	
	}
	else
	{
		pConPkt->Type=WPT_SYNC;
		pConPkt->Res=WPR_AUTH_ERROR;
		pConPkt->PktCnt=PktCnt;

		//告知版本号，配置参数
		pConPkt->Num=pConPkt->DataLen=0;
		//WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);//此处是违反协议规定的，但是为了屏蔽之前的版本的漏洞固件，故加上。
		WPktSetAttrib(pConPkt,WAA_SET,WDA_HOST_ADDR,RFS_DB()->RFSI_FLY_ADDR);
	}
	
	if(WNetSendPkt(FlyAddr,(void *)pConPkt,NULL))
	{
		if(pConPkt->Res==WPR_SUCESS) HostSyncFinishHook(TRUE);
	}

	Q_Free(pConPkt);
}

//发送变量给主机
void SendVarsToHost(int VarIdx,void *p)
{
	if(RFS_DB()->RFSI_FLY_ADDR==0) return;
		
	if(VarIdx && VarIdx<=SELF_VAR_TOTAL) //发送指定
	{
		HostChangeVarFlag(VarIdx-1);
	}
	else if(VarIdx==0)//发送所有
	{
		u8 i;
		for(i=0;i<RFS_GetConfig()->VarNum;i++) HostChangeVarFlag(i);
	}
}

static void SendStrToHost_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(TransRes==WBR_OK)
	{
		//Debug("Str To Fly OK %d\n\r",pBlock->PktSn);
		WCON_PACKET *pConPkt=(void *)pBlock->pData;
	}
	else
	{
		Debug("Sent Str Faild!\n\r");
	}
}

//发送字符串给主机
void SendStrToHost(int StrID,void *pStr,bool NeedFreeStr)
{
	u32 FlyAddr=RFS_DB()->RFSI_FLY_ADDR;

	if(FlyAddr)
	{
		WSTR_PACKET *pStrPkt=0;
		u16 Bytes=0;
		u16 PktSn;		
		
		if(pStr!=NULL) Bytes=strlen(pStr)+1;
		pStrPkt=Q_Malloc(16+Bytes);

		pStrPkt->Type=WPT_DEV_STR;
		pStrPkt->Res=WPR_MAIN;
		pStrPkt->PktCnt=GetWNetPktCnt();
		
		pStrPkt->Num=4;
		pStrPkt->DataLen=Bytes+4;

		MemCpy(pStrPkt->Data,&StrID,4);
		if(Bytes) MemCpy(&pStrPkt->Data[4],pStr,Bytes);

		PktSn=WNetSendPkt(FlyAddr,(void *)pStrPkt,(pStdFunc)SendStrToHost_CB);
		if(PktSn==0)
		{
			Debug("Send Str State Faild!\n\r");
		}
		
		Q_Free(pStrPkt);
		if(NeedFreeStr && pStr!=NULL && IsHeapRam(pStr)) Q_Free(pStr);//调用的时候申请的，需要回收
	}
}

//发送指令给主机
void SendCmdToHost(u32 FlyAddr,void *pStr)
{
	if(FlyAddr)
	{
		WSTR_PACKET *pStrPkt=0;
		u16 Bytes=0;
		u16 PktSn;		

		if(pStr!=NULL) Bytes=strlen(pStr)+1;
		pStrPkt=Q_Malloc(16+Bytes);

		pStrPkt->Type=WPT_CMD;
		pStrPkt->Res=WPR_MAIN;
		pStrPkt->PktCnt=GetWNetPktCnt();
		
		pStrPkt->Num=0;
		pStrPkt->DataLen=Bytes;

		MemCpy(pStrPkt->Data,pStr,Bytes);

		PktSn=WNetSendPkt(FlyAddr(FlyAddr),(void *)pStrPkt,(pStdFunc)SendStrToHost_CB);
		if(PktSn==0)
		{
			Debug("Send Str State Faild!\n\r");
		}
		
		Q_Free(pStrPkt);
	}
}

//发出时机
void SendOppoToHost(int OppoID,void *p)
{
	if(RFS_DB()->RFSI_FLY_ADDR)
	{
		WCON_PACKET *pConPkt=(void *)Q_Malloc(64);

		pConPkt->Type=WPT_CONFIG;
		pConPkt->Res=WPR_MAIN;
		pConPkt->PktCnt=GetWNetPktCnt();

		pConPkt->Num=pConPkt->DataLen=0;
		WPktSetAttrib(pConPkt,WAA_SET,WDA_SEM,OppoID);

		WnetSendToFly(RFS_DB()->RFSI_FLY_ADDR,(void *)pConPkt);

		Q_Free(pConPkt);
	}
}

//主动与主机解绑
void UnbindToHost(bool NeedSendMainPkt,void *a)
{
	u32 FlyAddr=RFS_DB()->RFSI_FLY_ADDR;
	u8 i;
	
	Debug("Unbind to host!\n\r");

#if PRODUCT_IS_WAVER
	LedRev(IOOUT_LED1);//yellow
	LedRev(IOOUT_LED2);//blue
#elif PRODUCT_IS_JUMPER
	LedRev(IOOUT_LED_FIRE);
	LedRev(IOOUT_LED_STUDY);
#endif

	if(FlyAddr)
	{
		u8 VarNum=RFS_GetConfig()->VarNum;
		
		if(NeedSendMainPkt)
		{
			WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
			pConPkt->Type=WPT_UNBIND;
			pConPkt->Res=WPR_MAIN;
			pConPkt->PktCnt=GetWNetPktCnt();
			WNetSendPkt(FlyAddr,(void *)pConPkt,NULL);
			Q_Free(pConPkt);
		}
		
		RFS_DB()->RFSI_FLY_ADDR=0;//清空主机地址
		
		for(i=0;i<SELF_VAR_MAX;i++)//恢复初值
		{			
			RFS_SetVarValSave(i,RFS_GetConfig()->VarInitVal[i]);
		}
		
		AddNextVoidFunc(FALSE,RFS_BurnToRom);
	}	
}

#endif

