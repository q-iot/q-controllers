//--------------------------Q Controllers---------------------------//
/*
Q-Ctrl是一套基于事件的控制器框架，类比于MCV框架，Q-Ctrl用于协调
存储（Data）、输入输出（IO）、控制器（Controller）三者的逻辑处理，
简称DIC框架。
Q-Ctrl基于stm32有大量的驱动代码可直接调用，也可以移植于其他单片机平台，
无需操作系统的支持，在遵守控制器编程规则的情况下，可处理以往需要操作系统
才能处理的复杂业务。
By Karlno 酷享科技

本文件用于一些公用的系统函数，可以供系统和页面调用，即此页函数均为可重入函数
*/
//------------------------------------------------------------------//

#include "SysDefines.h"

void MemSet(void *Dst,u8 C,u32 Byte)//字节安全，esp编译器不支持byte过大，会崩溃，很奇怪
{
	u32 n;
	u8 *p=Dst;
	
	if(Dst==NULL||Byte==0) return;

	for(n=0;n<Byte;n++)
	{
		*p++=0;
	}
}

void MemCpy(void *Dst,const void *Src,u32 Byte)//字节安全
{
	u32 n;
	u8 *pD=Dst;
	const u8 *pS=Src;
	
	if(Dst==NULL || Src==NULL || Dst==Src) return;
	if(Byte==0) return;
	
	for(n=0;n<Byte;n++)
	{
		*pD=*pS;
		pD++;pS++;
	}
}

//打印指定长度的字符串
void DisplayStrN(const char *Buf,u16 Len)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		if(Buf[i]==0) break;
		Debug("%c",Buf[i]);
	}
	Debug("\n\r");
}

//Buf 要打印的内存
//Len 要打印的字节数
//RawLen 每行要打印的字节数
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		Debug("%02x ",Buf[i]);
		if(i%RawLen==(RawLen-1)) Debug("\n\r");
	}
	if(i%RawLen!=(RawLen-1)) Debug("\n\r");
}

void DisplayBufU16(const u16 *Buf,u16 Len,u8 RawLen)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		Debug("%04x ",Buf[i]);
		if(i%RawLen==(RawLen-1)) Debug("\n\r");
	}
	if(i%RawLen!=(RawLen-1)) Debug("\n\r");
}

void DisplayBufU16_Dec(const u16 *Buf,u16 Len,u8 RawLen)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		Debug("%5u ",Buf[i]);
		if(i%RawLen==(RawLen-1)) Debug("\n\r");
	}
	if(i%RawLen!=(RawLen-1)) Debug("\n\r");
}

void DisplayBufU32(const u32 *Buf,u16 Len,u8 RawLen)
{
	int i;
	
	for(i=0;i<Len;i++)
	{
		Debug("%08x ",Buf[i]);
		if(i%RawLen==(RawLen-1)) Debug("\n\r");
	}
	if(i%RawLen!=(RawLen-1)) Debug("\n\r");
}

//比较两个内存块中字节是否相同，返回TRUE表示相同，Len指定比较长度
bool CompareBuf(const u8 *Buf1,const u8 *Buf2,u16 Len)
{
	u16 i;

	for(i=0;i<Len;i++)
	{
		if(Buf1[i]!=Buf2[i]) return FALSE;
	}

	return TRUE;
}

//模糊相等
//允许两值误差
//Tole是误差百分比
//A B不允许大于100 0000
bool FuzzyEqual(u32 A,u32 B,u8 Tole)
{
	if(A==B) return TRUE;
	if(Tole>=100) return TRUE;

	if(A>B)
	{
		if( B*100 > A*(100-Tole)) return TRUE;
	}
	else
	{
		if( A*100 > B*(100-Tole)) return TRUE;
	}
	
	return FALSE;	
}

//将不带前缀的十六进制数字字符串转化为整形
//遇到非预料字符返回0
u32 HexStr2Uint(const char *pStr)         
{
	u32 i=0,sum=0;

	if(pStr == NULL) return 0;
	
	while(pStr[i])            //当str[i]不为\0时执行循环
	{
		if(pStr[i]>='0'&&pStr[i]<='9')
		{
			sum=(sum<<4)+(pStr[i]-'0');
		}
		else if(pStr[i]>='a'&&pStr[i]<='f')
		{
			sum=(sum<<4)+(pStr[i]-'a'+10);
		}
		else if(pStr[i]>='A'&&pStr[i]<='F')
		{
			sum=(sum<<4)+(pStr[i]-'A'+10);
		}
		else
		{
			return 0;//str不正确字符
		}

		i++;
	}

	return sum;
}

