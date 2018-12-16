#include "Drivers.h"

#define DQ_PORT GPIOB
#define DQ_PIN GPIO_Pin_0

#define SET_OP_1WIRE		GPIO_SetBits(DQ_PORT, DQ_PIN)
#define CLR_OP_1WIRE		GPIO_ResetBits(DQ_PORT, DQ_PIN)
#define DQ_DIR_OUT		
#define DQ_DIR_IN		
#define CHECK_IP_1WIRE	GPIO_ReadInputDataBit(DQ_PORT, DQ_PIN)

#if 0//IN_DEBUG//for debug
#define PB1_0() //GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define PB1_1() //GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define PB2_0() GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define PB2_1() GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define PB3_0()//GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define PB3_1() //GPIO_SetBits(GPIOA, GPIO_Pin_4)
#else
#define PB1_0() 
#define PB1_1() 
#define PB2_0() 
#define PB2_1() 
#define PB3_0()
#define PB3_1() 
#endif

static void _delay_us(u32 i)
{
	i*=7;
    while(--i);
}

static u8 Init_18b20(void)  
{  
	u32 time = 0;  

	PB1_1();

	DQ_DIR_OUT;		//输出 
	SET_OP_1WIRE;			//H   
	_delay_us(50);
	CLR_OP_1WIRE;			//L  
	_delay_us(800);		//低电平持续480us以上  
	SET_OP_1WIRE;			//H  
	DQ_DIR_IN;		//输入  
	_delay_us(80);		//15~60us  
	
	while(CHECK_IP_1WIRE && time++)//等待18b20输入存在低脉冲
	{
		_delay_us(5);
		if(time == 200){Debug("NOIN\n\r");
			return 0;}
	}

	_delay_us(200);		//60~240us 等待存在脉冲结束
	DQ_DIR_OUT;  
	SET_OP_1WIRE;  	
	PB1_0();
	
	return 1;
}

static void Write_18b20(u8 x)  
{     
	u8 m;  
	u8 bit;
	
	DQ_DIR_OUT;
	for(m = 0; m < 8; m++)  
	{ 
		PB2_1();
		bit=x & (1 << m);
		if(bit)	//写数据了，先写低位的！ 
		{			
			CLR_OP_1WIRE;
			_delay_us(2);
			SET_OP_1WIRE; 
			_delay_us(60);		//15~60us   			
		}
		else  
		{			
			CLR_OP_1WIRE;
			_delay_us(60);		//15~60us 
			SET_OP_1WIRE; 
			_delay_us(2);		
		}		
		PB2_0();
	}  
	
#if 0//IN_DEBUG
	_delay_us(30);
#endif
}

static u8 Read_18b20()  
{      
	u8 temp=0,n;  

	//DQ_DIR_IN;
	for(n = 0; n < 8; n++)  
	{  
		PB3_1();
		
		CLR_OP_1WIRE;  
		_delay_us(2);
		SET_OP_1WIRE;  
		DQ_DIR_IN;   
		_delay_us(20); 
		if(CHECK_IP_1WIRE)  //读数据,从低位开始  
			temp |= (1 << n);  
		else  
			temp &= ~(1 << n);     
			
		DQ_DIR_OUT;  
		_delay_us(60); 
		PB3_0();
	}  
	return (temp);  
}   

s16 GetEnvTemp(void)		// 读取温度值 
{  
	u8 temph=0, templ=0;  
	u16 temp=0;
	s16 ret_temp_val=0;
	u8 ZeroFlag = 0;

	IntSaveInit();
	
	EnterCritical();
	
	Init_18b20();							// 复位18b20  
	Write_18b20(0xcc);				// 发出转换命令  
	Write_18b20(0x44);

	Init_18b20(); 
	Write_18b20(0xcc);				// 发出读命令  
	Write_18b20(0xbe);
  
	templ = Read_18b20();			// 读数据  
	temph = Read_18b20();
	
	LeaveCritical();
	
	temp = (temph<<8)|templ;

	//Debug("DSV:0x%x(%u)\n\r",temp,temp);
	
	if(temp&0x8000)
	{
		ZeroFlag = 1;
		temp=(~temp)+1;
	}
	else
	{
		ZeroFlag = 0;
	}	

	if((!ZeroFlag) && temp > 0x07d0)
	{
		temp=0x270f;//超过125度后，就显示9999
	}
	else if(ZeroFlag && temp > 0x0370)
	{
		temp=0x270f;
	}
	else
	{
		temp = (temp * 10)>>4;
	}
	
	if(ZeroFlag==1) ret_temp_val-=temp;
	else ret_temp_val=temp;
	
	return ret_temp_val; 
}

