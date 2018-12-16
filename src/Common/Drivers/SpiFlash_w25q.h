#ifndef __SPI_FLASH_W25Q_H__
#define __SPI_FLASH_W25Q_H__  

//页大小和页数目
#define W25Q_PAGE_SIZE 		256//每页256个字节
#define W25Q_PAGE_MAX 		65536 //w25q128 一共65536个页

//功能寄存器地址
#define WRITE_ENABLE 0X06
#define WRITE_DISABLE 0X04
#define READ_STAUS_REG1 0X05
#define READ_STAUS_REG2 0x35
#define WRITE_STAUS_REG 0X01
#define PROGRAM_PAGE 0x02
#define ERASE_BLOCK_64K 0xd8
#define ERASE_BLOCK_32K 0x52
#define ERASE_SECTOR_4K 0x20
#define ERASE_CHIP 0xc7
#define PAUSE_ERASE 0x75
#define RESTORE_ERASE 0x7a
#define DEEP_POWER_DOWN 0xb9
#define FAST_MODE 0xa3

#define READ_DATA 0x03
#define FAST_READ_DATA 0x0b

#define WAKE_UP 0xab
#define DEVICE_ID 0x90
#define JEDEC_ID 0x9f
#define UNIQUE_ID 0x4b

//状态寄存器bit
#define BIT_SREG_BUSY 0 //忙信号，只读
#define BIT_SREG_WEL 1 //写锁定，只读，为1时，表示写关闭，掉电及写或擦除操作后自动恢复为0
#define BIT_SREG_BP0 2 //块保护，非易失
#define BIT_SREG_BP1 3
#define BIT_SREG_BP2 4
#define BIT_SREG_TB 5 //低顶保护选择，非易失
#define BIT_SREG_SEC 6 //扇区保护寄存器，非易失
#define BIT_SREG_SRP0 7 //状态寄存器保护，非易失
#define BIT_SREG_SRP1 8
#define BIT_SREG_QE 9 //双速spi使能
#define BIT_SREG_SUS 15 //擦除挂起

typedef enum{
	WMP_NONE=0,//无保护
	WMP_UP_32C=0x01,//高三十二分之一的空间
	WMP_UP_16C,
	WMP_UP_8C,
	WMP_UP_4C,
	WMP_UP_2C,
	WMP_LOW_32C=0x09,//低三二十分之一的空间
	WMP_LOW_16C,
	WMP_LOW_8C,
	WMP_LOW_4C,
	WMP_LOW_2C,
	WMP_TOP_4K=0x11,//高4k空间
	WMP_TOP_8K,
	WMP_TOP_16K,
	WMP_TOP_32K,
	WMP_BOT_4K=0x19,//低4k空间
	WMP_BOT_8K,
	WMP_BOT_16K,
	WMP_BOT_32K,
	WMP_ALL=0x1f//全部
}W25Q_MEM_PROTE;//sec,tb,bp的值，保护的扇区，不再允许被写入

typedef enum{
	WSRP_SOFT_PROTE=0,//软件保护，wp脚无用，使用写保护指令来保护
	WSRP_HARD_PROTE,//硬件保护，wp脚低时允许写入，并且同时要开写保护指令
	WSRP_ONCE_POWER,//到下次重新供电之前，都不再允许写入状态寄存器
	WSRP_ONE_TIME,//一次性编程，状态寄存器将被永久保护，不允许被写入
}W25Q_STA_REG_PROTE;//状态寄存器保护方法


void W25Q_Write_Enable(void);
void W25Q_Write_Disable(void);//因为会自动disable，所以不常用此函数
u32 W25Q_Read_Id(void);
u16 W25Q_Read_Status_Reg(void);
void W25Q_Write_Status_Reg(u16 state);
void W25Q_Set_Status_Reg(W25Q_MEM_PROTE MP,W25Q_STA_REG_PROTE SRP);
void W25Q_Read_Data(u32 addr,u32 len,u8 *buf);
void W25Q_Fast_Read_Data(u32 addr,u32 len,u8 *buf);
void W25Q_Program(u32 addr,u16 len,const u8 *buf);
void W25Q_Sector_Erase(u8 sector,u8 num);
void W25Q_Erase(u32 addr,u8 Type);
void W25Q_Bulk_Erase(void);
void W25Q_Deep_Power_Down(void);
u8 W25Q_Wake_Up(void);
u32 W25Q_Init(void);

void W25Q_Protect(bool Enable);
bool W25Q_IsProtect(void);
void W25Q_SetOnlyRead(void);

#endif


