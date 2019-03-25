//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了ir解析库，可被开发者用于其他红外控制项目，减少代码开发量
*/
//------------------------------------------------------------------//

#include "Drivers.h"
#include "IrOut.h"
#include "Q_Heap.h"

#define DECODE_38K_MYSELF 0//自己解码38kHz的脉冲
static u32 gLast13usPluseTimeCnt=0;//38k脉冲最后一个脉冲点的时间记数

//硬件参数配置
#define IrTimer_ISR TIM2_IRQHandler
#define IrTimerSet Tim2_Set
#define IrlTimerID TIM2
#define WorkLed(x) do{}while(0);
#define StudyLed(x) do{LedSet(IOOUT_LED1,(x)?1:0);}while(0);
#define FireLed(x) do{LedSet(IOOUT_LED2,(x)?1:0);}while(0);

//软件参数设定
#define IR_RECV_PLUSE_MIN_LIMIT 8//最小解析脉冲个数
#define IR_RECV_MAX_WAVE_SPACE_MS 50 //波段最大时间间隔，单位ms
#define IR_RECV_MAX_TIMING_US 50000 //接收时定时器最大定时
#define IR_CHECK_TIME_UNIT_US 1 //定时器基准
#define IR_PULSE_RECORD_NUM 1024//记录脉冲个数
#define IR_PULSE_BUF_LEN (sizeof(u16)*IR_PULSE_RECORD_NUM)

#define OS_EnterCritical()
#define OS_ExitCritical()
#define OS_TaskYield()

#if 1
/********************************* IR Recv *******************************************/
static volatile bool gIsRecvState=FALSE;//是否在接收状态
static volatile u16 *gpIrPluseTimes=NULL;//记录每个电平持续时间
static volatile u16 gIrPluseNum=0;//当前记录的电平个数
static IR_RECORD gIrRecord;

void DisplayIrRecord(const IR_RECORD *pIrRcd)
{
	u16 i;
	u16 n;

	if(pIrRcd==NULL) return;

	Debug("IR Idx: ");
	for(i=0;i<MAX_IR_IDX_NUM;i++) 
	{
		if(pIrRcd->IdxTimes[i]==0) break;
		if(pIrRcd->IdxTimes[i]&0x01) //高电平
		{
			Debug("-");
		}
		else
		{
			Debug("_");
		}
		
		Debug("%u ",pIrRcd->IdxTimes[i]);
	}
	Debug("\n\r");

	Debug("IR List[%u]:\n\r",pIrRcd->PulseNum);
	if(pIrRcd->PulseNum>MAX_IR_PLUSE_NUM) return;

	for(i=0;i<pIrRcd->PulseNum;i++)
	{
		if(i&0x01) //高电平
		{
			n=pIrRcd->Pluse[i>>1]>>4;Debug("-");
		}
		else
		{
			n=pIrRcd->Pluse[i>>1]&0x0f;Debug("_");
		}

		Debug("%u",pIrRcd->IdxTimes[n]);
		//if(i%16==(16-1)) Debug("\n\r");
	}	

	Debug("\n\r\n\r");
}

//开始接收红外信号
void StartRecvIr(void)
{
	gIsRecvState=TRUE;
	MemSet(&gIrRecord,0,sizeof(gIrRecord));
	if(gpIrPluseTimes==NULL)
	{
		gpIrPluseTimes=Q_Malloc(IR_PULSE_BUF_LEN);
		//MemSet((void *)gpIrPluseTimes,0,IR_PULSE_BUF_LEN);
	}
	gIrPluseNum=0;
	WorkLed(1);
	StudyLed(0);
	IOIN_OpenExti(IOIN_IR_IN);
}

//关闭接收红外信号
void StopRecvIr(void)
{
	IOIN_CloseExti(IOIN_IR_IN);
	WorkLed(0);
	StudyLed(0);
	gIsRecvState=FALSE;
	gIrPluseNum=0;
	if(gpIrPluseTimes)
	{
		Q_Free(gpIrPluseTimes);
		gpIrPluseTimes=NULL;
	}
}

