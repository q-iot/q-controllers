//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了dht11驱动，可被开发者用于其他stm32项目，减少代码开发量
*/
//------------------------------------------------------------------//

#include "SysDefines.h"

#define DHT_GPIO_RCC RCC_APB2Periph_GPIOA 
#define DHT_GPIO_GROUP  GPIOA
#define DHT_GPIO_PIN GPIO_Pin_0

#define DHT_PIN_1() GPIO_SetBits(DHT_GPIO_GROUP,DHT_GPIO_PIN)
#define DHT_PIN_0() GPIO_ResetBits(DHT_GPIO_GROUP,DHT_GPIO_PIN)
#define DHT_PIN_READ() GPIO_ReadInputDataBit(DHT_GPIO_GROUP,DHT_GPIO_PIN)

static void Dht_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin = DHT_GPIO_PIN;    
	GPIO_Init(DHT_GPIO_GROUP, &GPIO_InitStructure);	
}

static void Dht_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_InitStructure.GPIO_Pin = DHT_GPIO_PIN;    
	GPIO_Init(DHT_GPIO_GROUP, &GPIO_InitStructure);
	DHT_PIN_1();
}

#define DHT_WAIT_CNT 200
#define DHT_HIGH_PLUSE_CNT 100
bool Dht_Read(u16 *pTemp,u16 *pHumidity)
{
	//static bool First=TRUE;
	u32 i=0;
	u8 Sum=0,Idx=0;
	u32 Data=0;
	u8 *pBytes=(void *)&Data;
	bool Ret=FALSE;

	/*if(First)
	{
		RCC_AHB1PeriphClockCmd(DHT_GPIO_RCC, ENABLE);
		First=FALSE;
	}*/
	
	if(pTemp==NULL || pHumidity==NULL) return FALSE;
	*pTemp=0;
	*pHumidity=0;
	
	Dht_Output();
	DHT_PIN_0();
	DelayMs(20);//开始信号必须大于18ms
	DHT_PIN_1();

	//OS_EnterCritical();
	Dht_Input();
	i=0;
	while(DHT_PIN_READ()==1) //等待低电平响应信号，80us超时退出
		if(i++>DHT_WAIT_CNT) goto over;//每us，i增加6.5左右

	i=0;
	while(DHT_PIN_READ()==0)//等待响应信号结束
		if(i++>DHT_WAIT_CNT) goto over;

	i=0;	
	while(DHT_PIN_READ()==1)//等待数据低电平
		if(i++>DHT_WAIT_CNT) goto over;
		
	//开始读取数据，此时是高电平
	for(Idx=0;Idx<40;Idx++)
	{
		i=0;
		while(DHT_PIN_READ()==0) if(i++>DHT_WAIT_CNT) goto over;//等待数据高电平

		i=0;
		while(DHT_PIN_READ()==1)//计算高电平的时间是28us还是70us 
		{
			if(i++>DHT_WAIT_CNT)
			{
				goto over;
			}
		}

		if(i>DHT_HIGH_PLUSE_CNT)
		{
			if(Idx<32) SetBit(Data,31-Idx);
			else SetBit(Sum,7-(Idx-32));
		}
	}

	i=0;
	while(DHT_PIN_READ()==0) if(i++>DHT_WAIT_CNT) goto over;//等待高电平,等待50us结束信号结束

	//计算校验和
	if(pBytes[0]+pBytes[1]+pBytes[2]+pBytes[3]==Sum)
	{
		*pTemp=LBit16(Data)>>8;
		*pHumidity=HBit16(Data)>>8;
		Ret=TRUE;
	}
	
over:
	//OS_ExitCritical();
	Dht_Output();

	/*if(Ret) 
	{
		Debug("%x %x %x %x = %x\n\r",pBytes[0],pBytes[1],pBytes[2],pBytes[3],Sum);
		Debug("DHT:%u.%u %u.%u\n\r",HBit8(*pTemp),LBit8(*pTemp),HBit8(*pHumidity),LBit8(*pHumidity));	
	}*/
	
	return Ret;
}



