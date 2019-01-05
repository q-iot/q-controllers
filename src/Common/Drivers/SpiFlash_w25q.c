#include "SysDefines.h"

//OS_SEM_T gSpiFlsMutex;
#define FlsMutexCreat() //gSpiFlsMutex=OS_SemCreateMutex()
#define FlsMutexRequest() //OS_SemTake(gSpiFlsMutex,OS_MAX_DELAY)
#define FlsMutexRelease() //OS_SemGive(gSpiFlsMutex)
#define WDG_FEED() IWDG_ReloadCounter()

#define Flash_SpiInit() SPI2_Init()
#define Select_Flash()     IOOUT_SetIoStatus(IOOUT_FLASH_CS,FALSE);
#define NotSelect_Flash()    IOOUT_SetIoStatus(IOOUT_FLASH_CS,TRUE);
#define WriteEn_Flash() 	IOOUT_SetIoStatus(IOOUT_FLASH_WP,TRUE);
#define WriteDis_Flash() 	IOOUT_SetIoStatus(IOOUT_FLASH_WP,FALSE);
#define SPI_Flash_Read() SPI_ReadByte(SPI2)
#define	SPI_Flash_Write(x) SPI_WriteByte(SPI2,x)

//返回true表示忙
bool W25Q_Busy(void)
{
	return (W25Q_Read_Status_Reg()&Bit(BIT_SREG_BUSY))?TRUE:FALSE;
}

//开启后才能进行写状态寄存器，页编程及擦除功能，写及擦除指令完成后，会自动disable
void W25Q_Write_Enable(void)
{
	Select_Flash();	
	WriteEn_Flash();
	SPI_Flash_Write(WRITE_ENABLE);	
	NotSelect_Flash();
}

void W25Q_Write_Disable(void)//因为会自动disable，所以不常用此函数
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_DISABLE);	
	WriteDis_Flash();
	NotSelect_Flash();
}

u32 W25Q_Read_Id(void)
{
	u8 i;
	u8 ID[4];
	
	Select_Flash();	
	SPI_Flash_Write(JEDEC_ID);	

	for(i=0;i<4;i++)
	{
		ID[i] = SPI_Flash_Read();	
	}
	
	NotSelect_Flash();

	return (ID[0]<<24)|(ID[1]<<16)|(ID[2]<<8)|ID[3];
}

u16 W25Q_Read_Status_Reg(void)
{
	u16 state;
	
	Select_Flash();	
	SPI_Flash_Write(READ_STAUS_REG1);	
 	state=SPI_Flash_Read();	
	NotSelect_Flash();

	Select_Flash();	
	SPI_Flash_Write(READ_STAUS_REG2);	
 	state=state | (SPI_Flash_Read()<<8);	
	NotSelect_Flash();
	
	return state;
}

//设置之前，要先设置W25Q_Write_Enable
void W25Q_Write_Status_Reg(u16 state)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_STAUS_REG);	
	SPI_Flash_Write(LBit8(state));
	SPI_Flash_Write(HBit8(state));
	NotSelect_Flash();
}

//根据功能设置状态寄存器
void W25Q_Set_Status_Reg(W25Q_MEM_PROTE MP,W25Q_STA_REG_PROTE SRP)
{
	u16 state=0;

	if(ReadBit(MP,0)) SetBit(state,BIT_SREG_BP0);
	if(ReadBit(MP,1)) SetBit(state,BIT_SREG_BP1);
	if(ReadBit(MP,2)) SetBit(state,BIT_SREG_BP2);
	if(ReadBit(MP,3)) SetBit(state,BIT_SREG_TB);
	if(ReadBit(MP,4)) SetBit(state,BIT_SREG_SEC);
	if(ReadBit(SRP,0)) SetBit(state,BIT_SREG_SRP0);
	if(ReadBit(SRP,1)) SetBit(state,BIT_SREG_SRP1);

	Select_Flash();	
	SPI_Flash_Write(WRITE_STAUS_REG);	
	SPI_Flash_Write(LBit8(state));
	SPI_Flash_Write(HBit8(state));
	NotSelect_Flash();
}

