#ifndef __IO_DEFINES_H__
#define __IO_DEFINES_H__

#include "IoMap.h"

#if PRODUCT_IS_LIFE1
typedef enum{
	IOIN_PIO0=0,
	IOIN_PIO1,
	IOIN_PIO2,
	IOIN_PIO3,
	IOIN_PIO4,
	IOIN_PIO5,
	IOIN_PIO6,
	IOIN_PIO7,
	IOIN_WRF_DRV_INT,
	
	IOIN_MAX,

	IOIN_KEY1=0,
	IOIN_KEY2=1,
	IOIN_IR_IN=2,
	IOIN_USER_KEY=0,
	
}IO_IN_DEFS;

typedef enum{
	IOOUT_LED1=0,
	IOOUT_LED2,
	IOOUT_FLASH_CS,
	IOOUT_FLASH_WP,
	IOOUT_WRF_DRV_CS,
	IOOUT_WRF_DRV_RESET,

	IOOUT_MAX,

	IOOUT_IR_OUT=0,
	
}IO_OUT_DEFS;


#else
typedef enum{
	IOIN_PIO0=0,
	IOIN_PIO1,
	IOIN_PIO2,
	IOIN_PIO3,
	IOIN_PIO4,
	IOIN_PIO5,
	IOIN_PIO6,
	IOIN_PIO7,
	IOIN_IR_IN,
	IOIN_USER_KEY,
	IOIN_WRF_DRV_INT,
	
	IOIN_MAX
}IO_IN_DEFS;

typedef enum{
	IOOUT_LED1=0,
	IOOUT_LED2,
	IOOUT_FLASH_CS,
	IOOUT_FLASH_WP,
	IOOUT_IR_OUT,
	IOOUT_WRF_DRV_CS,
	IOOUT_WRF_DRV_RESET,

	IOOUT_MAX
}IO_OUT_DEFS;
#endif

typedef struct{
	IO_IN_DEFS Idx;
	u8 Group;
	u8 Pin;
	GPIOMode_TypeDef GpioMode;
	EXTITrigger_TypeDef ExtiTrigger;
	uint8_t NvicIRQChannel;
	uint8_t NvicIRQPriority;
}IO_IN_HAL_DEFINE;

typedef struct{
	IO_OUT_DEFS Idx;
	u8 Group;
	u8 Pin;
	GPIOMode_TypeDef GpioMode;
	bool InitSet;//Ä¬ÈÏÖµ
}IO_OUT_HAL_DEFINE;

void IoDefinesInit(void);
void GPIO_ConfigOne(const IO_IN_HAL_DEFINE *pInPin,const IO_OUT_HAL_DEFINE *pOutPin);
void EXTI_ConfigOne(const IO_IN_HAL_DEFINE *pInPin);
void NVIC_ConfigOne(const IO_IN_HAL_DEFINE *pInPin);


void IOOUT_SetIoStatus(IO_OUT_DEFS Io,bool IsHigh);
void IOOUT_SetOne(const IO_OUT_HAL_DEFINE *pOutPin,bool IsHigh);
u8 IOOUT_ReadIoStatus(IO_OUT_DEFS Io);

void IOIN_SetIoStatus(IO_IN_DEFS Io,bool IsHigh);
u8 IOIN_ReadIoStatus(IO_IN_DEFS Io);
u8 IOIN_ReadOne(const IO_IN_HAL_DEFINE *pInPin);

void IOIN_OpenExti(IO_IN_DEFS Io);
void IOIN_OpenOneExti(const IO_IN_HAL_DEFINE *pInPin);
void IOIN_CloseExti(IO_IN_DEFS Io);
void IOIN_CloseOneExti(const IO_IN_HAL_DEFINE *pInPin);
u16 IOIN_ReadExti(IO_IN_DEFS Io) ;
u16 IOIN_ReadOneExti(const IO_IN_HAL_DEFINE *pInPin);

#define LedSet(LedId,Status) IOOUT_SetIoStatus(LedId,Status?FALSE:TRUE)
#define LedRev(LedId) IOOUT_SetIoStatus(LedId,IOOUT_ReadIoStatus(LedId)?FALSE:TRUE)


#endif

