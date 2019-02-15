//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了time驱动和定时器库，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//
#include "Drivers.h"

void Tim1_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim2_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim3_Init(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Tim4_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//uS_Base 表示单位，为1时，单位是us；为100时，单位是100us；最小值1，最大值900
//最终定时值= Val x uS_Base x 1us
//新定时设定会覆盖旧设定
//AutoReload用来设定是一次定时还是循环定时
//val和uS_Base其中任意一个为0，则停止当前定时。
//val不能为1，否则tim不工作
void Tim1_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM1, DISABLE);

    if(Val==0||uS_Base==0) return;

	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base>900) uS_Base=900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);//当装载器到此值时，cnt加1
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1使用APB2，72m时钟，比其他tim快一倍

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM1,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM1,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);/* 使能TIM1的 向上溢出 中断 */    

	TIM_Cmd(TIM1, ENABLE);/* 使能TIM1 */	 
}

//uS_Base 表示单位，为1时，单位是us；为100时，单位是100us；最小值1，最大值900
//最终定时值= Val x uS_Base x 1us
//新定时设定会覆盖旧设定
//AutoReload用来设定是一次定时还是循环定时
//val和uS_Base其中任意一个为0，则停止当前定时。
//val不能为1，否则tim不工作
void Tim2_Set(u16 Val,u16 uS_Base,bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM2, DISABLE);

    if(Val==0||uS_Base==0) return;

	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base>900) uS_Base=900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    TIM_TimeBaseStructure.TIM_Period        = (Val-1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM2,TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);/* 使能TIM2的 向上溢出 中断 */    

	TIM_Cmd(TIM2, ENABLE);/* 使能TIM2 */	 
}

void Tim3_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM3, DISABLE);

    if((Val == 0) || (uS_Base == 0)) return;

	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2 / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);/* 使能TIM3的 向上溢出 中断 */    

	TIM_Cmd(TIM3, ENABLE);/* 使能TIM3 */	 
}

void Tim4_Set(u16 Val, u16 uS_Base, bool AutoReload)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_ClocksTypeDef RCC_Clocks;

    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM4, DISABLE);

    if(Val == 0||uS_Base == 0) return;
	
	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;
    
    //定时频率为： 72M/(预分频+1)/预装载
    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    TIM_TimeBaseStructure.TIM_Period        = (Val - 1);
    TIM_TimeBaseStructure.TIM_Prescaler     = ((RCC_Clocks.PCLK1_Frequency*2 / 1000000) * uS_Base - 1);

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	if(AutoReload) TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Repetitive);
	else TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);
	
	TIM_ClearFlag(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);/* 使能TIM4的 向上溢出 中断 */    

	TIM_Cmd(TIM4, ENABLE);/* 使能TIM4 */	 
}

//PA6: TIM3_CH1
//占空比=Pluse/Val
//周期=Val*uS_Base
void IO7_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse)
{
	static u16 Old_Val=0;
	static u16 Old_uS_Base=0;
	static u16 Old_Pluse=0;
	
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
    TIM_OCInitTypeDef  TIM_OCInitStructure;  
    RCC_ClocksTypeDef RCC_Clocks;

	if(Old_Val==Val && Old_uS_Base==uS_Base && Old_Pluse==Pluse)//避免重复进入
	{
		return;
	}
	else
	{
		Old_Val=Val;
		Old_uS_Base=uS_Base;
		Old_Pluse=Pluse;
	}
	Debug("IO7 %u %u %u\n\r",Val,Pluse,uS_Base);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  
    TIM_DeInit(TIM3);

    if(Val && uS_Base && Pluse)//设置有效值
	{ 
		if(Pluse>=Val)//置高
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
			GPIO_SetBits(GPIOA,GPIO_Pin_6);

			return;
		}
		else//pwm
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // 复用推挽输出 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
		}
	} 
	else //置低
	{
		GPIO_InitTypeDef GPIO_InitStructure; 

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);

		return;
	}
    
    if(Val == 0 || uS_Base == 0 || Pluse==0) return;

	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;    

    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    
    TIM_TimeBaseStructure.TIM_Period = (Val-1);//当装载器到此值时，cnt加1
    TIM_TimeBaseStructure.TIM_Prescaler = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1使用APB2，72m时钟，比其他tim快一倍

    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;                                //设置时钟分频系数：不分频  
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;                 //向上计数溢出模式  
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);  
    
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;                           //配置为PWM模式1  
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;                
    TIM_OCInitStructure.TIM_Pulse = Pluse;                                       //设置跳变值，当计数器计数到这个值时，电平发生跳变  
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;                    //当定时器计数值小于CCR1时为高电平  
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;     //MOE=0 设置 TIM1输出比较空闲状态
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);                                    //使能通道1      
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  

    TIM_Cmd(TIM3,ENABLE);                                                      //使能TIM3      
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
}

//PA7: TIM1_CH1N
//占空比=Pluse/Val
//周期=Val*uS_Base
void IO8_PWM_CONFIG(u16 Val, u16 uS_Base,u16 Pluse)
{   
	static u16 Old_Val=0;
	static u16 Old_uS_Base=0;
	static u16 Old_Pluse=0;
	
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    RCC_ClocksTypeDef RCC_Clocks;

	if(Old_Val==Val && Old_uS_Base==uS_Base && Old_Pluse==Pluse)//避免重复进入
	{
		return;
	}
	else
	{
		Old_Val=Val;
		Old_uS_Base=uS_Base;
		Old_Pluse=Pluse;
	}
   	Debug("IO8 %u %u %u\n\r",Val,Pluse,uS_Base);
   	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  
    TIM_DeInit(TIM1);

	if(Val && uS_Base && Pluse)
	{ 
		if(Pluse>=Val)//置高
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
			GPIO_SetBits(GPIOA,GPIO_Pin_7);

			return;
		}
		else//pwm
		{
			GPIO_InitTypeDef GPIO_InitStructure; 

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // 复用推挽输出 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
			GPIO_Init(GPIOA, &GPIO_InitStructure); 
		}
	} 
	else//置低
	{ 
		GPIO_InitTypeDef GPIO_InitStructure; 

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  

		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(GPIOA, &GPIO_InitStructure); 
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);

		return;
	}
  
    if(Val == 0 || uS_Base == 0 || Pluse==0) return;

	if(Val==1)//解决val不能为1的问题
	{
		Val=2;
		uS_Base>>=1;
		if(uS_Base==0) uS_Base=1;
    }
    
    if(uS_Base > 900) uS_Base = 900;    

    RCC_GetClocksFreq(&RCC_Clocks);//获取系统频率
    
    TIM_TimeBaseStructure.TIM_Period = (Val-1);//当装载器到此值时，cnt加1
    TIM_TimeBaseStructure.TIM_Prescaler = ((RCC_Clocks.PCLK1_Frequency*2/1000000)*uS_Base - 1);//tim1使用APB2，72m时钟，比其他tim快一倍
    
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式  /* 向上计数模式 */  
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;        //设置 周期 计数值
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;           //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;         //使能输出比较状态
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //使能  互补 输出状态
    TIM_OCInitStructure.TIM_Pulse = Pluse;                //设置待装入捕获比较寄存器的脉冲值         占空时间 
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;       //输出极性:TIM输出比较极性低
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //互补 输出极性高
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;     //MOE=0 设置 TIM1输出比较空闲状态
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  //MOE=0 重置 TIM1输出比较空闲状态
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);        //设定好的参数 初始化TIM  
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);

    TIM_Cmd(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


