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

本文件定义的控制器主要用来做透传通信的逻辑处理
*/
//------------------------------------------------------------------//
#include "SysDefines.h"
#include "Product.h"
#include "WDevFunc.h"

//控制器事件处理函数
static EVENT_HANDLER_RESUTL RsComInit_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	COM2_Init(RFS_DB()->Com2Baud);
	Com2_DmaConfig();
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);

	return EFR_OK;
}

//发送失败，改变模式
static void PassToDev_CB(WNET_BASE_RES TransRes, WNET_INFO_BLOCK *pBlock)
{
	if(TransRes!=WBR_OK)
	{
		Debug("Trans to 0x%x Failed[%u]!\n\r",pBlock->DstAddr,TransRes);

		SetWorkMode(MNM_PRE_WORK);
		LedIndicate(LMO_WAIT_BROTHER);
		WDevSyncToDev(RFS_DB()->RFSI_BROTHER_ADDR);//发出同步包给兄弟
	}
}

//收到rs串口数据
static EVENT_HANDLER_RESUTL RsComHandler_EF(EVENT_BIT_FLAG Event,int DateLen,u8 *pData)
{
	//Debug("RS COM:\n\r");
	//DisplayBuf(pData,DateLen,16);

	if(DateLen==0 || pData==NULL) return EFR_OK;
	if(WorkMode()!=MNM_WORK) return EFR_OK;

	LedSet(IOOUT_LED2,0);
	WDevPassData(RFS_DB()->RFSI_BROTHER_ADDR,DateLen,pData,PassToDev_CB);
	LedSet(IOOUT_LED2,1);

	if(IsHeapRam(pData)) Q_Free(pData);

	return EFR_OK;
}

//按键被按下并松开后的事件
static EVENT_HANDLER_RESUTL KeyHandler_EF(EVENT_BIT_FLAG Event,int a,void *p)
{
	u16 KeyIo=a&0xffff;
	u16 Ms=a>>16;

	if(KeyIo==1)
	{
		
	}

	return EFR_OK;
}


//控制器定义和注册
static const EVENT_FUNC_ITEM gRsComController[]={
{EBF_INIT,RsComInit_EF},
{EBF_RS_COM_CMD,RsComHandler_EF},
{EBF_KEY,KeyHandler_EF},

{EBF_NULL,NULL}//控制器定义一律以此结尾
};

void RsComControllerReg(void)
{
	ControllerRegister(gRsComController,"My New Controller");
}