//获取捕获的红外信号，只能取一次
bool CaptureRecvIr(IR_RECORD *pIr)
{
	MemCpy(pIr,&gIrRecord,sizeof(IR_RECORD));
	MemSet(&gIrRecord,0,sizeof(IR_RECORD));

	if(pIr->Pluse==0 || pIr->Type!=SIT_IR) return FALSE;

	return TRUE;
}

//仅供内部调用
IR_RECORD *SetCaptureBuf(IR_RECORD *pIr)
{
	MemCpy(&gIrRecord,pIr,sizeof(IR_RECORD));

	return &gIrRecord;
}

//将脉冲同索引数组进行比较
static u8 ComparePluseToIdx(u16 Num,u16 *pItem)
{
	u16 i;

	for(i=0;i<MAX_IR_IDX_NUM;i++)
	{
		if(pItem[i]==0) return 0;
		if(((pItem[i]^Num)&0x01)==0)//同是高电平或者同是低电平
		{
			if(FuzzyEqual(Num,pItem[i],IR_FUZZY_PARAM)==TRUE) return i+1;
		}
	}

	return 0;
}

//分析波形到记录体
static bool RecordIrPluse(const u16 *pPluseMs,u16 Num,IR_RECORD *pIrRcd)
{
	u16 Res,n;
	u16 IdxCnt=0;

	if(pIrRcd==NULL) return FALSE;
	MemSet(pIrRcd,0,sizeof(IR_RECORD));

	//Debug("Pluse[%u]\n\r",Num);
	//DisplayBufU16_Dec(pPluseMs,Num,16);

	if(Num<IR_RECV_PLUSE_MIN_LIMIT) return FALSE;
	if(Num>MAX_IR_PLUSE_NUM) return FALSE;
	if(Num&0x01==0) return FALSE;//记录个数必须为奇数，有始有终

	for(n=0;n<Num;n++)
	{
		if(pPluseMs[n]==0) break;
		Res=ComparePluseToIdx(pPluseMs[n],pIrRcd->IdxTimes);
		if(Res==0)//未比较出容差值，建立新索引
		{
			if(IdxCnt>=MAX_IR_IDX_NUM) 
			{
				Debug("Res Need too big!\n\r");
				return FALSE;//解码失败，索引不够用
			}
			pIrRcd->IdxTimes[IdxCnt]=pPluseMs[n];
			IdxCnt++;
			Res=IdxCnt;
		}
		else if(Res>MAX_IR_IDX_NUM)
		{
			Debug("Idx error!\n\r");
			return FALSE;
		}

		if(n&0x01) //高电平
		{
			pIrRcd->Pluse[n>>1]|=((Res-1)<<4);//高4位
		}
		else
		{
			pIrRcd->Pluse[n>>1]=(Res-1)&0x0f;//低4位
		}
	}

	pIrRcd->PulseNum=n;	
	pIrRcd->Type=SIT_IR;
	pIrRcd->SendCnt=0;

	//DisplayIrRecord(pIrRcd);

	return TRUE;
}

//将脉冲数据记录到数组
void StoragePluseTime2Array(bool IrFire,u16 TimeCnt)
{
	if(gIrPluseNum == 0) StudyLed(1);

	if(gIrPluseNum < IR_PULSE_RECORD_NUM)
	{
		gpIrPluseTimes[gIrPluseNum]=TimeCnt;//记录电平时间
		if(IrFire) gpIrPluseTimes[gIrPluseNum]|=1;//最低位为1，表示有ir信号
		else gpIrPluseTimes[gIrPluseNum]&=~1;//最低位为0，表示ir信号静默
		gIrPluseNum++;
	}
}

