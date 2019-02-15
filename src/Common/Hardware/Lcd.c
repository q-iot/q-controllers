//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了ILI9320驱动和简单的gui，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//

#include "Drivers.h"
#include "Lcd.h"
#if LCD_BGLIGHT_PWM_MODE
#include "Time.h"
#endif

#define LCD_ILI9320_On() GPIO_SetBits(GPIOC,GPIO_Pin_8)
#define LCD_ILI9320_Off() GPIO_ResetBits(GPIOC,GPIO_Pin_8)

#define Bank1_LCD_R    ((uint32_t)0x60000000)    //disp Reg ADDR
#define Bank1_LCD_D    ((uint32_t)0x60020000)	//disp Data ADDR 

static u8 gLcdScale=100;

//9320功能寄存器地址
#define WINDOW_XADDR_START	0x0050 // 水平的开始地址组
#define WINDOW_XADDR_END		0x0051 // 水平的结束地址组
#define WINDOW_YADDR_START	0x0052 // 垂直的开始地址组
#define WINDOW_YADDR_END		0x0053 // 垂直的结束地址组
#define GRAM_XADDR		    		0x002A // GRAM 水平的地址组
#define GRAM_YADDR		    		0x002B // GRAM 垂直的地址组
#define GRAMWR 			    			0x002C // Write GRAM
#define GRAMRD								0x002E // Read GRAM

/**********************************************
函数名：LCD_DelayMs
功能：用于LCD配置延时
入口参数：延时数
返回值：无
***********************************************/
static void LCD_DelayMs(u32 Ms)
{
  u32 i;
	for(; Ms; Ms--)
		for(i=1000;i;i--);
}

/*************************************************
函数名：LCD_WriteIndex
功能：传入寄存器地址
入口参数：寄存器地址
返回值：无
*************************************************/
static void LCD_WriteIndex(u16 index)
{
	*(__IO u16 *) (Bank1_LCD_R)= index;	
}

/*************************************************
函数名：LCD_WR_Reg
功能：对lcd寄存器，写命令
入口参数：寄存器地址和命令
返回值：寄存器值
*************************************************/
static void LCD_WriteReg(u16 index,u16 val)
{	
	*(__IO u16 *) (Bank1_LCD_R)= index;	
	*(__IO u16 *) (Bank1_LCD_D)= val;	
}

/*************************************************
函数名：LCD_RD_Reg
功能：对lcd寄存器，读值
入口参数：寄存器地址
返回值：寄存器值
*************************************************/
static u16 LCD_ReadReg(u16 index)
{	
	*(__IO u16 *) (Bank1_LCD_R)= index;	
	return (*(__IO u16 *) (Bank1_LCD_D));
}

/*************************************************
函数名：LCD_WR_Data
功能：对lcd写数据
入口参数：数据值
返回值：无
*************************************************/
static void LCD_WriteData(u16 val)
{   
	*(__IO u16 *) (Bank1_LCD_D)= val; 	
}

/*************************************************
函数名：LCD_RD_Data
功能：读lcd数据
入口参数：无
返回值：数据
*************************************************/
static u16 LCD_ReadData(void)
{
	return(*(__IO u16 *) (Bank1_LCD_D));	
}