//pStr，如果不符合要求，返回-1
//支持十进制,十六进制,二进制
//忽略空格，遇到不正确字符则退出
u32 Str2Uint(const char *pStr)         
{
	u32 i=0,sum=0;

	if(pStr == NULL) return 0;
	
	if(pStr[0]=='0'&&pStr[1]=='x')
	{
		i=2;
		while(pStr[i])            //当str[i]不为\0时执行循环
		{
			if(pStr[i]>='0'&&pStr[i]<='9')
			{
				sum=(sum<<4)+(pStr[i]-'0');
			}
			else if(pStr[i]>='a'&&pStr[i]<='f')
			{
				sum=(sum<<4)+(pStr[i]-'a'+10);
			}
			else if(pStr[i]>='A'&&pStr[i]<='F')
			{
				sum=(sum<<4)+(pStr[i]-'A'+10);
			}
			else if(pStr[i] == ' ')//空格不予理会
			{
			}
			else
			{
				break;//str不正确字符
			}

			i++;
		}
	}
	else if(pStr[0]=='0'&&pStr[1]=='b')
	{
		i=2;
		while(pStr[i])            //当str[i]不为\0时执行循环
		{
			if(pStr[i]>='0'&&pStr[i]<='1')
			{
				sum=(sum<<1)+(pStr[i]-'0');
			}
			else if(pStr[i] == ' ')//空格不予理会
			{
			}			
			else
			{
				break;//str不正确字符
			}

			i++;
		}
	}
	else
	{
		while(pStr[i])            //当str[i]不为\0时执行循环
		{
			if(pStr[i] == ' ')//空格不予理会
			{
			}
			else if(pStr[i]<'0'||pStr[i]>'9') 
			{
				break;//str不正确字符
			}
			else
			{
				sum=sum*10+(pStr[i]-'0');
			}
			
			i++;
		}
	}
	
	return sum;
} 

//有符号转换
//pStr，如果不符合要求，返回-1
//支持十进制,十六进制,二进制
//忽略空格，遇到不正确字符则退出
//第一个字符是数字或者负号
s32 Str2Sint(const char *pStr)
{
	if(pStr[0]=='-')
	{
		return 0-Str2Uint(&pStr[1]);
	}
	else
	{
		return Str2Uint(pStr);
	}
}

//特别注意，当运算中有浮点的数字时要把，数字后面加上一个f。
//例如表达式中有4.321参与运算。。当你不在4.321后加f时，stm32F405的片子不知道把他当做单精度float用FPU来运算，
//默认可能是当做double来运算（我不确定），运算速度还是很慢。。切记所有浮点数字后面加上f，，，，
//有时候keil会提示warning:  #1035-D: single-precision operand implicitly converted to double-precision 
//这句话的意思就是单精度运算隐式转换成了双精度运算了。这个时候就要在单精度数字后面加个f
//将浮点型转化为整形
int Float2Int(float f) 
{     
	bool minus=(f<0.0f?TRUE:FALSE); 
	int a;

	if(minus) f*=-1; 
	a=(int)f; 
	if((f-a)>=0.5f) ++a; 
	if(minus)a*=-1; 
	return a; 
}

typedef union{
   float f;
   u8 b[4];
}DT_FORM_CONVER;

//浮点型转ieee标准
u32 Float2Ieee(float f_num)
{
	u32 Temp32=0;
	u16 i;
	DT_FORM_CONVER DtformConver;

	DtformConver.f=f_num;
	for(i=0;i<4;i++)
	{
		Temp32 |= (u32)(DtformConver.b[i]<<(i*8));
	}

	return Temp32;
}

//ieee标准转浮点型
float Ieee2Float(u32 b_num)
{
	u16 i;
	DT_FORM_CONVER DtformConver;

	for(i=0;i<4;i++)
	{
		DtformConver.b[i] = (u8)(b_num>>(i*8));
	}
	return DtformConver.f;
}

static char gIpAddrStrBuf[20];
char *Ip2Str(void *pIp)//仅调试安全
{
	u8 *pIpU8=pIp;
	
	sprintf(gIpAddrStrBuf,"%d.%d.%d.%d",pIpU8[0],pIpU8[1],pIpU8[2],pIpU8[3]);

	return gIpAddrStrBuf;
}

