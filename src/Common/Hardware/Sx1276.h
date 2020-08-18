#ifndef SX1276_H
#define SX1276_H

void Sx1276_Deinit(void);
void Sx1276_Init(u32 MyAddr,u8 RssiThred);
void Sx1276_SetChannel(u16 Channel);
u8 Sx1276_GetRssi(void);
void Sx1276_SetRssiThred(u8 RssiThred);
u8 Sx1276_GetRssiThred(void);
bool Sx1276_TxPacket(u32 TxAddr,u8 *Buf,u8 Len);
void Sx1276_SetTxPower(u8 Val);
void Sx1276_TestPacket(void);
bool Sx1276_ChannelBusy(void);
void Sx1276_ISR(void);

void Sx1276_IntoOokMode(void);
void Sx1276_LeaveOokMode(void);

#endif

