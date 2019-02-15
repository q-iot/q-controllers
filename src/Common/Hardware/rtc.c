//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了rtc驱动和年月日时间库，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//

#include "Drivers.h"

#define RTC_START_YEAR 1980  //可从1904年后的任何一个闰年开始,如果改变此值，星期初值要改变
#define RTC_WEEK_INIT_OFFSET 2//周天取值0，周六取值6以此类推
#define DAY_SECONDS 86400	//一天的总秒数

//闰年的逐月秒数计算
const int Leap_Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+29),
	DAY_SECONDS*(31+29+31),
	DAY_SECONDS*(31+29+31+30),
	DAY_SECONDS*(31+29+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30+31),
};

//非闰年的逐月秒数计算
const int Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+28),
	DAY_SECONDS*(31+28+31),
	DAY_SECONDS*(31+28+31+30),
	DAY_SECONDS*(31+28+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30+31),
};

#define RTCClockSource_LSE // 使用外部时钟， 32.768KHz 


//时间转计数
//格式不对返回0
u32 RtcTime2Cnt(RTC_TIME *pTime)
{
	u32 TimeCnt,Tmp;
	u8 LeapFlag=0;

	TimeCnt=pTime->year-RTC_START_YEAR;
	if((TimeCnt>135)||(pTime->year<RTC_START_YEAR)) return 0; //年份检测
	
	if(TimeCnt) Tmp=(TimeCnt-1)/4+1;//判断闰年个数
	else Tmp=0;
	LeapFlag=(TimeCnt%4)?0:1;
	TimeCnt=(TimeCnt*365+Tmp)*DAY_SECONDS;//年换算成的秒数

	if((pTime->mon<1)||(pTime->mon>12)) return 0;  //月份检查
	
	if(LeapFlag)
	{
		if(pTime->day>((Leap_Month_Seconds[pTime->mon]-Leap_Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0; //日检查
		Tmp=Leap_Month_Seconds[pTime->mon-1];
	}
	else
	{
		if(pTime->day>((Month_Seconds[pTime->mon]-Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0;//日检查
		Tmp=Month_Seconds[pTime->mon-1];
	}

	if(pTime->hour>23) return 0; //小时检查
	if(pTime->min>59) return 0;	 //分钟检查
	if(pTime->sec>59) return 0;	 //秒检查

	TimeCnt+=(Tmp+(pTime->day-1)*DAY_SECONDS);

	TimeCnt+=(pTime->hour*3600 + pTime->min*60 + pTime->sec);

	return TimeCnt;
}

//计数转时间
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime)
{ 
  u32 Tmp,i;

  //计算周期年，考虑到闰年的存在，以4年一个周期
  Tmp=TimeCnt%(DAY_SECONDS*366+DAY_SECONDS*365*3);
  if(Tmp<DAY_SECONDS*366) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+0;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+1;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+2;
  else pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+3;

  if(Tmp<DAY_SECONDS*366) //闰年
  {
		for(i=1;i<13;i++)
	    {
			if(Tmp<Leap_Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Leap_Month_Seconds[i-1];//用数组查询代替复杂的计算
				break;
		  	}
	   }
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1)
  {
		Tmp-=DAY_SECONDS*366;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}		
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2)
  {
  		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*1;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}	  		
  }
  else
  {
		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*2;
		
	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}			
  }

  pTime->week=(TimeCnt/DAY_SECONDS+RTC_WEEK_INIT_OFFSET)%7;//因为1912.1.1是星期一，所以偏移值=1
  //注意获取的值范围为0-6，这样是为了方便数组查阅，从而翻译成中文显示

  pTime->day=Tmp/DAY_SECONDS+1;
  Tmp=Tmp%DAY_SECONDS;

  pTime->hour = Tmp/3600;
  Tmp=Tmp%3600;

  pTime->min = Tmp/60;

  pTime->sec = Tmp%60;
}

/*******************************************************************************
* Function Name  : RtcConfiguration
* Description    : Configures the rtc.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#define RTC_RST_FLAG		0xa5a5
void RtcConfiguration(void)
{
    u32 rtcintcnt=0x200000;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  //打开电源管理和备份寄存器时钟
    PWR_BackupAccessCmd(ENABLE);            //使能RTC和备份寄存器的访问(复位默认关闭)
    BKP_DeInit();                           //BKP外设复位
    RCC_LSEConfig(RCC_LSE_ON);              //打开外部低速晶体
    
    //while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (--rtcintcnt));//等待LSE准备好
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    if(rtcintcnt!=0)//内部晶振
    {
    	Debug("RTC LSE WORKING\n\r");
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); //选择LSE位RTC时钟
        RCC_RTCCLKCmd(ENABLE);                  //使能RTC时钟
        RTC_WaitForSynchro();                   //等待RTC寄存器和APB时钟同步
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成(必须在对RTC寄存器写操作钱调用)
        RTC_ITConfig(RTC_IT_SEC, ENABLE);       //使能RTC秒中断
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成
        RTC_ITConfig(RTC_IT_ALR, ENABLE);
        RTC_WaitForLastTask();                  //等待RTC寄存器写操作完成
        RTC_SetPrescaler(32767);                //设置RTC预分频器值产生1秒信号计算公式 fTR_CLK = fRTCCLK/(PRL+1)
        RTC_WaitForLastTask();   
    }
    else//用外部晶振
    {
    	Debug("!!!RTC LSE NO WORK!!!\n\r");
        rtcintcnt=0x200000;    
        RCC_HSEConfig(RCC_HSE_ON);/* Enable HSE */
        while ( (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) && (--rtcintcnt) );/* Wait till HSE is ready */
        if ( rtcintcnt == 0 )
        {
            return;
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);/* Select HSE/128 as RTC Clock Source */
    
	    RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */    
	    RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
	    
	    /* Set RTC prescaler: set RTC period to 1sec */
	    RTC_SetPrescaler(8000000/128-1); /* RTC period = RTCCLK/RTC_PR = (8MHz/128)/(8000000/128) */
	    
	    RTC_WaitForLastTask();/* Wait until last write operation on RTC registers has finished */
    }
}

