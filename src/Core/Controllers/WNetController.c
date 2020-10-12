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

本文件定义的控制器主要用来处理WNET的交互
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"
#include "LedsMode.h"
#include "WDevFunc.h"


#define PAIR_LOOP_MS 2000
static void PairLoopTimer(int cnt,void *p) //保持搜索包发送
{
	if(cnt==0) //超过搜寻时间，退出
	{
		SetWorkMode(MNM_IDLE);
		LedIndicate(LMO_IDLE);
		return;
	}
	else if(cnt>=0xffff){}
	else {cnt--;}
	
	if(WorkMode()==MNM_START_PAIR)//循环发搜索包
	{
		WDevSearchDev();
		AddOnceMsFunc(PAIR_LOOP_MS,PairLoopTimer,cnt,NULL);
	}
	else if(WorkMode()==MNM_SYNC)
	{
		AddOnceMsFunc(PAIR_LOOP_MS,PairLoopTimer,cnt,NULL);
	}
	else if(WorkMode()==MNM_PRE_WORK)
	{
		WDevSyncToDev(RFS_DB()->RFSI_BROTHER_ADDR);//发出同步包给兄弟
		AddOnceMsFunc(PAIR_LOOP_MS,PairLoopTimer,cnt,NULL);
	}
	else //模式改变，停止循环
	{

	}
}

//控制器事件处理函数
static EVENT_HANDLER_RESUTL Init_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	IOIN_OpenExti(IOIN_KEY1);
	IOIN_OpenExti(IOIN_KEY2);

	gWnetTimer=AddSysTimer(STT_MANUAL,WNET_TIMER_POLL_MS,EBF_NULL,WNetTimerPoll,FALSE);

	WRF_DRV.pWRF_Init(WNetMyAddr(),RFS_DB()->RFSI_RSSI_THRD);//硬件的初始化
	WNetInit();//wnet的初始化

	if(RFS_DB()->RFSI_BROTHER_ADDR)
	{
		SetWorkMode(MNM_PRE_WORK);
		LedIndicate(LMO_WAIT_BROTHER);
		AddOnceMsFunc(Rand(0x3ff),PairLoopTimer,60000/PAIR_LOOP_MS,NULL);//开启一分钟的搜寻时间
	}
	else
	{
		SetWorkMode(MNM_IDLE);
	}

	return EFR_OK;
}

//系统while(1)空等时循环调用的事件
static EVENT_HANDLER_RESUTL Idle_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	if(IOIN_ReadIoStatus(IOIN_WRF_DRV_INT)==WRF_DRV_INT_LEVEL)
	{
		WRF_DRV.pWRF_ISR();
	}
	
	return EFR_OK;
}

//裸数据进入
static EVENT_HANDLER_RESUTL WNet_PacketIn_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	WNetPacketInHandler();
	return EFR_OK;
}

//应用包进入
static EVENT_HANDLER_RESUTL WNet_DataIn_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	u8 *pData;
	u16 DataLen;
	u32 SrcAddr,DstAddr;

	pData=GetWNetRecvBuf(&DataLen,&SrcAddr,&DstAddr);
	if(pData!=NULL)
	{		
		if(DataLen) WNetDataInHandler(SrcAddr,DstAddr,pData,DataLen);
		WNetAppCopyRecvDataFinish();//处理完毕，回收内存					
	}

	return EFR_OK;
}

//按键被按下并松开后的事件
static EVENT_HANDLER_RESUTL KeyHandler_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	u16 KeyIo=a&0xffff;
	u16 Ms=a>>16;

	//Debug("New Key%u %umS\n\r",KeyIo,Ms);

	if(KeyIo==0)
	{
		if(Ms>=5000) //解绑兄弟
		{
			if(RFS_DB()->RFSI_BROTHER_ADDR)
			{
				SetWorkMode(MNM_IDLE);
				LedIndicate(LMO_IDLE);
				WDevUnbindDev(RFS_DB()->RFSI_BROTHER_ADDR);
				RFS_DB()->RFSI_BROTHER_ADDR=0;
				AddNextVoidFunc(FALSE,RFS_BurnToRom);
			}
			else //还没兄弟
			{
				//LedIndicate(LMO_ERR);//错误灯
			}
		}
		else if(Ms>=2000)//绑定兄弟
		{
			if(RFS_DB()->RFSI_BROTHER_ADDR)//已经有兄弟了
			{
				//LedIndicate(LMO_ERR);//错误灯
			}
			else if(WorkMode()==MNM_IDLE)
			{
				//进入绑定逻辑
				SetWorkMode(MNM_START_PAIR);
				LedIndicate(LMO_WAIT_PAIR);//准备配对
				AddOnceMsFunc(PAIR_LOOP_MS,PairLoopTimer,60000/PAIR_LOOP_MS,NULL);//开启一分钟的搜寻时间
				WDevSearchDev();
			}
		}
		else //小于2s的按下
		{
			if(WorkMode()==MNM_START_PAIR) //当前是配对状态，则取消配对
			{
				SetWorkMode(MNM_IDLE);
				LedIndicate(LMO_CANCLE);
			}
			else if(WorkMode()==MNM_WORK)
			{
				WDevSyncToDev(RFS_DB()->RFSI_BROTHER_ADDR);
			}
		}
	}

	return EFR_OK;
}

//控制器定义和注册
static const EVENT_FUNC_ITEM gWNetController[]={
{EBF_INIT,Init_EF},
{EBF_IDLE,Idle_EF},
{EBF_WNET_PACKET_IN,WNet_PacketIn_EF},
{EBF_WNET_DATA_IN,WNet_DataIn_EF},
{EBF_KEY,KeyHandler_EF},

{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void WNetControllerReg(void)
{
	ControllerRegister(gWNetController,"WNet Controller");
}

