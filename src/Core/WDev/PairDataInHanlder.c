#include "SysDefines.h"
#include "WDevFunc.h"

//配对流程，发起端a，被绑定端b
//a b先后进入配对模式
//a[start] 发出广播包add client  >> b[start]
//a[start] << b[sync] 发出同步包sync
//a[sync] 发出单播包add client >> b[sync]
//b固化对方地址
//a收到单播包的底层ack后 固化对方地址
//绑定完成
//[sync]状态下，即不会发送广播包 也不会接收其他设备的包


void PairSyncFinishHook(bool IsSucess)
{
	if(IsSucess) 
	{
		Debug("Brother Sync Sucess!\n\r");
		SetWorkMode(MNM_WORK);
		LedIndicate(LMO_WORK);
	}
	else
	{

	}
}

//配对包发送成功
static void PairToDev_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(TransRes==WBR_OK)
	{
		if(ClientAddr(pBlock->DstAddr)!=0xffff) //单播代表确定绑定关系
		{
			Debug("Pair To Dev 0x%x\n\r",pBlock->DstAddr);
			RFS_DB()->RFSI_BROTHER_ADDR=pBlock->DstAddr;
			AddNextVoidFunc(FALSE,RFS_BurnToRom);
			SetWorkMode(MNM_WORK);
			LedIndicate(LMO_WORK);
		}
	}
	else
	{
		Debug("Pair To Dev 0x%x Failed!\n\r",pBlock->DstAddr);
		SetWorkMode(MNM_START_PAIR);
	}
}

//透传包进入
static void PairPassThrough(WSTR_PACKET *pDataPkt,u32 SrcAddr,u32 DstAddr)
{
	if(pDataPkt->Type!=WPT_PASS_THROUGH) return;
	if(SrcAddr!=RFS_DB()->RFSI_BROTHER_ADDR) return;

	Com2_Send_Dma(pDataPkt->Data,pDataPkt->DataLen);
}

//兄弟包进入
void PairPacketIn(WCON_PACKET *pConPkt,u32 SrcAddr,u32 DstAddr)
{
	switch(pConPkt->Type)
	{
		case WPT_PASS_THROUGH:
			if(ClientAddr(DstAddr) != WNET_BROADCAST_ADDR)//非广播
			{
				PairPassThrough((void *)pConPkt,SrcAddr,DstAddr);
			}
			break;
		case WPT_ADD_CLIENT:
			if(GroupAddr(SrcAddr)==GroupAddr(WNetMyAddr()))//同组设备
			{
				if(ClientAddr(DstAddr) == WNET_BROADCAST_ADDR)//广播
				{
					if(WorkMode()!=MNM_START_PAIR) break;
					SetWorkMode(MNM_SYNC);
					WDevSyncToDev(SrcAddr);//配对试探
					RevertWorkMode(1000);
				}
				else//单播
				{
					if(WorkMode()!=MNM_SYNC) break;
					if(RFS_DB()->RFSI_BROTHER_ADDR==0)
					{
						RFS_DB()->RFSI_BROTHER_ADDR=SrcAddr;
						AddNextVoidFunc(FALSE,RFS_BurnToRom);
						SetWorkMode(MNM_WORK);
						LedIndicate(LMO_WORK);
					}
				}
			}
			break;
		case WPT_SYNC:
			if(ClientAddr(DstAddr) == WNET_BROADCAST_ADDR) break;//不允许广播
			if(WorkMode()==MNM_START_PAIR)
			{
				if(GroupAddr(SrcAddr)==GroupAddr(WNetMyAddr()))//同组设备
				{
					SetWorkMode(MNM_SYNC);
					WDevPairDev(SrcAddr,PairToDev_CB);//发送单播配对包，并固化地址
					RevertWorkMode(500);
				}
			}
			else if(WorkMode()==MNM_SYNC) //已经与其他人预配对了，不能理会对方的同步包
			{

			}
			else 
			{
				if(SrcAddr == RFS_DB()->RFSI_BROTHER_ADDR) 
				{
					if(WorkMode()==MNM_WORK) 
					{
						Debug("Brother Key Come on\n\r");
						LedIndicate(LMO_KEY_INDICATE);//响应对方的按键测试
					}
					else if(WorkMode()==MNM_PRE_WORK) 
					{
						PairSyncFinishHook(TRUE);//上线成功
					}
				}
				else //非兄弟发来的包
				{
					WDevUnbindDev(SrcAddr);
				}
			}
			break;
		case WPT_UNBIND:
			if(ClientAddr(DstAddr) == WNET_BROADCAST_ADDR) break;//不允许广播
			if(SrcAddr != RFS_DB()->RFSI_BROTHER_ADDR) break;

			Debug("Unbind By 0x%x\n\r",SrcAddr);
			SetWorkMode(MNM_IDLE);
			LedIndicate(LMO_IDLE);
			RFS_DB()->RFSI_BROTHER_ADDR=0;
			AddNextVoidFunc(FALSE,RFS_BurnToRom);
			break;
	}
}