//用户处理程序，IR输入管脚有电平变化时调用，在中断中进行
//ir接收脚，常态是1，接收到信号变0，所以中断结束时，io状态和接收时刚好相反
void IrPulseIn_ISR(void)
{
	u32 Count=TIM_GetCounter(IrlTimerID);
	bool IrFireEnd=IOIN_ReadIoStatus(IOIN_IR_IN)?TRUE:FALSE;
	Debug("%c",IrFireEnd?'-':'-');
	if(Count==0) 
	{
#if DECODE_38K_MYSELF //自己解码38k
		gLast13usPluseTimeCnt=0;
#endif
		gIrPluseNum=0;
		IrTimerSet(IR_RECV_MAX_TIMING_US,IR_CHECK_TIME_UNIT_US,FALSE);//	开启每1us增加一个计数的定时器，最高50ms	
		return;
	}
	
#if DECODE_38K_MYSELF //自己解码38k
	if(Count-gLast13usPluseTimeCnt < 15)//38k的一个脉冲13us
	{
		gLast13usPluseTimeCnt=Count;
		return;
	}
	else //
	{
		if(IrFireEnd)
		{
			return;//不会有超过13us的fire脉冲，所以直接返回
		}
		else
		{
			StoragePluseTime2Array(TRUE,gLast13usPluseTimeCnt);//记录前面的fire总长
			StoragePluseTime2Array(FALSE,Count);//记录信号静默总长
			gLast13usPluseTimeCnt=0;
		}
	}	
#else
	StoragePluseTime2Array(IrFireEnd,Count);
#endif	

	IrTimerSet(IR_RECV_MAX_TIMING_US,IR_CHECK_TIME_UNIT_US,FALSE);//	开启每1us增加一个计数的定时器，最高50ms	
}

//ir脉冲处理完成后的处理，在中断中处理
//处理完毕后，会得到正确的gpIrRecvDatas输出
static void IrRecvEnd_ISR(void)
{
	bool Res;	
	bool NeedYield;

	StudyLed(0);

#if DECODE_38K_MYSELF //自己解码38k
	if(gLast13usPluseTimeCnt)	//保存最后一段信号脉冲的时间
	{
		StoragePluseTime2Array(TRUE,gLast13usPluseTimeCnt);
	}
#endif
	
	OS_EnterCritical();
	Res=RecordIrPluse((void *)gpIrPluseTimes,gIrPluseNum,&gIrRecord);
	gIrPluseNum=0;
	MemSet((void *)gpIrPluseTimes,0,IR_PULSE_BUF_LEN);
	OS_ExitCritical();
	
	if(Res)//解析到有效信号
	{
		//SysEventSend(SEN_IR_CAPTURE,0,&gIrRecord,&NeedYield);
	}
	
	DisplayIrRecord(&gIrRecord);

	if(NeedYield)
	{
		//Debug("NeedYeild %u\n\r",OS_GetNowMs());
		OS_TaskYield();
	}	
}
#endif

#if 1
/********************************* IR Send *******************************************/
static volatile u16 gIrSendBitNum=0;//已经发送的脉冲个数
static volatile u16 gIrSendPluseNum=0;//当前记录的电平个数
static volatile u16 *gpIrSendPluseTimes;//记录每个电平持续时间
static volatile bool gSendingFlag=FALSE;//正在发送标识

//将数据恢复成电平时间
//返回脉冲个数
static u16 RestorePluseTime(u16 *pPluseTimes,const IR_RECORD *pIrRcd)
{
	u16 i,n;
	
	if(pPluseTimes==NULL) return 0;
	if(pIrRcd==NULL) return 0;
	if(pIrRcd->PulseNum>MAX_IR_PLUSE_NUM) return 0;

	for(i=0;i<pIrRcd->PulseNum;i++)
	{
		if(i&0x01) //高电平
		{
			n=pIrRcd->Pluse[i>>1]>>4;
		}
		else
		{
			n=pIrRcd->Pluse[i>>1]&0x0f;
		}

		pPluseTimes[i]=pIrRcd->IdxTimes[n];
	}
	pPluseTimes[i]=0;

	return pIrRcd->PulseNum;
}