/*******************************************************************************
* Function Name  : RTC_Config
* Description    : 上电时调用本函数，自动检查是否需要RTC初始化， 
*                       若需要重新初始化RTC，则调用RTC_Configuration()完成相应操作
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RtcSetUp(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_TIME NowTime;

	/* 使能rtc中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//检查备用寄存器的值是否改变了，如果改变了，说明battery掉电了，需要重新配置rtc
	if(BKP_ReadBackupRegister(BKP_RTC_FLAG) != RTC_RST_FLAG)
	{
		/* Backup data register value is not correct or not yet programmed (when
		 	the first time the program is executed) */
		Debug("RTC need configure!\n\r");
		
		/* RTC Configuration */
		RtcConfiguration();

		NowTime.year=2012;
		NowTime.mon=1;
		NowTime.day=1;
		NowTime.hour=0;
		NowTime.min=0;
		NowTime.sec=30;
		if(RtcAdjustTime(&NowTime,RtcOp_SetTime)==TRUE)
		{
			Debug("RTC set sucess!\n\r");
		}
		else
		{
			Debug("RTC set error!\n\r");
		}

/*
		NowTime.sec=0;
		if(RtcAdjustTime(&NowTime,RtcOp_SetAlarm)==TRUE)
		{
			Debug("Alarm set sucess!\n\r");
		}
		else
		{
			Debug("Alarm set error!\n\r");
		}		
*/
		//备用寄存器写入一个值，供下次做掉电检查
		BKP_WriteBackupRegister(BKP_RTC_FLAG, RTC_RST_FLAG);  
	}
	else
	{
		if(RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)//检查是否上电复位
	    {
	      Debug("System power on reset!\n\r");
	    }
	    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)//检查是否手动复位
	    {
	      Debug("System reset!\n\r");
	    }	   
	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	    PWR_BackupAccessCmd(ENABLE);	
	    
		Debug("No need to configure RTC....\n\r");
		
		RTC_WaitForSynchro();//等待RTC寄存器和APB时钟同步
	    RTC_ITConfig(RTC_IT_SEC, ENABLE);//使能RTC秒中断
	    RTC_WaitForLastTask();//等待写操作完成
	    RTC_ITConfig(RTC_IT_ALR, ENABLE);
	    RTC_WaitForLastTask();//等待写操作完成
	}
	RCC_ClearFlag();//清除复位标志
	
	RTC_WaitForLastTask();

	Debug("RTC configured finished\n\r");
	RTC_WaitForLastTask();

	RtcReadTime(&NowTime,RtcOp_GetAlarm);
	Debug("Alarm:%d.%d.%d %d:%d:%d\n\r",NowTime.year,NowTime.mon,NowTime.day,
		NowTime.hour,NowTime.min,NowTime.sec);
}

