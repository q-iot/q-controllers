//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了一套定时器机制，可被开发者用于其他项目，减少代码开发量
*/
//------------------------------------------------------------------//
#include "SysDefines.h"

#define T_Debug(x,y...)

#define TFUNC_RCD_NUM 32//总记录数

#if 1
static volatile u32 gTfThisTimCnt=0;//本次定时长度

//定时器单位Sec
volatile u32 gSecFuncRtc=0;//会被外部秒中断引用，递减
#define TfTimSet(Val) {gSecFuncRtc=Val;gTfThisTimCnt=Val;} //设定定时器
#define TfTimCancle() {gSecFuncRtc=0;}   //取消定时器
#define GetTfTimCount() (gTfThisTimCnt-gSecFuncRtc)			//获取定时器跑了多久

#endif

//以下内容不同参考时间的定时器函数都一样


typedef struct{
	u8 Flag;//本条被使用时，置idx+1
	u16 EntresCnt;//进入次数，第一次进入为1，以此类推
	u32 IntParam;//参数，给回调api用
	void *pParam;//参数，给回调api用
	u32 ThisRem;//本次剩余时间，计时器运作时，此值会变
	u32 TotalRem;//最终剩余时间.ThisRem+TotalRem=实际总到期时间，计时器运作时，此值会变
	u32 Interval;//间隔触发时间
	u32 TotalTim;// 初始化的总时间
	pStdFunc pStdCB;//回调函数
	pExpFunc pExpCB;//回调函数，第一个参数用来记录EntresCnt
}TFUNC_RCD;

typedef struct{
	bool NeedExcFlag;
	u16 EntresCnt;
	u32 IntParam;//参数，给回调api用
	void *pParam;//参数，给回调api用
	pStdFunc pStdCB;//回调函数
	pExpFunc pExpCB;//回调函数，第一个参数用来记录EntresCnt
}FUNC_CB_INFO;


static volatile TFUNC_RCD gTfRcd[TFUNC_RCD_NUM];//记录体
static volatile TFUNC_RCD *gpNearest=NULL;//最短定时的记录


#define PrintNowTime()  T_Debug("                                                 @ %d Sec\r",GetRtcCount()-SysVars()->SysStartRtc);

void SecFuncRcdDisp(void)
{
	u8 i;
	u8 n=0xff;

	
	if(gpNearest!=NULL)
	{
		n=((u32)gpNearest-(u32)gTfRcd)/sizeof(TFUNC_RCD);
	}

	Debug("\n\r---------------------------[TIMING FUNC LIST]---------------------------\n\r");
	for(i=0;i<TFUNC_RCD_NUM;i++)
	{
		if(gTfRcd[i].Flag == 0) continue;
		if(gTfRcd[i].pStdCB) Debug("AddSecFunc%x[%u]:%u ,",gTfRcd[i].pStdCB,i,gTfRcd[i].IntParam);
		else Debug("AddSecFunc%x[%u]:%u ,",gTfRcd[i].pExpCB,i,gTfRcd[i].IntParam);
		Debug("ThisRem:%d,TotalRem:%d,Interval:%d %s\n\r",
			gTfRcd[i].ThisRem,gTfRcd[i].TotalRem,gTfRcd[i].Interval,n==i?"<--":"");
	}
	Debug("------------------------------------------------------------------------\n\r\n\r");
}

void SecFuncInit(void)
{
	TfTimSet(0);
	MemSet((void *)gTfRcd,0,sizeof(gTfRcd));
}

