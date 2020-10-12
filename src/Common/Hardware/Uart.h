#ifndef __UART_H__
#define __UART_H__

#include "stm32f10x.h"
#include <stdio.h>

//#define debug(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#define Debug(format,...) printf(format,##__VA_ARGS__)  
#define DebugCol(format,...) printf("\033[7m");printf(format,##__VA_ARGS__);printf("\033[0m");

void COM1_Init(u32 bound);
void COM2_Init(u32 bound);
void Com2_Send(u16 Len,const u8 *pData);
void COM3_Init(u32 bound);
void Com3_Send(u16 Len,const u8 *pData);

#define UST_SOH  0x01
#define UST_STX  0x02
#define UST_EOT  0x04
#define UST_ACK  0x06
#define UST_NAK  0x15
#define UST_CAN  0x18
#define UST_CTRLZ 0x1A

//485œ‡πÿ
void COM4_Init(bool Enable);
void COM5_Init(void);
void SendDataCom485(USART_TypeDef* USARTx,u8 *pData,u16 Len);

#endif