/*************************************************
函数名：LCD_Power_On
功能：LCD驱动序列
入口参数：无
返回值：无
*************************************************/
static void LCD_PowerOn(void)
{
	u8 iddata[4];
	
    LCD_Reset();
    
    LCD_WriteIndex(0xD3);
    iddata[0] = LCD_ReadData();
    iddata[1] = LCD_ReadData();
    iddata[2] = LCD_ReadData();
    iddata[3] = LCD_ReadData();
    Debug("Lcd driver ic:%x%x%x%x\n\r",iddata[0],iddata[1],iddata[2],iddata[3]);
    
	LCD_WriteIndex(0xCF);
	LCD_WriteData(0x00);
	LCD_WriteData(0x81);
	LCD_WriteData(0x30);

	LCD_WriteIndex(0xED);
	LCD_WriteData(0x64);
	LCD_WriteData(0x03);
	LCD_WriteData(0x12);
	LCD_WriteData(0x81);

	LCD_WriteIndex(0xE8);
	LCD_WriteData(0x85);
	LCD_WriteData(0x10);
	LCD_WriteData(0x78);

	LCD_WriteIndex(0xCB);
	LCD_WriteData(0x39);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x00);
	LCD_WriteData(0x34);
	LCD_WriteData(0x02);

	LCD_WriteIndex(0xF7);
	LCD_WriteData(0x20);

	LCD_WriteIndex(0xEA);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);

	LCD_WriteIndex(0xB1);
	LCD_WriteData(0x00);
	LCD_WriteData(0x1B);

	LCD_WriteIndex(0xB6);
	LCD_WriteData(0x0A);
	LCD_WriteData(0xA2);

	LCD_WriteIndex(0xC0);
	LCD_WriteData(0x35);

	LCD_WriteIndex(0xC1);
	LCD_WriteData(0x11);

	LCD_WriteIndex(0xC5);
	LCD_WriteData(0x45);
	LCD_WriteData(0x45);

	LCD_WriteIndex(0xC7);
	LCD_WriteData(0xA2);

	LCD_WriteIndex(0xF2);
	LCD_WriteData(0x00);

	LCD_WriteIndex(0x26);
	LCD_WriteData(0x01);

	LCD_WriteIndex(0xE0); //Set Gamma
	LCD_WriteData(0x0F);
	LCD_WriteData(0x26);
	LCD_WriteData(0x24);
	LCD_WriteData(0x0B);
	LCD_WriteData(0x0E);
	LCD_WriteData(0x09);
	LCD_WriteData(0x54);
	LCD_WriteData(0xA8);
	LCD_WriteData(0x46);
	LCD_WriteData(0x0C);
	LCD_WriteData(0x17);
	LCD_WriteData(0x09);
	LCD_WriteData(0x0F);
	LCD_WriteData(0x07);
	LCD_WriteData(0x00);
	LCD_WriteIndex(0XE1); //Set Gamma
	LCD_WriteData(0x00);
	LCD_WriteData(0x19);
	LCD_WriteData(0x1B);
	LCD_WriteData(0x04);
	LCD_WriteData(0x10);
	LCD_WriteData(0x07);
	LCD_WriteData(0x2A);
	LCD_WriteData(0x47);
	LCD_WriteData(0x39);
	LCD_WriteData(0x03);
	LCD_WriteData(0x06);
	LCD_WriteData(0x06);
	LCD_WriteData(0x30);
	LCD_WriteData(0x38);
	LCD_WriteData(0x0F);

	LCD_WriteIndex(0x36); //rbg order
	LCD_WriteData(0x08);

	LCD_WriteIndex(0X2A); 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0xEF);

	LCD_WriteIndex(0X2B); 
	LCD_WriteData(0x00);
	LCD_WriteData(0x00);
	LCD_WriteData(0x01);
	LCD_WriteData(0x3F);

	LCD_WriteIndex(0x3a); // Memory Access Control
	LCD_WriteData(0x55);
	LCD_WriteIndex(0x11); //Exit Sleep
	
	LCD_DelayMs(120);
	LCD_WriteIndex(0x29); //display on
	LCD_WriteIndex(0x2c); 

    return;
}





/*************************************************
函数名：LCD_Power_Off
功能：LCD关闭序列
入口参数：无
返回值：无
*************************************************/
static void LCD_PowerOff(void)
{
	return;
}

/*************************************************
函数名：LCD_WR_Data_Start
功能：LCD开始批量传数据前调用
入口参数：无
返回值：无
*************************************************/
void LCD_BlukWriteDataStart(void)
{	
    LCD_WriteIndex(GRAMWR);
}

/*************************************************
函数名：LCD_ReadDataStart
功能：LCD开始批量传数据前调用
入口参数：无
返回值：无
*************************************************/
void LCD_BulkReadDataStart(void)
{	
    LCD_WriteIndex(GRAMRD);
}

/*************************************************
函数名：LCD_BulkWriteData
功能：对lcd批量写数据
入口参数：数据值
返回值：无
*************************************************/
void LCD_BulkWriteData(u16 val)
{   
	*(__IO u16 *) (Bank1_LCD_D)= val; 	
}