//定时TimCnt到期后执行回调函数
//如果Interval不为零，则间隔Interval就执行回调
//回调函数可能在中断或主函数里执行
//回调函数中pRecord->TotalRem == 0表示定时器到期了
//如果不为零，则表示间隔回调
//TimCnt为0，会执行高优先级的next func
//如果Interval==0，pCallBack将视为pStdFunc，这点要注意
//当TimCnt==-1时，无限循环，Interval为循环时间
bool AddSecFunc(u32 TimCnt,u32 Interval,pExpFunc pCallBack,int IntParam,void *pParam)
{
	u8 i;
	TFUNC_RCD *pNewRecord;
	IntSaveInit();
	
	if(pCallBack==NULL) return FALSE;

	if(Interval)//TimCnt==0或者1，间隔就只是用来判断pcallback的类型，不会多次进入
	{
		if(TimCnt==0) return AddNextExpFunc(TRUE,pCallBack,1,IntParam,pParam);
		//else if(TimCnt==1) return AddNextExpFunc(FALSE,pCallBack,1,IntParam,pParam);
	}
	else
	{
		if(TimCnt==0) return AddNextStdFunc(TRUE,(pStdFunc)pCallBack,IntParam,pParam);
		//else if(TimCnt==1) return AddNextStdFunc(FALSE,(pStdFunc)pCallBack,IntParam,pParam);
	}

	PrintNowTime();
	T_Debug(" +++ SecFunc:%dSec[%d]\n\r",TimCnt,Interval);
	
	EnterCritical();
	for(i=0;i<TFUNC_RCD_NUM;i++)//找空位子
	{
		if(gTfRcd[i].Flag == 0) break;
	}
	if(i==TFUNC_RCD_NUM) 
	{
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			Debug("TF[%d]:%x\n\r",i,gTfRcd[i].pExpCB!=NULL?(u32)gTfRcd[i].pExpCB:(u32)gTfRcd[i].pStdCB);
		}
		Debug("SecFunc FULL!\r\n");
		LeaveCritical();
		RebootBoard();
		while(1);
	}
	
	//记录新的
	pNewRecord=(void *)&gTfRcd[i];
	pNewRecord->Flag=i+1;
	pNewRecord->TotalTim=TimCnt;
	if(Interval)//有间隔时间
	{
		pNewRecord->ThisRem=Interval;
		if(pNewRecord->TotalTim==(u32)-1) pNewRecord->TotalRem=(u32)-1;
		else pNewRecord->TotalRem=TimCnt-Interval;
		pNewRecord->pStdCB=NULL;
		pNewRecord->pExpCB=pCallBack;
	}
	else//一次性
	{
		pNewRecord->ThisRem=TimCnt;
		pNewRecord->TotalRem=0;
		pNewRecord->pStdCB=(pStdFunc)pCallBack;
		pNewRecord->pExpCB=NULL;
	}
	pNewRecord->Interval=Interval;
	pNewRecord->IntParam=IntParam;
	pNewRecord->pParam=pParam;
	pNewRecord->EntresCnt=0;

	//找到最小的定时
	if(gpNearest == NULL)//未存储，直接赋值
	{
		gpNearest=pNewRecord;

		//根据最小值定时
		TfTimSet(gpNearest->ThisRem);
	}
	else
	{
		u32 TimCnt=GetTfTimCount();
		if(TimCnt>gpNearest->ThisRem) TimCnt=gpNearest->ThisRem;//防止减出负数溢出
		T_Debug(" Rem(%d)-Cnt(%d) <? New(%d)\n\r",gpNearest->ThisRem,GetTfTimCount(),pNewRecord->ThisRem);
		if((gpNearest->ThisRem-TimCnt) <= pNewRecord->ThisRem)//最小值依然小
		{
			T_Debug(" XXX No Need Change TimeSet\n\r");
			pNewRecord->ThisRem+=TimCnt;//先加上逝去的时间，方便到期的时候扣掉
			LeaveCritical();
			//SecFuncRcdDisp();
			return TRUE;
		}
		else//新加进来的更急，改变指针
		{
			T_Debug(" XXX Change Nearest\n\r",gpNearest->ThisRem,TimCnt);

			//所有的未到期的时钟都要扣去逝去的时间
			{
				TFUNC_RCD *pRecord;
				for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)//检查哪些需要触发
				{
					if(pRecord->Flag == 0) continue;
					if(pRecord == pNewRecord) continue;
					pRecord->ThisRem-=TimCnt;
				}
			}			
			
			gpNearest=pNewRecord;

			//根据最小值定时
			TfTimSet(gpNearest->ThisRem);
		}
	}

	LeaveCritical();
	//SecFuncRcdDisp();
	return TRUE;
}

//添加不需要有时间间隔Interval的定时函数
//定时TimCnt到期后执行回调函数
//回调函数可能在中断或主函数里执行
//TimCnt为0，会执行高优先级的next func
bool AddOnceSecFunc(u32 TimCnt,pStdFunc pCallBack,int IntParam,void *pParam)
{
	return AddSecFunc(TimCnt,0,(pExpFunc)pCallBack,IntParam,pParam);
}

