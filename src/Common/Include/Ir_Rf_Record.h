#ifndef IR_AND_RF_H
#define IR_AND_RF_H

typedef enum{
	SIT_NULL=0,
	SIT_IR,
	SIT_RF_STD,
	SIT_RF_OTHERS,

}SIG_TYPE;

//------------------------------------ IR ----------------------------------------
#define IR_FUZZY_PARAM 10 //模糊对比的百分比因子
#define MAX_IR_IDX_NUM 16
#define MAX_IR_PLUSE_NUM 428
typedef struct{
	u32 ID;
	SIG_TYPE Type:2;//信号类型
	u16 SendCnt:4;//发送个数，乘以4，为实际发送个数，ir无用
	u16 PulseNum:10;//脉冲个数
	
	u16 IdxTimes[MAX_IR_IDX_NUM];//索引值
	u8 Pluse[MAX_IR_PLUSE_NUM/2];//每个字节分高4位和低4位，存储2个脉冲长度，先存低4位，存低电平
}IR_RECORD;


//------------------------------------ RF ----------------------------------------
#define RF_FUZZY_PARAM 10 //模糊对比的百分比因子
#define MAX_RF_IDX_NUM 16
#define MAX_RF_PLUSE_NUM 428

typedef struct{
	u32 ID;//存储id
	SIG_TYPE Type:2;//信号类型
	u16 SendCnt:4;//发送个数，乘以4，为实际发送个数
	u16 PulseNum:10;//脉冲个数

	u32 Code;//编码，低24位有效
	u16 BasePeriod;// 4a的时间长度，同步位低位为124a，宽高电平为12a，窄高电平为4a，低电平为4a或12a
}RF_STD_RECORD;

typedef struct{
	u32 ID;
	SIG_TYPE Type:2;//信号类型
	u16 SendCnt:4;//发送个数，乘以4，为实际发送个数
	u16 PulseNum:10;//脉冲个数

	u16 IdxTimes[MAX_RF_IDX_NUM];//索引值
	u8 Pluse[MAX_RF_PLUSE_NUM/2];//每个字节分高4位和低4位，存储2个脉冲长度，先存低4位，存高电平
}RF_RECORD;











#endif
