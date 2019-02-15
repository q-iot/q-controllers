#ifndef _IR_DRIVER_H_
#define _IR_DRIVER_H_

#include "Ir_Rf_Record.h"

void DisplayIrRecord(const IR_RECORD *pIrRcd);

void IrPulseIn_ISR(void);

void StartRecvIr(void);//开始等待接收红外信号
void StopRecvIr(void);//停止接收红外信号
void StartSendIr(const IR_RECORD *pIrRcd);//根据数据发射红外信号

bool CaptureRecvIr(IR_RECORD *pIr);
IR_RECORD *SetCaptureBuf(IR_RECORD *pIr);

#endif

