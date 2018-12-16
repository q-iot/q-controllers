#ifndef TLV_CODE_H
#define TLV_CODE_H

typedef struct{
	u8 Type;
	u8 Len;
	u8 Str[2];
}TLV_DATA;

void TLV_Debug(u8 *pTlv,u16 Len);
u16 TLV_Build(u8 *pOut,u16 BufLen,SRV_VALUE_TYPE Type,u8 *ValueStr);
u8 TLV_Decode(u8 *pIn,u16 BufLen,u16 Idx,TLV_DATA *pItem);


#endif
