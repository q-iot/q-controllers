#ifndef __Q_PUBLIC_FUNC_H__
#define __Q_PUBLIC_FUNC_H__


//2									公用函数	(PublicFunc.c)						

#if 1
void MemSet(void *Dst,u8 C,u32 Byte);
void MemCpy(void *Dst,const void *Src,u32 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

void DisplayStrN(const char *Buf,u16 Len);//打印指定长度的字符串
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);//打印buf内容
void DisplayBufU16_Dec(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU32(const u32 *Buf,u16 Len,u8 RawLen);
bool CompareBuf(const u8 *Buf1,const u8 *Buf2,u16 Len);//比较两个内存块中字节是否相同，返回TRUE表示相同，Len指定比较长度
bool FuzzyEqual(u32 A,u32 B,u8 Tole);

u32 HexStr2Uint(const char *pStr);//将不带前缀的十六进制数字字符串转化为整形       
u32 Str2Uint(const char *pStr);//字符串转换为整形
s32 Str2Sint(const char *pStr);//有符号转换
int Float2Int(float f);//浮点型转换为整型，四舍五入
u32 Float2Ieee(float f_num);//浮点型转ieee标准
float Ieee2Float(u32 b_num);//ieee标准转浮点型
char *Ip2Str(void *pIp);
bool Str2Ip(const char *pStr,u8 *Ip);
void StrChrRep(char *pStr,char Orgc,char Repc);
const char *FindNumFromStr(const char *pStr,s32 *pNumRet);
void Str2Lower(char *pStr);
bool IsNullStr(char *pStr);
bool NotNullStr(char *p);
u16 StrnCmp(const char *pStr1,const char *pStr2,u16 Bytes);
u16 StrnCpy(char *pDst,const char *pSrc,u16 Bytes);
char *ChkStr(const char *pStr1, const char *pStr2 );
char *FindStr(char *pStr,char *pStrStart,char *pStrEnd);

u32 AlignTo4(u32 v);
bool IsAlign4(u32 v);
u32 AlignTo8(u32 v);
bool IsAlign8(u32 v);

u32 MakeHash33(const u8 *pData,u32 Len);
u32 CheckSum(const u8 *pData,u32 Len);
u16 CRC16(const u8 *pData,u16 Len);
void PrintChineseCharToCode(const u8 *pChineseStr);
u16 Rev16(u16 Data);
u32 Rev32(u32 Data);
u32 Rand(u32 Mask);



#endif