/*************************************************
函数名：LCD_BulkReadData
功能：批量读lcd数据
入口参数：无
返回值：数据
*************************************************/
u16 LCD_BulkReadData(void)//4???
{
  register u16 Data;
  LCD_ReadData();//丢掉无用字节
  Data=LCD_ReadData();
  Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB换序
  LCD_WriteData(Data);
  return Data;
}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_SetXY(u16 x,u16 y)
{
  LCD_WriteIndex(GRAM_XADDR);
  LCD_WriteData(x>>8);
  LCD_WriteData(x&0xFF);
  LCD_WriteIndex(GRAM_YADDR);
  LCD_WriteData(y>>8);
  LCD_WriteData(y&0xFF);
  //LCD_WriteIndex(GRAMWR);
}

/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点,Y_IncMode表示先自增y再自增x
返回值：无
*************************************************/
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist)
{		
	LCD_WriteIndex(0X2A);  //column address set 
	LCD_WriteData(x_start >> 8);     //set the head of column address
	LCD_WriteData(x_start & 0xff);
	LCD_WriteData(x_end >> 8);     //set the end of column address
	LCD_WriteData(x_end & 0xff);
	LCD_WriteIndex(0X2B);  //column address set 
	LCD_WriteData(y_start >> 8);     //set the head of column address
	LCD_WriteData(y_start & 0xff);
	LCD_WriteData(y_end >> 8);     //set the end of column address
	LCD_WriteData(y_end & 0xff);
  
  	if(0){//4???
		u16 ModeReg=LCD_ReadReg(0x0003);

		if(yIncFrist)
			ModeReg|=(0x8);
		else
			ModeReg&=(~0x8);
		LCD_WriteReg(0x0003, ModeReg);
	}
}

/*************************************************
函数名：LCD_Set_XY_Addr_Direction
功能：设置lcd读或写自增的方向
入口参数：0:由0到高，1:由高到0
返回值：无
*************************************************/
static void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection,bool yIncFirst)
{
	u16 Mode;

	LCD_WriteIndex(RDDMADCTL);
	LCD_ReadData();
	Mode = LCD_ReadData();
	Mode &=0x1F;
	
	switch(xyDirection)
	{
		case xInc_yInc:
			Mode|=0x00;
			break;
		case xInc_yDec:
			Mode|=0x80;
			break;
		case xDec_yInc:
			Mode|=0x40;
			break;
		case xDec_yDec:
			Mode|=0xC0;
			break; 
	}
	
	if(yIncFirst)
		Mode |=0x20;
		
	LCD_WriteIndex(MADCTL);
	LCD_WriteData(Mode); 
}


/*************************************************
函数名：LCD_BGR_Mode
功能：设置lcd RGB顺序
入口参数：0:RGB   1:BGR
返回值：无
*************************************************/
void LCD_BgrMode(bool UseBGR)
{
	u8 Reg;
	
	LCD_WriteIndex(0x36);

	Reg=LCD_ReadData();
	//Debug("Mo:%x\n\r",Reg);
	
	if(UseBGR)
		LCD_WriteData(0x08|Reg);
	else
		LCD_WriteData(0x00|Reg);

		//Debug("Mo:%x\n\r",LCD_ReadData());
}

/*************************************************
函数名：LCD_Addr_Inc
功能：地址自增
入口参数：无
返回值：无
*************************************************/
void LCD_AddrInc(void)//4????
{
	register u16 Color16;
	//LCD_WriteIndex(GRAMRD);
	LCD_ReadData();
	Color16=LCD_ReadData();
	//LCD_WriteIndex(GRAMWR);
	LCD_WriteData((((Color16>>11)&0x001f)|(Color16&0x07e0)|((Color16<<11)&0xf800)));//将16位RGB(565)色彩换算成16位BGR(565)色彩
}

/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 Data)
{
	LCD_WriteIndex(GRAM_XADDR);
	LCD_WriteData(x>>8);
	LCD_WriteData(x&0xFF);
	LCD_WriteIndex(GRAM_YADDR);
	LCD_WriteData(0x00);
	LCD_WriteData(y&0xFF);
	LCD_WriteIndex(GRAMWR);   
	LCD_WriteData(Data);  
}

