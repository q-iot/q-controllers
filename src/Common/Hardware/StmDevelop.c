//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件定义了与stm32硬件相关的函数，移植Q-Ctrl时此部分需重写
*/
//------------------------------------------------------------------//
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

//获取硬件唯一ID
u32 GetHwID(u8 *pID)
{
	static u32 HwID=0;
	u8 i;

	if(pID==NULL)
	{
		if(HwID==0) HwID=MakeHash33((u8 *)0x1FFFF7E8,12);
	}
	else
	{
		HwID=MakeHash33((u8 *)0x1FFFF7E8,12);
	    for(i=0;i<12;i++) pID[i]=*(u8 *)(0x1FFFF7E8+i);
	}
	
	return HwID;
}

#define MAX_SYSCALL_INTERRUPT_PRIORITY 	(1<<4) 

__asm u32 SaveCpuStatus(void)
{
#if 0
	PRESERVE8

	push { r0 }
	mov r0, #MAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	pop { r0 }
	bx r14
#else 
    MRS     R0, PRIMASK 
    CPSID   I
    BX      LR
#endif
}

__asm void RestoreCpuStatus(u32 cpu_sr)
{
#if 0
	PRESERVE8

	push { r0 }
	mov r0, #0
	msr basepri, r0
	pop { r0 }
	bx r14
#else
    MSR     PRIMASK, R0
    BX      LR
#endif
}





