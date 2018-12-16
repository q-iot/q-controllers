#ifndef QSYS_PUBLIC_FUNC_H
#define QSYS_PUBLIC_FUNC_H


//2									公用函数	(PublicFunc.c)						

//字符串转换为整形
//pStr必须是十进制的数字，如果不符合要求，返回0
u32 StrToUint(const u8 *pStr);

//有符号转换
//pStr，如果不符合要求，返回-1
//支持十进制,十六进制,二进制
//忽略空格，遇到不正确字符则退出
//第一个字符是数字或者负号
s32 StrToSint(const u8 *pStr);

//浮点型转换为整型，四舍五入
int FloatToInt(float f);

//浮点型转ieee标准
u32 Float2Ieee(float f_num);

//ieee标准转浮点型
float Ieee2Float(u32 b_num);

#if 1
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

//比较两个内存块中字节是否相同，返回TRUE表示相同，Len指定比较长度
bool CompareBuf(u8 *Buf1,u8 *Buf2,u16 Len);

//打印指定长度的字符串
void DisplayStrN(const u8 *Buf,u16 Len);

//打印buf内容
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16_Dec(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU32(const u32 *Buf,u16 Len,u8 RawLen);

bool FuzzyEqual(u32 A,u32 B,u8 Tole);

bool Str2Ip(const u8 *pStr,u8 *Ip);
char *Ip2Str(void *pIp);

void StrToLower(u8 *pStr);

bool IsNullStr(u8 *pStr);
bool NotNullStr(u8 *p);

u16 StrnCmp(const u8 *pStr1,const u8 *pStr2,u16 Bytes);
u8 *ChkStr(const u8 *pStr1, const u8 *pStr2 );
u8 *FindStr(u8 *pStr,u8 *pStrStart,u8 *pStrEnd);

u32 AlignTo4(u32 v);
bool IsAlign4(u32 v);
u32 AlignTo8(u32 v);
bool IsAlign8(u32 v);

u32 MakeHash33(u8 *pData,u32 Len);

u32 CheckSum(const u8 *pData,u32 Len);

u16 CRC16(const u8 *pData,u16 Len);

u16 Rev16(u16 Data);

u32 Rev32(u32 Data);

u32 Rand(u32 Mask);

u32 GetHwID(u8 *pID);

u32 GenerateDefPw(void);
u32 GetDutRegSn(void);
u8 *GetMacAddress(void);

void PrintChineseCharToCode(const u8 *pChineseStr);
u32 SaveCpuStatus(void);
void RestoreCpuStatus(u32);

#if 1
#define IntSaveInit() u32 cpu_sr=0;
#define EnterCritical() cpu_sr=SaveCpuStatus()
#define LeaveCritical() RestoreCpuStatus(cpu_sr)
#else
#define IntSaveInit() 
#define EnterCritical() __set_PRIMASK(1);
#define LeaveCritical() __set_PRIMASK(0);
#endif

#endif