/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
u16 LCD_ReadPoint(u16 x,u16 y)
{
	register u16 Data;
	LCD_WriteIndex(GRAM_XADDR);
	LCD_WriteData(x>>8);
	LCD_WriteData(x&0xFF);
	LCD_WriteIndex(GRAM_YADDR);
	LCD_WriteData(0x00);
	LCD_WriteData(y&0xFF);
	LCD_WriteIndex(GRAMRD);   
	LCD_ReadData();
	Data=LCD_ReadData();
	Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB换序
	//4???? LCD_WriteData(Data);//如果读操作能自增，则无需此句
	return Data;
}

/*************************************************
函数名：LCD_Light_Set
功能：LCD设置背光亮度
入口参数：Scale:0-100，0为熄灭，100最亮
返回值：无
*************************************************/
void LCD_Light_Set(u8 Scale)
{
	Debug("LCD Light Set:%d\n\r",Scale);
#if LCD_BGLIGHT_PWM_MODE
	Tim3_PWM(Scale);
#else
	if(Scale) GPIO_SetBits(GPIOC,GPIO_Pin_6);
	else GPIO_ResetBits(GPIOC,GPIO_Pin_6);
#endif
	gLcdScale=Scale;
}

/*************************************************
函数名：LCD_Light_State
功能:查询当前背光亮度
入口参数：无
返回值：0-100，0为熄灭，100最亮
*************************************************/
u8 LCD_Light_State(void)
{
	return gLcdScale;
}

/**********************************************
函数名：FSMC_LCD_Init
功能：用于FSMC配置
入口参数：无
返回值：无
***********************************************/
static void LCD_FSMC_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE); 
  
  p.FSMC_AddressSetupTime = 0x02;
  p.FSMC_AddressHoldTime = 0x00;
  p.FSMC_DataSetupTime = 0x05;
  p.FSMC_BusTurnAroundDuration = 0x00;
  p.FSMC_CLKDivision = 0x00;
  p.FSMC_DataLatency = 0x00;
  p.FSMC_AccessMode = FSMC_AccessMode_B;
    
  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;	  

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}

/**********************************************
函数名：LCD_Configuration
功能：用于LCD的IO配置
入口参数：延时数
返回值：无
***********************************************/
static void LCD_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE, ENABLE); 	

  //驱动ic供电控制
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //LCD Reset
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

#if LCD_BGLIGHT_PWM_MODE //LCD 背光控制,	  
  /*配置PC6 PWM*/
  //pc6属于tim3 ch1的remap
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //TIM3 完全重映射 
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

  //FSMC的GPIOD管脚
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_14 	//D0
  												| GPIO_Pin_15 		//D1
  												| GPIO_Pin_0		//D2
  												| GPIO_Pin_1		//D3
  												| GPIO_Pin_8 		//D13
  												| GPIO_Pin_9 		//D14
  												| GPIO_Pin_10 		//D15  		
  												| GPIO_Pin_7		//NE1
  												| GPIO_Pin_11		//RS
  												| GPIO_Pin_4		//nRD
  												| GPIO_Pin_5;		//nWE
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  //FSMC的GPIOE管脚
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_7		//D4
  												| GPIO_Pin_8 		//D5
  												| GPIO_Pin_9 		//D6
  												| GPIO_Pin_10 		//D7
  												| GPIO_Pin_11 		//D8
  												| GPIO_Pin_12 		//D9
  												| GPIO_Pin_13		//D10
  												| GPIO_Pin_14 		//D11
  												| GPIO_Pin_15; 	//D12  												
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

/*************************************************
函数名：LCD_Init
功能：初始化启动lcd
入口参数：无
返回值：无
*************************************************/
void LCD_Init(void)
{
	LCD_FSMC_Init();
	LCD_DelayMs(100);	 
	LCD_Configuration();
	LCD_ILI9320_On();
	LCD_DelayMs(100);
	LCD_Light_Set(100);
	LCD_PowerOn();
	LCD_BgrMode(FALSE);

	LCD_SetAddrIncMode(xDec_yInc,TRUE);
}

void LCD_DeInit(void)
{
	LCD_PowerOff();
}

/**********************************************
函数名：LCD_Reset
功能：LCD复位
入口参数：延时数
返回值：无
***********************************************/ 
void LCD_Reset(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
    LCD_DelayMs(50);					   
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 	 
	LCD_DelayMs(500);	
}