//目前在main中执行
void SecFuncExpired(void)
{
	u16 i;
	u32 MinVal=0xffffffff;
	TFUNC_RCD *pRecord;
	FUNC_CB_INFO *pCbInfo=Q_Malloc(sizeof(FUNC_CB_INFO)*TFUNC_RCD_NUM);
	u32 ThisTimCnt=GetTfTimCount();
	
	IntSaveInit();
	
	PrintNowTime();
	T_Debug(" --- NowConsume:%dSec\n\r",ThisTimCnt);

	EnterCritical();
	for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)//检查哪些需要触发
	{
		pCbInfo[i].NeedExcFlag=FALSE;
		if(pRecord->Flag == 0) continue;
		if(pRecord->ThisRem <= ThisTimCnt)//到期了的时钟
		{
			pRecord->ThisRem=0;
			pRecord->EntresCnt++;
			
			pCbInfo[i].NeedExcFlag=TRUE;//做标记				
			pCbInfo[i].EntresCnt=pRecord->EntresCnt;
			pCbInfo[i].IntParam=pRecord->IntParam;//拷贝信息给回调用
			pCbInfo[i].pParam=pRecord->pParam;
			pCbInfo[i].pStdCB=pRecord->pStdCB;
			pCbInfo[i].pExpCB=pRecord->pExpCB;

			if(pRecord->Interval == 0) //没有间隔
			{
				PrintNowTime();
				T_Debug(" --- SecFunc:%dSec\n\r",pRecord->TotalTim);
				MemSet((void *)pRecord,0,sizeof(TFUNC_RCD));	//销毁记录	
			}
			else //有间隔触发
			{
				//准备下次定时
				if(pRecord->TotalRem == 0)//走完间隙了
				{	
					PrintNowTime();
					T_Debug(" --- SecFunc:%dSec\n\r",pRecord->TotalTim);
					MemSet((void *)pRecord,0,sizeof(TFUNC_RCD));	//销毁记录
				}
				else if(pRecord->TotalRem < pRecord->Interval) //小于间隙
				{
					PrintNowTime();
					T_Debug(" ||| SecFunc:%dSec\n\r",pRecord->TotalTim);
					pRecord->ThisRem=pRecord->TotalRem;
					pRecord->TotalRem=0;
				}
				else //还有多次间隙
				{
					PrintNowTime();
					T_Debug(" ||| SecFunc:%dSec\n\r",pRecord->TotalTim);
					pRecord->ThisRem=pRecord->Interval;
					if(pRecord->TotalTim!=(u32)-1) pRecord->TotalRem-=pRecord->Interval;
				}	
			}
		}
		else
		{
			pRecord->ThisRem-=ThisTimCnt;
		}
	}

	gpNearest=NULL;
	for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)
	{
		if(pRecord->Flag == 0) continue;
		if(pRecord->ThisRem < MinVal) 
		{
			MinVal=pRecord->ThisRem;
			gpNearest=pRecord;
		}
	}
		
	if(gpNearest != NULL)//已经找到下一次的最小定时器
	{
		TfTimSet(gpNearest->ThisRem);	
	}
	else
	{
		TfTimCancle();
	}
	LeaveCritical();
	
	//SecFuncRcdDisp();

	//开始执行callback
	for(i=0;i<TFUNC_RCD_NUM;i++)
	{		
		FUNC_CB_INFO *pCb=&pCbInfo[i];
		if(pCb->NeedExcFlag==TRUE)
		{
			if(pCb->pStdCB!=NULL) pCb->pStdCB(pCb->IntParam,pCb->pParam);
			else if(pCb->pExpCB!=NULL) pCb->pExpCB(pCb->EntresCnt,pCb->IntParam,pCb->pParam);
		}				
	}

	Q_Free(pCbInfo);
}


//检查一个cb是否已经被设置到定时函数中
bool SecFuncAlready(void *pCB)
{
	u8 i;
	IntSaveInit();

	if(pCB!=NULL)
	{
		EnterCritical();
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			if(gTfRcd[i].Flag == 0) continue;

			if(gTfRcd[i].pStdCB==pCB || gTfRcd[i].pExpCB==pCB)
			{
				LeaveCritical();
				return TRUE;
			}			
		}	
		LeaveCritical();
	}
	
	return FALSE;
}

//删除所有同一个回调函数的定时函数
void DeleteSecFuncByCB(void *pCB)
{
	TFUNC_RCD *pRecord;
	u32 MinVal=0xffffffff;
	u16 i;
	IntSaveInit();

	if(pCB!=NULL)
	{
		EnterCritical();
		for(i=0;i<TFUNC_RCD_NUM;i++)
		{
			if(gTfRcd[i].Flag == 0) continue;

			if(gTfRcd[i].pStdCB==pCB || gTfRcd[i].pExpCB==pCB)
			{
				MemSet((void *)&gTfRcd[i],0,sizeof(TFUNC_RCD));	//销毁记录	
			}			
		}

		//查找最近的定时函数
		gpNearest=NULL;
		for(i=0,pRecord=(void *)gTfRcd;i<TFUNC_RCD_NUM;i++,pRecord++)
		{
			if(pRecord->Flag == 0) continue;
			if(pRecord->ThisRem < MinVal) 
			{
				MinVal=pRecord->ThisRem;
				gpNearest=pRecord;
			}
		}
			
		if(gpNearest != NULL)//已经找到下一次的最小定时器
		{
			TfTimSet(gpNearest->ThisRem);	
		}
		else
		{
			TfTimCancle();
		}
		
		LeaveCritical();
	}
}

