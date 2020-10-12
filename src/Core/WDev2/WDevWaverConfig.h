#ifndef WNET_WAVER_CONFIG_H
#define WNET_WAVER_CONFIG_H

#include "LimitMarco.h"

typedef enum{
	WDT_NULL=0,
	WDT_WDEV_IO,//万纳模块
	WDT_OPEN_DUT,//open dev
	WDT_JMP,//转发器
	WDT_COM,//串口透传
	WDT_WIFI,//wifi模块设备
	WDT_LOW_POWER,//低功耗设备
	WDT_BLUE_TOOTH,//蓝牙设备

}WAVER_DUT_TYPE;


typedef enum{
	WIM_NULL=0,
	
	WIM_IPD,//下拉输入，响应上升沿和下降沿，发生响应时，可以匹配、反向匹配、取反、计数变量。变量响应时间为立刻，适合接数字信号。
	
	WIM_KEYU,//按键模式，会有防干扰处理，只响应上升沿，发生响应时，可以匹配、反向匹配、取反、计数变量。变量响应时间为延时200ms，适合接按键。
	WIM_KEYU_OPPO,//按键模式带时机，响应上升沿时，发出时机。时机号就是io号，1-8。
	
	WIM_IPU,//上拉输入

	WIM_OUT_PP,//推挽输出，根据绑定的变量来匹配输出，当变量某bit发生变化时，可以匹配、反向匹配、取反。
	WIM_OUT_PP_KEY,//推挽输出，不绑定变量，当接受到key值时，可以置高、置低、取反输出。key值就是io号，1-8。

	WIM_OUT_OD,//开漏输出
	WIM_OUT_OD_KEY,//开漏输出

	WIM_IRQ,//串口通信信号，高电平有效	
	WIM_AIN,//模拟量输入，采样周期同上报周期相同，但上报周期到期时，如变量未变，则不上报
	WIM_PWM,//pwm输出，只支持io7，io8
	
	WIM_MAX
}WAVER_IO_MODE;

typedef enum{
	AIM_NULL=0,
	AIM_MATCH,//匹配。输出时，变量对应bit为1，输出1，bit为0，输出0。输入时，io为1，bit改为1，io为0，bit改为0。
	AIM_REVERSE,//反向匹配。输出时，bit为1，输出0，bit为0，输出1。输入时，io为1，bit改为0，io为0，bit改为1。
	AIM_NEGATION,//取反。输出时，bit改变则取反输出io状态。输入时，io有中断，则bit取反
	AIM_COUNT,//计数。输出时，当有指定key键过来，则输出脉冲，脉冲长度为100ms乘以Num，脉冲电平为bit。输入时，io有中断，则变量加1

	AIM_MAX,
}VAR_IO_MATCH;

typedef struct{
	WAVER_IO_MODE IoMode:4;//io模式
	VAR_IO_MATCH VarIoMth:3;//io与变量的关系
	bool IsRelVar:1;//num表示变量号时，此位为0，表示自身变量；此位为1，表示关联变量。
	u8 Num:4;//变量号，0表示不映射，1-8为内部变量序号，1-4为关注变量序号。
	u8 Bit:4;//变量位，0-15
}WAVER_IO_CONFIG;

typedef struct{
	u8 AdcMode;//使用adc时，此值置1
	u8 Tolerance;//容差，1-99%，采样值运算的结果超过容差，变量才改变
	u16 Factor;//因子，采样值除以此值，再加上偏差，才存储到变量，此值为实际值的100倍，不允许为0
	s16 Offset;//偏差
}WAVER_AIN_CONFIG;

typedef enum{
	WPW_NULL=0,//不使用
	WPW_FIXED,//固定式，变量的bit控制开关
	WPW_PERIOD,//脉宽可调式，变量决定Cnt值
	WPW_FACTOR,//占空比可调式，变量决定PluseCnt值
}WAVER_PWM_TYPE;

typedef struct{
	WAVER_PWM_TYPE PwmMode;
	u8 a;
	u16 Cnt;//决定脉宽
	u16 PluseCnt;//决定占空比
	u16 uS_Base;
}WAVER_PWM_CONFIG;

typedef enum{
	AUM_NULL=0,//无效的变量
	AUM_CMD,//不主动上报，串口发送时上报
	AUM_CHANGE,//变量改变就发送
	AUM_IO_KEY,//通过io key来发送，param存储io号，1-8
	AUM_PERIOD,//周期性发送，param存储周期 TIME_PERIOD_TYPE。adc必须选此项，否则不会主动采样。

	AUM_MAX
}VAR_UPD_MODE;

typedef enum{
	APP_1S=0,
	APP_5S,
	APP_10S,
	APP_30S,
	APP_1M,
	APP_5M,
	APP_10M,
	APP_30M,
	APP_1H,
	APP_3H,
	APP_6H,
	APP_10H,
	APP_24H,
	APP_72H,
	APP_1W,	

	APP_MAX,
}TIME_PERIOD_TYPE;//对应gTimeMap数组

typedef struct{
	VAR_UPD_MODE Mode:3;//变量上报的模式
	bool SaveBit:1;//设置后，变量掉电存储
	u8 UpdParam:4;//定时上传时，存储TIME_PERIOD_TYPE。通过io发送变量时，存储io号，1-8。
}WAVER_VAR_CONFIG;//变量配置

typedef struct{
	TIME_PERIOD_TYPE AwakenPeriod:4;//定时周期唤醒时间，为0时，表示不休眠
	u8 a:4;
	u8 b;
	
	u8 LifeTime:7;//定时唤醒后的存活时间，为0时，表示永远存活
	bool LifeTimeIsMin:1;	//存活时间是否以分钟为单位

	u8 RfLifeTime:7;//射频唤醒后的存活时间，只有特定型号具备此功能
	bool RfLifeIsMin:1;	//存活时间是否以分钟为单位
}WAVER_LOW_POWER_CONFIG;//低功耗配置

typedef struct{
	u32 ProdID;//厂设id，用户申请，酷享审核分配
	u8 TabVer;//配置表的版本号，填1
	WAVER_DUT_TYPE DutType;//u8 板卡类型，必须跟板卡配套，板卡上会印制
	u16 WNetGroup;//万纳组地址

	WAVER_IO_CONFIG IoConfig[WAVER_IO_NUM];//每个io的运行模式
	WAVER_AIN_CONFIG AioConfig[WAVER_AIO_NUM];//目前只提供4路adc
	WAVER_PWM_CONFIG PwmConfig[WAVER_PWM_NUM];//目前只提供2路pwm

	u8 VarNum;//自身变量的个数
	u8 a;
	u16 b;
	WAVER_VAR_CONFIG VarConfig[SELF_VAR_MAX];//变量上报模式
	s16 VarInitVal[SELF_VAR_TOTAL];//自身变量初始值	

	u32 resv[8];
	
	WAVER_LOW_POWER_CONFIG LowPowerConfig;//低功耗设置

	u32 resvv[8];	
}WAVER_PARAM_TAB;//量产工具所下发的信息


















#endif

