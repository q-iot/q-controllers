#include "SysDefines.h"

void IWDG_Configuration(void)
{
	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* 狗狗时钟分频,40K/256=156HZ(6.4ms)*/
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* 喂狗时间 1s/6.4MS=156 .注意不能大于0xfff*/
	IWDG_SetReload(156*5);

	/* 喂狗*/
	IWDG_ReloadCounter();

	/* 使能狗狗*/
	IWDG_Enable();
}

void IWDG_PeriodCB(void)
{
	IWDG_ReloadCounter();
}

//重启板卡
void RebootBoard(void) 
{
	SaveCpuStatus();//关闭所有中断

	SCB->AIRCR  = ((0x05FA << SCB_AIRCR_VECTKEY_Pos) | 
	               (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | 
	               SCB_AIRCR_SYSRESETREQ_Msk);              
	__DSB();                                                      

	while(1);
}

void DefaultConfig(void)
{
	//RFS_BurnDefaultToRom();
	RebootBoard();
}

