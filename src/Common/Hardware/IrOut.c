#include "Drivers.h"

#if 1//定时器+io的形式
#define IrPluseGen_ISR TIM3_IRQHandler
#define IrPluseTimerSet Tim3_Set
#define IrPluseTimerID TIM3

//设置发送或者停止发送38k波形
//Val非零时发送ir
void SetSendIrData(u8 Val) 
{
	IrPluseTimerSet((Val)?13:0,1,TRUE);
	IOOUT_SetIoStatus(IOOUT_IR_OUT,(Val)?TRUE:FALSE);
}

//开启时发送38k载波
void IrPluseGen_ISR(void)
{
	if(TIM_GetITStatus(IrPluseTimerID, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(IrPluseTimerID, TIM_IT_Update);

		if(IOOUT_ReadIoStatus(IOOUT_IR_OUT))//现在是高电平
			IOOUT_SetIoStatus(IOOUT_IR_OUT,FALSE);
		else
			IOOUT_SetIoStatus(IOOUT_IR_OUT,TRUE);
	}
}
#else//pwm形式
//设置发送或者停止发送38k波形
//Val非零时发送ir
void SetSendIrData(u8 Val) 
{
	PWM1_CONFIG((Val)?26:0,1,13);//pa6,tim3
}
#endif

