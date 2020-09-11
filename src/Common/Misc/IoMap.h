#ifndef IO_MAP_H
#define IO_MAP_H

typedef enum{
	GPI_A=0,
	GPI_B,
	GPI_C,
	GPI_D,
	GPI_E,
	GPI_F,
	GPI_G,

	GPI_MAX
}_GPI;

typedef enum{
	GPin0=0,
	GPin1,
	GPin2,
	GPin3,
	GPin4,
	GPin5,
	GPin6,
	GPin7,
	GPin8,
	GPin9,
	GPin10,
	GPin11,
	GPin12,
	GPin13,
	GPin14,
	GPin15,

	GPin_MAX
}_GPIN;

typedef struct{
	GPIO_TypeDef* GpioGroup;
	uint32_t RccId;
	uint8_t GpioPortSource;
}GPIO_GROUP_MAP;

typedef struct{
	uint16_t GpioPin;
	uint8_t GpioPinSource;
	uint32_t ExtiLine;
}GPIO_PIN_MAP;

#endif