//读数据，自动翻页,addr为字节地址
void W25Q_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 n;

	if(buf==NULL || len==0) return;

	FlsMutexRequest();
	Select_Flash();	
	SPI_Flash_Write(READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	
	for(n=0;n<len;n++)//一次性读出
	{
		buf[n]=SPI_Flash_Read();
	}
	NotSelect_Flash();
	FlsMutexRelease();	
}

//快速读数据，不用
void W25Q_Fast_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 n;

	if(buf==NULL || len==0) return;

	FlsMutexRequest();
	Select_Flash();	
	SPI_Flash_Write(FAST_READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	SPI_Flash_Write(0);//多加的dumy
	
	for(n=0;n<len;n++)//一次性读出
	{
		buf[n]=SPI_Flash_Read();
	}
	NotSelect_Flash();
	FlsMutexRelease();
}

//编程函数，页编程前一定要进行页擦除!!!
//原始的页编程指令最多只能写256字节，写到页边缘会自行回卷到页首
//我们需要对它进行修改，让他可以一次多写点
//完成后，WEL位会自动归零
//被SEC,TB,BP保护的地址，不会被编程
void W25Q_Program(u32 addr,u16 len,const u8 *buf)
{
	u32 i,n;
	u16 FirstBytes;
	u16 Loop;
	u16 LastBytes;

	FirstBytes=(addr&0xff)?W25Q_PAGE_SIZE-(addr&0xff):0;//防止写的时候不是从页头开始写的情况

	if(len<=FirstBytes)
	{
		FirstBytes=len;
		Loop=0;
		LastBytes=0;
	}
	else
	{
		Loop=(len-FirstBytes)/W25Q_PAGE_SIZE;
		LastBytes=(len-FirstBytes)%W25Q_PAGE_SIZE;
	}
	
	if(FirstBytes)//处理第一页
	{
		FlsMutexRequest();
		W25Q_Write_Enable();
		Select_Flash();	
		SPI_Flash_Write(PROGRAM_PAGE);	
		SPI_Flash_Write((addr>>16) & 0xff);
		SPI_Flash_Write((addr>>8) & 0xff);
		SPI_Flash_Write(addr & 0xff);

		for(i=0;i<FirstBytes;i++)
			SPI_Flash_Write(buf[i]);

		NotSelect_Flash();
		while(W25Q_Busy());	
		FlsMutexRelease();

		buf+=FirstBytes,addr+=FirstBytes;
	}
	
	for(n=0;n<Loop;n++,buf+=W25Q_PAGE_SIZE,addr+=W25Q_PAGE_SIZE)
	{
		FlsMutexRequest();
		W25Q_Write_Enable();
		Select_Flash();	
		SPI_Flash_Write(PROGRAM_PAGE);	
		SPI_Flash_Write((addr>>16) & 0xff);
		SPI_Flash_Write((addr>>8) & 0xff);
		SPI_Flash_Write(addr & 0xff);

		for(i=0;i<W25Q_PAGE_SIZE;i++)
			SPI_Flash_Write(buf[i]);

		NotSelect_Flash();
		while(W25Q_Busy());	
		FlsMutexRelease();
	}

	if(LastBytes)//最后一页
	{
		FlsMutexRequest();
		W25Q_Write_Enable();
		Select_Flash();	
		SPI_Flash_Write(PROGRAM_PAGE);	
		SPI_Flash_Write((addr>>16) & 0xff);
		SPI_Flash_Write((addr>>8) & 0xff);
		SPI_Flash_Write(addr & 0xff);

		for(i=0;i<LastBytes;i++)
			SPI_Flash_Write(buf[i]);

		NotSelect_Flash();
		while(W25Q_Busy());		
		FlsMutexRelease();
	}
}

//64k擦除
void W25Q_Sector_Erase(u8 sector,u8 num)
{	
	FlsMutexRequest();
	for(;num;num--)
	{
		W25Q_Write_Enable();
		Select_Flash();	
		SPI_Flash_Write(ERASE_BLOCK_64K);
		SPI_Flash_Write(sector++);
		SPI_Flash_Write(0);
		SPI_Flash_Write(0);
		NotSelect_Flash();
		while(W25Q_Busy())//如在操作系统中，所以此处可使用os延时
		{
			WDG_FEED();
		}
	}
	FlsMutexRelease();
}

