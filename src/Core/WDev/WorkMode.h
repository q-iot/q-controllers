#ifndef _WORK_MODE_H_
#define _WORK_MODE_H_



typedef enum{
	MNM_IDLE=0,//闲置，未绑定
	MNM_START_PAIR,//开始配对
	MNM_SYNC,//确定配对关系
	MNM_PRE_WORK,//有配对关系，但对方没上线
	MNM_WORK,//已经添加，正常工作

	MNM_MAX
}NOW_WORK_MODE;
NOW_WORK_MODE WorkMode(void);
void SetWorkMode(NOW_WORK_MODE Mode);
void RevertWorkMode(u32 DelayMs);


#endif