//解析ip字符串
bool Str2Ip(const char *pStr,u8 *Ip)
{
	char Buf[32];
	u8 *pIp1Str=Buf;
	u8 *pIp2Str;
	u8 *pIp3Str;
	u8 *pIp4Str;

	if(strlen((void *)pStr)>30) return FALSE;
	strcpy((void *)Buf,(void *)pStr);

	if(pStr[0]<'0'||pStr[0]>'9') return FALSE;

	pIp2Str=FindStr(pIp1Str,".",NULL);
	if(pIp2Str == NULL) return FALSE;
	
	pIp3Str=FindStr(pIp2Str,".",NULL);
	if(pIp2Str == NULL) return FALSE;

	pIp4Str=FindStr(pIp3Str,".",NULL);
	if(pIp2Str == NULL) return FALSE;
	
	Ip[0]=Str2Uint(pIp1Str);
	Ip[1]=Str2Uint(pIp2Str);
	Ip[2]=Str2Uint(pIp3Str);
	Ip[3]=Str2Uint(pIp4Str);

	return TRUE;
}

//字符串中的字符替换
void StrChrRep(char *pStr,char Orgc,char Repc)
{
	while(*pStr)
	{
		if(*pStr==Orgc) *pStr=Repc;
		pStr++;
	}
}

//从字符串中找数字起点
//返回数字结尾位置，如果没有数字，返回null
//暂不支持负数
const char *FindNumFromStr(const char *pStr,s32 *pNumRet)
{
	const char *pRet=NULL;
	char Buf[12]={0};
	u16 i=0;
	bool IsNegNum=FALSE;//是负数
	bool Found=FALSE;//找到数字了

	if(pNumRet!=NULL) *pNumRet=0;
	if(pStr==NULL) return NULL;
	
	while(*pStr)
	{
		if(*pStr&0x80) //汉字
		{
			if(Found){pRet=pStr;break;}
			pStr+=2;
		}
		else //ascii
		{
			if(*pStr>='0' && *pStr<='9')
			{
				Found=TRUE;
				if(i<10) Buf[i++]=*pStr;
				else i++;
			}
			else if(*pStr=='-')//可能是负数
			{
				if(pStr[1]>='0' && pStr[1]<='9')//确实是负数
				{
					IsNegNum=TRUE;
				}
				else
				{
					if(Found){pRet=pStr;break;}//数字结束了
				}
			}
			else
			{
				if(Found){pRet=pStr;break;}//数字结束了
			}
			pStr++;
		}		
	}	

	if(Found && pRet==NULL) pRet=pStr;

	if(Found && pNumRet!=NULL)
	{
		u32 Num=0;
		if(i<=10) Num=Str2Uint(Buf);
		if(IsNegNum) *pNumRet=0-Num;
		else *pNumRet=0;
	}	

	return pRet;
}

//将字符串变成小写
//字符串长度不要超过255
void Str2Lower(char *pStr)
{
	u16 Len=strlen((void *)pStr);
	u16 i;
	
	for(i=0;i<=Len;i++)//命令字符串全部转小写
	{
		if(pStr[i]<0x80)//ascii
		{
			if(pStr[i]>='A' && pStr[i]<='Z')
				pStr[i]=pStr[i]+32;
		}
		else//中文
		{
			i++;
		}
	}
}

//查字符串是否空
bool IsNullStr(char *pStr)
{
	if(pStr==NULL || pStr[0]==0) return TRUE;

	return FALSE;
}

//查字符串是否空
bool NotNullStr(char *p)
{
	if(p==NULL || *p==0) return FALSE;

	return TRUE;
}

//比较字符串，支持通配符*
//相等返回0
u16 StrnCmp(const char *pStr1,const char *pStr2,u16 Bytes)
{
	u16 i;

	for(i=0;i<Bytes;i++)
	{
		if(pStr1[i]!=pStr2[i] && pStr2[i]!='*')
			return i;
	}

	return 0;
}

//字符串拷贝，最长拷贝bytes字节，含结束符
//返回拷贝的长度，不含结束符
u16 StrnCpy(char *pDst,const char *pSrc,u16 Bytes)
{
	u16 i;

	if(Bytes==0) return 0;
	Bytes--;
	
	for(i=0;i<Bytes;i++)
	{
		pDst[i]=pSrc[i];
		if(pSrc[i]==0)break;
	}
	pDst[i]=0;

	return i;
}

