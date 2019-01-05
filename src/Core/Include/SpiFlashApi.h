#ifndef SPI_FLASH_API_H
#define SPI_FLASH_API_H

#include "SpiFlash_w25q.h"

#define SpiFlsReadData(Addr,Len,pBuf) W25Q_Read_Data(Addr,Len,pBuf)
#define SpiFlsWriteData(Addr,Len,pBuf) W25Q_Program(Addr,Len,pBuf)
#define SpiFlsEraseSector(Sector,Num) W25Q_Sector_Erase(Sector,Num)
#define SpiFlsEraseMinSec(Addr) W25Q_Erase(Addr,1) // 4k小块擦除
#define SpiFlsEraseChip() W25Q_Bulk_Erase()
#define SpiFlsInit() W25Q_Init()
//#define SpiFlsWriteEn() W25Q_Write_Enable()
//#define SpiFlsBusy() W25Q_Busy()

#define FLASH_SECTOR_PAGE_NUM 	256//每个扇区256页
#define FLASH_PAGE_SIZE 					256//每个页256字节
#define FLASH_PAGE_TOTLA					65536//总共页数
#define FLASH_SECTOR_BYTES 			(FLASH_SECTOR_PAGE_NUM*FLASH_PAGE_SIZE)//每个扇区字节数
#define FLASH_SECTOR_TOTAL 			256//一共256个64k扇区
#define FLASH_MIN_SEC_BYTES			4096// 4k小扇区

//flash布局
//flash 布局,64k一个扇区，w25q128一共256个扇区，65536页，可通过WMP_LOW_32C将低8个扇区保护起来

#define FM_IAP_BIN_BASE_SECTOR 0
#define FM_IAP_BIN_SEC_NUM 16

#define FM_OEM2UNI_TABLE_BASE_SECTOR 16
#define FM_OEM2UNI_TABLE_SEC_NUM 2

#define FM_UNI2OEM_TABLE_BASE_SECTOR 18
#define FM_UNI2OEM_TABLE_SEC_NUM 2

#define FM_DATABASE_BASE_SECTOR 24
#define FM_DATABASE_SEC_NUM 8

#define FM_INFOSAVE_BASE_SECTOR 32
#define FM_INFOSAVE_SEC_NUM 96

#endif
