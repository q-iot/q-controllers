#ifndef HOST_DATA_IN_HANDLER_H
#define HOST_DATA_IN_HANDLER_H


void AppButtonActive(u8 Button);

void WDevConfigHandler(WCON_PACKET *pConPkt,u32 SrcAddr,u32 DstAddr);
void HostCmdHandler(WCON_PACKET *pCmdPkt,u32 SrcAddr,u32 DstAddr);





#endif