//从str1里面找str2，str2里允许有通配符*，但首字节不能是*
//找到返回位置，未找到返回NULL
char *ChkStr(const char *pStr1, const char *pStr2 )
{
	u16 len2=strlen((void *)pStr2);

	if(len2==0) return NULL;
	if(strlen((void *)pStr1)==0) return NULL;

	for(;*pStr1;++pStr1)
	{
		if(*pStr1==*pStr2 && StrnCmp(pStr1,pStr2,len2)==0) 
			return (char *)pStr1;
	}
	
	return NULL;
}

//根据头尾寻找字符串，并将尾部所在位置置0
//允许pStrEnd为空，当pStrEnd为空，则只找头
//返回的是去掉头字符串的头部指针
char *FindStr(char *pStr,char *pStrStart,char *pStrEnd)
{
	char *pStart,*pEnd;
	
	pStart=(void *)strstr((void *)pStr,(void *)pStrStart);
	if(pStart==NULL) return NULL;
	
	pStart+=strlen((void *)pStrStart);
	if(pStrEnd==NULL || pStrEnd[0]==0) return pStart;
	pEnd=(void *)strstr((void *)pStart,(void *)pStrEnd);
	if(pEnd==NULL) return NULL;

	pEnd[0]=0;
	return pStart;
}

//根据分隔符，拆分字符串
//idx从1开始
u16 SplitStr(char *pStr,char GapChar,u16 Idx)
{
	u16 i=0;
	
	while(pStr[i])
	{
		if(pStr[i]==GapChar)
		{
			Idx--;
			if(Idx==0) return i;
		}
	}

	return 0;
}

// 4字节对齐
u32 AlignTo4(u32 v)
{
	if(v&0x03) v=(v&~0x03)+4;
	return v;
}

//检查是否4字节对齐
bool IsAlign4(u32 v)
{
	return (v&0x03)?FALSE:TRUE;
}

// 8字节对齐
u32 AlignTo8(u32 v)
{
	if(v&0x07) v=(v&~0x07)+8;
	return v;
}

//检查是否8字节对齐
bool IsAlign8(u32 v)
{
	return (v&0x07)?FALSE:TRUE;
}

//计算hash33值
u32 MakeHash33(const u8 *pData,u32 Len)
{
	u32 hash=(u32)-1;
	u32 i=0;

 	if(pData == NULL || Len == 0) return (u32)-1;
 	
	for(;i<Len;i++)
	{
		hash+=pData[i];
		hash+=(hash<<5);
	}

	return hash;
}

//计算校验和
u32 CheckSum(const u8 *pData,u32 Len)
{
	u32 Sum=0;
	u32 i=0;

 	if(pData == NULL || Len == 0) return 0;
 	
	for(;i<Len;i++)
	{
		Sum+=pData[i];
	}

	return Sum;
}

/*Table of CRC values for high-order byte*/
const static u8 auchCRCHi[]={
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/*Table of CRC values for low-order byte*/
const static char auchCRCLo[]={
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

//计算crc
u16 CRC16(const u8 *pData,u16 Len)
{
	u8 uchCRCHi=0xff; /*校验码高字节初值*/
	u8 uchCRCLo=0xff;/* 校验码低字节初值*/
	u16 uIndex;       

	while(Len--)
	{
		uIndex=uchCRCHi^*pData++;
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	
	return(uchCRCHi<<8|uchCRCLo);
}

void PrintChineseCharToCode(const u8 *pChineseStr)
{
	int i;
	u16 Len=strlen(pChineseStr);

	Debug("{");
	for(i=0;i<Len;i++)
	{
		Debug("0x%02x,",pChineseStr[i]);
	}
	Debug("\b}, /\/\%s\n\r",pChineseStr);
}

//交换u16的高低字节
//0x1234 -> 0x3412
u16 Rev16(u16 Data)
{
	return __REV16(Data);
}

//翻转u32的字节顺序
//0x12345678 -> 0x78563412
u32 Rev32(u32 Data)
{
	return __REV(Data);
}

//返回非0随机值
//如果想得到包含0的随机值，右移1位即可。
u32 Rand(u32 Mask)
{
	u32 RandNum=((GetAdcRand()+(*((volatile u32 *)0xE000E018)))*(*((volatile u32 *)0xE000E018)))&Mask;	 //by systick

#if 0
	{
		u32 i;
		Debug("GetAdcRand %u\n\r",GetAdcRand());for(i=0xfffff;i;i--);
	}
#endif



	return (RandNum?RandNum:(0x12345678&Mask));
}



