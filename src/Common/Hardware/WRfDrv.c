#include "Drivers.h"
#include "Si4432.h"
#include "Sx1276.h"

#if WRF_HW_DRV == USE_SI4432
WRF_DRV_FUNC gWRfDrv={//射频驱动的回调函数
	Si4432_Deinit,
	Si4432_Init,
	Si4432_SetChannel,
	Si4432_GetRssi,
	Si4432_SetRssiThred,
	Si4432_GetRssiThred,
	Si4432_TxPacket,
	Si4432_SetTxPower,
	Si4432_TestPacket,
	Si4432_ChannelBusy,
	Si4432_ISR,
	NULL,
	Si4432_IntoOokMode,
	Si4432_LeaveOokMode
};
#elif WRF_HW_DRV == USE_LORA//sx1278
WRF_DRV_FUNC gWRfDrv={//射频驱动的回调函数
	Sx1276_Deinit,
	Sx1276_Init,
	Sx1276_SetChannel,
	Sx1276_GetRssi,
	Sx1276_SetRssiThred,
	Sx1276_GetRssiThred,
	Sx1276_TxPacket,
	Sx1276_SetTxPower,
	Sx1276_TestPacket,
	Sx1276_ChannelBusy,
	Sx1276_ISR,
	NULL,
	Sx1276_IntoOokMode,
	Sx1276_LeaveOokMode
};
#else 
#error "No define wrf hw drv!"
#endif