//擦除
//type=1, 4k擦除
//type=2, 32k擦除
//type=3, 64k擦除
void W25Q_Erase(u32 addr,u8 Type)
{
	FlsMutexRequest();
	W25Q_Write_Enable();
	Select_Flash();	
	if(Type==1) SPI_Flash_Write(ERASE_SECTOR_4K);	
	else if(Type==2) SPI_Flash_Write(ERASE_BLOCK_32K);
	else SPI_Flash_Write(ERASE_BLOCK_64K);
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	NotSelect_Flash();
	while(W25Q_Busy())//如在操作系统中，所以此处可使用os延时
	{
		WDG_FEED();
	}
	FlsMutexRelease();
}

//全片擦除
void W25Q_Bulk_Erase(void)
{
	FlsMutexRequest();
	W25Q_Write_Enable();
	Select_Flash();	
	SPI_Flash_Write(ERASE_CHIP);	
	NotSelect_Flash();

	while(W25Q_Busy())//如在操作系统中，所以此处可使用os延时
	{
		WDG_FEED();
	}
	FlsMutexRelease();
}

void W25Q_Deep_Power_Down(void)
{
	u32 i;
	Select_Flash();	
	SPI_Flash_Write(DEEP_POWER_DOWN);	
	NotSelect_Flash();
	for(i=5000;i;i--);
}

u8 W25Q_Wake_Up(void)
{
	u8 res;
	u32 i;
	
	Select_Flash();	
	SPI_Flash_Write(WAKE_UP);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	res=SPI_Flash_Read();
	
	NotSelect_Flash();
    for(i=5000;i;i--);
    
	return res;
}

//返回可供操作的存储大小
u32 W25Q_Init(void)
{
	//u32 ID;
	
	Flash_SpiInit();
	NotSelect_Flash(); 

	FlsMutexCreat();
	
	switch(W25Q_Read_Id())
	{
		case 0xef401500:
			Debug("W25Q16:2M Byte\n\r");
			return (2*1024*1024);
		case 0xef401600:
			Debug("W25Q32:4M Byte\n\r");
			return (4*1024*1024);
		case 0xef401700:
			Debug("W25Q64:8M Byte\n\r");
			return (8*1024*1024);
		case 0xef401800:
			Debug("W25Q128:16M Byte\n\r");
			return (16*1024*1024);
		default:
			Debug("SPI Flash UnKnow! %x\n\r",W25Q_Read_Id());
			return (0);
	}
}


//wp引脚，起到的作用和W25Q_Write_Enable是完全相似的，所以单单靠置低wp管脚，无法做到保护指定扇区
//写状态寄存器为WMP_ALL，WSRP_HARD_PROTE，然后再置低wp，才能起到保护扇区的目的
//wp引脚的目的，可能是为了防止spi总线上的干扰，所以用wp做额外的一层保护
//或者板卡上wp直接拉低，然后设置了flash之后，flash就变成完全只读的芯片
void W25Q_Protect(bool Enable)
{
	W25Q_Write_Enable();
	if(Enable) W25Q_Set_Status_Reg(WMP_ALL,WSRP_SOFT_PROTE);//wp脚也不过是硬件上的状态寄存器写保护，无法用单纯wp脚做到指定扇区写保护的开启或关闭
	else W25Q_Set_Status_Reg(WMP_NONE,WSRP_SOFT_PROTE);
	W25Q_Write_Disable();
}

bool W25Q_IsProtect(void)
{
	u16 Reg=W25Q_Read_Status_Reg();

	if(Reg&Bit(BIT_SREG_BP0)) return TRUE;
	if(Reg&Bit(BIT_SREG_BP1)) return TRUE;
	if(Reg&Bit(BIT_SREG_BP2)) return TRUE;
	if(Reg&Bit(BIT_SREG_TB)) return TRUE;
	if(Reg&Bit(BIT_SREG_SEC)) return TRUE;

	return FALSE;
}

//如果板子上wp直接接地，那么一旦调用此函数，flash将变成只读
//如果板子上wp可以软件控制，那么一旦调用此函数锁定片子后，可以通过拉高wp，再调用W25Q_Protect(FALSE)来解锁。
void W25Q_SetOnlyRead(void)
{
	W25Q_Write_Enable();
	W25Q_Set_Status_Reg(WMP_ALL,WSRP_HARD_PROTE);
	W25Q_Write_Disable();
}

