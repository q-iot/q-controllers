//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件封装了stm32内置rom的读写函数，可用于其他stm32项目，减少开发量
*/
//------------------------------------------------------------------//

#include "SysDefines.h"
#include "Product.h"

#define ROM_ADDR_START 0x08000000

#if PRODUCT_IS_FLY_0XX
#define ROM_ADDR_MAX 0x08080000
#elif PRODUCT_C8_SUPPORT
#define ROM_ADDR_MAX 0x08010000
#else //stm32f103cb
#define ROM_ADDR_MAX 0x08020000
#endif

//打印指定页
void DebugRomPage(u32 PageNum)
{
	IntSaveInit();
	EnterCritical();//关闭所有中断，防止由于分配大数组导致的栈溢出
	if(PageNum<ROM_FLASH_SIZE/ROM_FLASH_PAGE_SIZE)
	{
		u32 CodeBuf[ROM_FLASH_PAGE_U32_NUM];

		Rom_ReadPage(CodeBuf,IAP_ROM1_ADDR+PageNum*ROM_FLASH_PAGE_SIZE,sizeof(CodeBuf));
		DisplayBufU32(CodeBuf,ROM_FLASH_PAGE_U32_NUM,8);
	}
	LeaveCritical();
}

//读出一页
//三个参数必须都能被4整除
bool Rom_ReadPage(void *Buf,u32 Address,u32 Len)
{
	u32 i;
	u32 *pB=Buf;
	__IO u32 *p=(void *)(Address);

	if(Buf==NULL) return FALSE;

	if((u32)Buf%4!=0 || Address%4!=0 || Len%4!=0) //参数对齐检查
		return FALSE;
		
	if( Address<ROM_ADDR_START || (Address+ROM_FLASH_PAGE_SIZE)>ROM_ADDR_MAX ) //参数范围检查
		return FALSE;

	Len=Len>>2;
	if(Len>ROM_FLASH_PAGE_U32_NUM) Len=ROM_FLASH_PAGE_U32_NUM;

	for(i=0;i<Len;i++) pB[i]=p[i];
	
	return TRUE; 
}

//写入一页，请在临界区执行
//三个参数必须都能被4整除
bool Rom_WritePage(void *Buf,u32 Address,u32 Len)
{
	u32 i;
	FLASH_Status Status;

	if(Buf==NULL) return FALSE;
	
	if((u32)Buf%4!=0 || Address%4!=0 || Len%4!=0) //参数对齐检查
		return FALSE;
		
	if( Address<ROM_ADDR_START || (Address+ROM_FLASH_PAGE_SIZE)>ROM_ADDR_MAX ) //参数范围检查
		return FALSE;

	DebugCol(" ");
	FLASH_Unlock();

	Status=FLASH_ErasePage(Address);
	if(Status!=FLASH_COMPLETE)//檫除
	{
		FLASH_Lock();
		DebugCol("X"); 
		Debug("FlashStatus(%x):%d\n\r",Address,Status);
		while(1);
//		return FALSE;
	}

	Len=Len>>2;
	if(Len>ROM_FLASH_PAGE_U32_NUM) Len=ROM_FLASH_PAGE_U32_NUM;

	for(i=0;i<Len;i++)
	{
		if(FLASH_ProgramWord(Address+4*i, ((u32 *)Buf)[i] ) != FLASH_COMPLETE) 
		{
			FLASH_Lock();  
			return FALSE;
		}
	}
	
	FLASH_Lock();  
	return TRUE; 
}

bool Rom_ErasePage(u32 PageNum)
{
	u32 Address=IAP_ROM1_ADDR+PageNum*ROM_FLASH_PAGE_SIZE;
	FLASH_Status Status;

	if( Address<ROM_ADDR_START || (Address+ROM_FLASH_PAGE_SIZE)>ROM_ADDR_MAX ) //参数范围检查
		return FALSE;
		
	FLASH_Unlock();

	Status=FLASH_ErasePage(Address);
	if(Status!=FLASH_COMPLETE)//檫除
	{
		FLASH_Lock();
		DebugCol("X"); 
		Debug("FlashStatus(%x):%d\n\r",Address,Status);
		return FALSE;
	}

	FLASH_Lock();
	
	return TRUE;
}

