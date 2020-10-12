#include "Drivers.h"
#include "Q_Heap.h"
#include "EventInHandler.h"

#define US2_TX_LEN 512
#define US2_RX_LEN 512
static u8 US2_TX_BUF[US2_TX_LEN];
static u8 US2_RX_BUF[US2_RX_LEN];

static volatile u16 gSendNumA=0;
static volatile u16 gSendNumB=0;

void Com2_DmaConfig(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  /* DMA1 Channel6 (triggered by USART2 Rx event) Config */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

  /* DMA1 Channel6 (triggered by USART2 Rx event) Config */
  DMA_DeInit(DMA1_Channel6);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);// 初始化外设地址，相当于“哪家快递”
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)US2_RX_BUF;// 内存地址，相当于几号柜
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为数据来源，即为收快递
  DMA_InitStructure.DMA_BufferSize = US2_RX_LEN ;// 缓存容量，即柜子大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // 外设地址不递增，即柜子对应的快递不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;// 内存递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设字节宽度，即快递运输快件大小度量（按重量算，还是按体积算）
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;// 内存字节宽度，即店主封装快递的度量(按重量，还是按体质进行封装)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// 正常模式，即满了就不在接收了，而不是循环存储
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;// 优先级很高，对应快递就是加急
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 内存与外设通信，而非内存到内存
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);// 把参数初始化，即拟好与快递公司的协议
  DMA_Cmd(DMA1_Channel6, ENABLE);// 启动DMA，即与快递公司签订合同，正式生效

  /* DMA1 Channel7 (triggered by USART2 Tx event) Config */
  DMA_DeInit(DMA1_Channel7);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);  // 外设地址，串口1， 即发件的快递
  DMA_InitStructure.DMA_MemoryBaseAddr =(u32)US2_TX_BUF;// 发送内存地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;// 外设为传送数据目的地，即发送数据，即快递是发件
  DMA_InitStructure.DMA_BufferSize = 0;  //发送长度为0，即未有快递需要发送
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);//初始化

  USART_DMACmd(USART2, USART_DMAReq_Tx|USART_DMAReq_Rx, ENABLE);// 使能DMA串口发送和接受请求
}

//清除DMA 缓存，并终止DMA
static void Com2_Dma_Clr(void)
{
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel7,0);
    
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel6,US2_RX_LEN);
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

//发送数据
u16 Com2_Send_Dma(u8 *pData, u16 Len)
{
	if(Len==0 || pData==NULL) return 0;//判断长度是否有效

	while(DMA_GetCurrDataCounter(DMA1_Channel7));// 检查DMA发送通道内是否还有数据
	USART_ITConfig(USART2,USART_IT_TC,ENABLE);//使能串口发送完成中断
	MemCpy(US2_TX_BUF, pData,(Len>US2_TX_LEN?US2_TX_LEN:Len));
	DMA_Cmd(DMA1_Channel7,DISABLE);//DMA发送数据-要先关 设置发送长度 开启DMA
	DMA_SetCurrDataCounter(DMA1_Channel7,Len);// 设置发送长度
	DMA_Cmd(DMA1_Channel7, ENABLE);  // 启动DMA发送
	return Len;
}

//发送完成中断
void Com2_Tx_TC_ISR(void)
{
	DMA_Cmd(DMA1_Channel7, DISABLE); // 关闭DMA
	DMA_SetCurrDataCounter(DMA1_Channel7,0);// 清除数据长度
	USART_ITConfig(USART2,USART_IT_TC,DISABLE);//关闭串口发送完成中断
}

//接收空闲函数
void Com2_Rx_IDLE_ISR(void)
{
	int Len=0;
	u8 *pBuf=NULL;
	
	Len=US2_RX_LEN-DMA_GetCurrDataCounter(DMA1_Channel6);// 总的buf长度减去剩余buf长度，得到接收到数据的长度
	pBuf=(u8 *)Q_Malloc(Len+2);
	MemCpy(pBuf,US2_RX_BUF,Len);
	
	Com2_Dma_Clr();
	SendEvent(EBF_RS_COM_CMD,Len,pBuf);
}

