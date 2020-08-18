#ifndef DRIVERS_H
#define DRIVERS_H

#define Bit(bit)		(1<<(bit))
#define SetBit(reg,bit) (reg|=(1<<(bit)))
#define ClrBit(reg,bit) (reg&=~(1<<(bit)))
#define RevBit(reg,bit) (reg^=(1<<(bit)))
#define ReadBit(reg,bit) ((reg>>(bit))&1)
#define HBit8(v) (((v)>>8)&0xff)
#define LBit8(v) ((v)&0xff)
#define HBit16(v) (((v)>>16)&0xffff)
#define LBit16(v) ((v)&0xffff)

//用u8数组来表示众多对象的标识
#define SetArrayBit(a,bit) SetBit(a[(bit)>>3],7-((bit)&0x7))
#define ClrArrayBit(a,bit) ClrBit(a[(bit)>>3],7-((bit)&0x7))
#define ReadArrayBit(a,bit) ReadBit(a[(bit)>>3],7-((bit)&0x7))



#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "system_stm32f10x.h"
#include "misc.h"
#include "core_cm3.h"
#include "Uart.h"
#include "Adc.h"  
#include "Spi.h"
#include "IoDefines.h"
#include "Time.h"
#include "rtc.h"
#include "Ir.h"
#include "Delay.h"
#include "RomFlash.h"
#include "PublicFunc.h"
#include "StmDevelop.h"
#include "Sx1276.h"

/* ISR Priority 0(highest)-15(lowest)*/
//数字越小优先级越高	
#define SYSTICK_INT_Priority	0
#define EXTI_Si_Priority 3
#define EXTI_Pio_Priority 3
#define USART1_IRQn_Priority 3
#define USART3_IRQn_Priority 3




#endif
