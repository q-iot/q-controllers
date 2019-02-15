#ifndef __SPI_H__
#define __SPI_H__

#define SPI_ReadByte(SPIx)	SPI_WriteByte(SPIx,0)
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte);
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet);
void SPI1_Init(void);
void SPI2_Init(void);
void SPI3_Init(void);

#define SPIv_ReadByte() SPIv_WriteByte(0)
u8 SPIv_WriteByte(u8 Byte);
void SPIv_Init(void);

#endif