//脉冲完成后的回调
static void IrSent_CB(void)
{
	if(gpIrSendPluseTimes)
	{
		Q_Free(gpIrSendPluseTimes);
		gpIrSendPluseTimes=NULL;
	}
	
	gSendingFlag=FALSE;
	FireLed(0);
}

//pIrRecord，存储红外信息的数组，为NULL时发送刚刚收到的ir
//SendQueueLen，数组成员个数
//不可在中断中调用
void StartSendIr(const IR_RECORD *pIrRcd)
{	
	StopRecvIr();

	if(gSendingFlag==TRUE)//发射进行中
	{ 
		Debug("Old UnFinish\n\r");
		return;//旧的没发送完，退出
	}

	gSendingFlag=TRUE;

	if(pIrRcd == NULL) //调试用
	{
		pIrRcd=&gIrRecord;
	}

	//DisplayIrRecord(pIrRcd);

	if(gpIrSendPluseTimes==NULL) gpIrSendPluseTimes=Q_Malloc(IR_PULSE_BUF_LEN);
	//MemSet((void *)gpIrSendPluseTimes,0,sizeof(gpIrSendPluseTimes));
	gIrSendPluseNum=RestorePluseTime((void *)gpIrSendPluseTimes,pIrRcd);
	if(0)//for debug
	{
		u16 i;
		Debug("Send[%u]:",gIrSendPluseNum);
		for(i=0;i<gIrSendPluseNum;i++)Debug("%c%u",(i&0x01)?'-':'_',gpIrSendPluseTimes[i]);
	}

	if(gIrSendPluseNum == 0)//无数据
	{
		gSendingFlag=FALSE;
		return;
	}

	gIrSendBitNum=0;
	SetSendIrData(1);//开始发送波形
	//Debug("_%u",gpIrSendPluseTimes[0]);
	IrTimerSet(gpIrSendPluseTimes[0],IR_CHECK_TIME_UNIT_US,FALSE);//	开启每1us增加一个计数的定时器
	FireLed(1);
	
	//while(gSendingFlag==TRUE);//等待发送结束
}

//发送脉冲的定时器中断
static void IrSend_ISR(void)
{
	if(gIrSendBitNum < gIrSendPluseNum)
	{
		gIrSendBitNum++;
		SetSendIrData(gIrSendBitNum&0x01);		
		//if(gIrSendBitNum&0x01) Debug("-%u",gpIrSendPluseTimes[gIrSendBitNum]);
		//else Debug("_%u",gpIrSendPluseTimes[gIrSendBitNum]);
		if(gpIrSendPluseTimes[gIrSendBitNum]==0)//发到末尾了，脉冲全部发完，执行wave space延时
		{
			gIrSendBitNum = gIrSendPluseNum;
			SetSendIrData(0);//停止发送波形
			IrTimerSet(0,IR_CHECK_TIME_UNIT_US,FALSE);
			IrSent_CB();
		}
		else
		{
			IrTimerSet(gpIrSendPluseTimes[gIrSendBitNum],IR_CHECK_TIME_UNIT_US,FALSE);
		}
	}
	else if(gIrSendBitNum == gIrSendPluseNum)
	{
		SetSendIrData(0);//停止发送波形
		IrTimerSet(0,IR_CHECK_TIME_UNIT_US,FALSE);
		IrSent_CB();
	}
	else 
	{
		Debug("IrSendBit %d,IrSendNum %d\n\r",gIrSendBitNum,gIrSendPluseNum);
		while(1);
	}
}
#endif

//定时器中断函数
void IrTimer_ISR(void)
{
	if(TIM_GetITStatus(IrlTimerID, TIM_IT_Update) != RESET)//如果定时器到期，说明电平持续时间长于65525us，说明采集结束
	{
		TIM_ClearITPendingBit(IrlTimerID, TIM_IT_Update);		

		if(gIsRecvState)//处理接收
		{
			IrRecvEnd_ISR();
		}
		else//处理发送
		{
			IrSend_ISR();
		}		
	}
}

