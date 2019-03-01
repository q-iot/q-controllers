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

本文件是项目代码的c启动文件
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"
#include "Controllers.h"

u32 gWdgTimer=0;//系统定时器句柄
u32 gTimingFuncTimer=0;//系统定时器句柄

//硬件初始化
void HardwareInit(void)
{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//使用swd调试
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1,ENABLE);//TIM1_CH1N设置到PA7

	IoDefinesInit();//io口初始化

	SpiFlsInit();

	Tim2_Init();
	Tim4_Init();
}

void EventStateHandler(void)
{
	EVENT_BIT_FLAG Event=EBF_NULL;
	s32 S32Param=0;
	void *pParam=NULL;
	
	CleanAllEvent();//进循环之前清除所有事件标志位

	//控制器init
	ControllerEvtPost(EBF_INIT,0,NULL);
	
	while(1)
	{
		pParam=WaitEvent(&Event,&S32Param);

		if(Event>=EBF_MAX) continue;//过滤
		
		//系统快速事件处理
 		if(Event==EBF_NEXT_QUICK_FUNC) NextFuncExcute(TRUE);

		//控制器回调处理
		switch(Event)//轮询检查标志位，当有事件时，立刻执行处理程序
		{
			case EBF_NEXT_QUICK_FUNC:
			case EBF_SEC_FUNC:
			case EBF_SYS_CMD:
			case EBF_NEXT_LOOP_FUNC:
				break;

			default:
				ControllerEvtPost(Event,S32Param,pParam);//事件分发给控制器
		}

		//系统内部事件处理
		switch(Event)//轮询检查标志位，当有事件时，立刻执行处理程序
		{
			case EBF_SEC_FUNC:
				SecFuncExpired();
				break;
			case EBF_Q_COM_CMD:
				if(IsHeapRam(pParam)) Q_Free(pParam);//释放用完的内存				
				break;
			case EBF_SYS_CMD:
				{
					extern bool SysCmdHandler(u16 Len,const char *pStr,char *pOutStream);
					SysCmdHandler(S32Param,pParam,NULL);
				}
				break;				
			case EBF_NEXT_LOOP_FUNC:
				NextFuncExcute(FALSE);
				break;
		}
	}
}

int main(void)
{	
	SysTick_Init();//节拍定义	
	SystemInit();//系统时钟初始化
	COM1_Init(115200);//调试串口
	COM3_Init(74880);//用户串口，接q-wifi

	DebugCol("\n\n\n\n\rQ-Controller");Debug(" ");
	DebugCol("%u\n\r",GetHwID(NULL));
	Debug("Release %u\n\r",RELEASE_DAY);

    RFS_Init();//数据存储初始化
	Adc1_Rand_Init(0);//adc pa0 pa1 pa2 pa3，每个bit代表一路

	NextLoopFuncInit();//sys time 会用到
	SysTimerInit();//必须放到比较靠前的位置，防止有些初始化程序要用到Timing功能
	gWdgTimer=AddSysTimer(STT_AUTO,WDG_CB_MS,EBF_NULL,IWDG_PeriodCB,TRUE);
	gTimingFuncTimer=AddSysTimer(STT_MANUAL,0,EBF_NULL,MsFuncExpired,FALSE);
	
	MsFuncInit();//必须放到比较靠前的位置，防止有些初始化程序要用到Timing功能
	SecFuncInit();//秒级定时器
	HardwareInit();//底层硬件初始化

	//指示灯
	LedSet(IOOUT_LED1,1);//yellow
	LedSet(IOOUT_LED2,1);//blue
	DelayMs(200);
	LedSet(IOOUT_LED1,0);//yellow
	LedSet(IOOUT_LED2,0);//blue
	DelayMs(200);

	//控制器逐个注册，靠前的事件触发时优先执行
	QComControllerReg();
	QWifiControllerReg();
	NewControllerReg();
	TestControllerReg();

#if 1//!ADMIN_DEBUG
	IWDG_Configuration();//开启看门狗
#endif

	//开中断
	IOIN_OpenExti(IOIN_PIO0);
	
	EventStateHandler();

	while(1);
}


/*******************END OF FILE****/

