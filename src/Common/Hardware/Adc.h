#ifndef __ADC_H__
#define __ADC_H__

#include "Product.h"

void Adc1_Rand_Init(u8 UseAio);
u16 Adc1_GetVal(u8 Chan);
u16 Adc1_GetValByAio(u8 AioID);
u32 GetAdcRand(void);
u16 GetCpuTemp(void);

#define ADC_RAND_CH 1
#define ADC_CPU_CH 2
#define ADC_VREF_CH 2

#endif

