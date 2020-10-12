#include "SysDefines.h"
#include "WDevFunc.h"

//开始配对
void WDevPairDev(WNET_ADDR WAddr,pStdFunc pSentCallBack)
{
	WCON_PACKET *pConPkt=NULL;

	if(WAddr==0) return;

	pConPkt=Q_Malloc(64);
	pConPkt->Type=WPT_ADD_CLIENT;
	pConPkt->Res=WPR_MAIN;
	pConPkt->PktCnt=GetWNetPktCnt();
	pConPkt->Num=pConPkt->DataLen=0;
	WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
		
	WNetSendPkt(WAddr,(void *)pConPkt,pSentCallBack);
	Q_Free(pConPkt);
}

static void SyncToDev_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(WorkMode()==MNM_PRE_WORK) 
	{
		if(TransRes==WBR_OK)
		{
			PairSyncFinishHook(TRUE);
		}
		else
		{
			PairSyncFinishHook(FALSE);
		}
	}
	else if(WorkMode()==MNM_WORK)//4测试掉线状态
	{
		if(TransRes!=WBR_OK)
		{
			Debug("Sync to 0x%x Failed[%u]!\n\r",pBlock->DstAddr,TransRes);

			SetWorkMode(MNM_PRE_WORK);
			LedIndicate(LMO_WAIT_BROTHER);
		}
	}
	else //配对状态
	{

	}
}

//发出同步包
void WDevSyncToDev(WNET_ADDR WAddr)
{
	WCON_PACKET *pConPkt=NULL;

	if(GroupAddr(WAddr)==0 || ClientAddr(WAddr)==0) return;
	if(GroupAddr(WAddr)==0xffff || ClientAddr(WAddr)==0xffff) return;

	Debug("Sync WDev 0x%x\n\r",WAddr);
	
	pConPkt=(void *)Q_Malloc(64);
	
	pConPkt->Type=WPT_SYNC;
	pConPkt->Res=WPR_MAIN;
	pConPkt->PktCnt=GetWNetPktCnt();
	pConPkt->Num=pConPkt->DataLen=0;
	WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
	
	WNetSendPkt(WAddr,(void *)pConPkt,(pStdFunc)SyncToDev_CB);

	Q_Free(pConPkt);
}

//主动与主机解绑
void WDevUnbindDev(WNET_ADDR WAddr)
{
	Debug("Unbind Dev 0x%x\n\r",WAddr);

	if(WAddr)
	{
		WCON_PACKET *pConPkt=(void *)Q_Malloc(64);
		pConPkt->Type=WPT_UNBIND;
		pConPkt->Res=WPR_MAIN;
		pConPkt->PktCnt=GetWNetPktCnt();
		pConPkt->Num=pConPkt->DataLen=0;
		WPktSetAttrib(pConPkt,WAA_SET,WDA_VER,WNET_VERSION);
		
		WNetSendPkt(WAddr,(void *)pConPkt,NULL);
		Q_Free(pConPkt);
	}	
}

//透传数据
void WDevPassData(WNET_ADDR WAddr,u16 Len,void *pData,pStdFunc pCb)
{
	WSTR_PACKET *pDataPkt=NULL;

	if(GroupAddr(WAddr)==0 || ClientAddr(WAddr)==0) return;
	if(GroupAddr(WAddr)==0xffff || ClientAddr(WAddr)==0xffff) return;
	if(Len==0 || pData==NULL) return;
	
	//Debug("Trans WDev 0x%x\n\r",WAddr);
	
	pDataPkt=(void *)Q_Malloc(16+Len);
	
	pDataPkt->Type=WPT_PASS_THROUGH;
	pDataPkt->Res=WPR_MAIN;
	pDataPkt->PktCnt=GetWNetPktCnt();
	pDataPkt->DataLen=Len;
	pDataPkt->Num=0;
	MemCpy(pDataPkt->Data,pData,Len);
	
	WNetSendPkt(WAddr,(void *)pDataPkt,(pStdFunc)pCb);
	Q_Free(pDataPkt);
}