void RtcSetUp2(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RTC_TIME NowTime;

	/* 使能rtc中断 */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/*PWR时钟（电源控制）与BKP时钟（RTC后备寄存器）使能*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/*使能RTC和后备寄存器访问*/
	PWR_BackupAccessCmd(ENABLE);

	/*从指定的后备寄存器（BKP_DR1）中读出数据*/
	if(BKP_ReadBackupRegister(BKP_RTC_FLAG) != RTC_RST_FLAG)
	{
		/* 将外设BKP的全部寄存器重设为缺省值 */
		BKP_DeInit(); 

		/* 启用 LSE（外部低速晶振）*/
		RCC_LSEConfig(RCC_LSE_ON); 
		/*等待外部晶振震荡 需要等待比较长的时间*/
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

		/*使用外部晶振32.768K作为RTC时钟*/
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);  
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		//允许RTC的秒中断(还有闹钟中断和溢出中断可设置)
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		RTC_WaitForLastTask();
		RTC_ITConfig(RTC_IT_ALR, ENABLE);
		RTC_WaitForLastTask();
		
		//32768晶振预分频值是32767,不过一般来说晶振都不那么准
		RTC_SetPrescaler(32767);  //如果需要校准晶振,可修改此分频值
		RTC_WaitForLastTask();

		//写入RTC后备寄存器1 0xa5a5
		BKP_WriteBackupRegister(BKP_RTC_FLAG, RTC_RST_FLAG);  
		//RTC_Blank=1; /*这个标志代表RTC是没有预设的(或者说是没有上纽扣电池) 用串口呀啥的输出来。*/
	}
	//如果RTC已经设置
	else
	{
		//等待RTC与APB同步
		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		//使能秒中断 
		RTC_ITConfig(RTC_IT_SEC, ENABLE);  //这句可以放到前面吗？
		RTC_WaitForLastTask();//等待写操作完成
	    RTC_ITConfig(RTC_IT_ALR, ENABLE);
		RTC_WaitForLastTask(); //又等....
	 }

	//清除标志
	RCC_ClearFlag(); 

	RtcReadTime(&NowTime,RtcOp_GetAlarm);
	Debug("Alarm:%d.%d.%d %d:%d:%d\n\r",NowTime.year,NowTime.mon,NowTime.day,
		NowTime.hour,NowTime.min,NowTime.sec);
}

static uint32_t RTC_GetAlarmCount(void)
{
  uint16_t tmp = 0;
  tmp = RTC->ALRL;
  return (((uint32_t)RTC->ALRH << 16 ) | tmp) ;
}

//获取当前时间的函数
//只需要定义一个结构体实体，将地址赋予给pTime，就可以得到当前的日期时间
//算法比较繁琐，学习的人要耐心看
//在arm构架里，除法需要比较长的时间，所以尽量避免少用除法和余法
void RtcReadTime(RTC_TIME *pTime,RTC_OPERATE Op)
{ 
	u32 TimeCnt=0;

	RTC_WaitForLastTask();
	if(Op==RtcOp_GetAlarm) TimeCnt = RTC_GetAlarmCount();//获取当前闹钟值
	else TimeCnt = RTC_GetCounter();//获取当前计时器值
	RTC_WaitForLastTask();

	//Debug("RTC READ CNT:%u\n\r",TimeCnt);
	RtcCnt2Time(TimeCnt,pTime);
}

//时间调整函数
//参数传递用结构体地址，不需要设定星期值，注意要定义结构体实体！
bool RtcAdjustTime(RTC_TIME *pTime,RTC_OPERATE Op)
{
	u32 TimeCnt;

	TimeCnt=RtcTime2Cnt(pTime);

	if(TimeCnt == 0) return FALSE;
	
	switch(Op)
	{	
		case RtcOp_SetTime:
			RTC_WaitForLastTask(); 
			RTC_SetCounter(TimeCnt);	
			RTC_WaitForLastTask();

			RTC_WaitForLastTask();

			RTC_WaitForLastTask();
			break;
		case RtcOp_SetAlarm:
			RTC_WaitForLastTask(); 
			RTC_SetAlarm(TimeCnt);
			RTC_WaitForLastTask();
			break;
	}
	
	return TRUE;
}



