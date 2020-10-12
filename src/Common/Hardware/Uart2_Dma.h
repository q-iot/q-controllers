#ifndef UART_DMA_H
#define UART_DMA_H


void Com2_DmaConfig(void);
u16 Com2_Send_Dma(u8 *,u16);
void Com2_Tx_TC_ISR(void);
void Com2_Rx_IDLE_ISR(void);



#endif

