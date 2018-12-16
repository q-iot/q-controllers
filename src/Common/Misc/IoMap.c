
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

const GPIO_GROUP_MAP gGroupMap[GPI_MAX]={
{GPIOA,RCC_APB2Periph_GPIOA,GPIO_PortSourceGPIOA},
{GPIOB,RCC_APB2Periph_GPIOB,GPIO_PortSourceGPIOB},
{GPIOC,RCC_APB2Periph_GPIOC,GPIO_PortSourceGPIOC},
{GPIOD,RCC_APB2Periph_GPIOD,GPIO_PortSourceGPIOD},
{GPIOE,RCC_APB2Periph_GPIOE,GPIO_PortSourceGPIOE},
{GPIOF,RCC_APB2Periph_GPIOF,GPIO_PortSourceGPIOF},
{GPIOG,RCC_APB2Periph_GPIOG,GPIO_PortSourceGPIOG}
};

const GPIO_PIN_MAP gPinMap[GPin_MAX]={
{GPIO_Pin_0,GPIO_PinSource0,EXTI_Line0},
{GPIO_Pin_1,GPIO_PinSource1,EXTI_Line1},
{GPIO_Pin_2,GPIO_PinSource2,EXTI_Line2},
{GPIO_Pin_3,GPIO_PinSource3,EXTI_Line3},
{GPIO_Pin_4,GPIO_PinSource4,EXTI_Line4},
{GPIO_Pin_5,GPIO_PinSource5,EXTI_Line5},
{GPIO_Pin_6,GPIO_PinSource6,EXTI_Line6},
{GPIO_Pin_7,GPIO_PinSource7,EXTI_Line7},
{GPIO_Pin_8,GPIO_PinSource8,EXTI_Line8},
{GPIO_Pin_9,GPIO_PinSource9,EXTI_Line9},
{GPIO_Pin_10,GPIO_PinSource10,EXTI_Line10},
{GPIO_Pin_11,GPIO_PinSource11,EXTI_Line11},
{GPIO_Pin_12,GPIO_PinSource12,EXTI_Line12},
{GPIO_Pin_13,GPIO_PinSource13,EXTI_Line13},
{GPIO_Pin_14,GPIO_PinSource14,EXTI_Line14},
{GPIO_Pin_15,GPIO_PinSource15,EXTI_Line15}
};
