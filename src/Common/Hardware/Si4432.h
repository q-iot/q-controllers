#ifndef SI4432_H
#define SI4432_H


void Si4432_Deinit(void);
void Si4432_Init(u32 MyAddr,u8 RssiThred);
void Si4432_SetChannel(u16 Channel);
u8 Si4432_GetRssi(void);
void Si4432_SetRssiThred(u8 RssiThred);
u8 Si4432_GetRssiThred(void);
bool Si4432_TxPacket(u32 TxAddr,u8 *Buf,u8 Len);
void Si4432_SetTxPower(u8 Val);
void Si4432_TestPacket(void);
bool Si4432_ChannelBusy(void);
void Si4432_ISR(void);

void Si4432_IntoOokMode(void);
void Si4432_LeaveOokMode(void);



#endif

