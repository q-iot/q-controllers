#ifndef KEY_HANDLER_H
#define KEY_HANDLER_H

typedef enum{
	KEY_NULL=0,
	KI_KEY1,
	KI_KEY2,
	KI_KEY3,
	KI_KEY4,

	KI_MAX
}KEY_ID;

typedef enum{
	KEY_NULL_ACT=0,
	KEY_PRESS,//按下动作
	KEY_RELEASE,//松开
	KEY_RESET,//
}KEY_ACTIVE;

void KeyHandler(KEY_ID KeyID,KEY_ACTIVE Act,u32 Ms);
void KeyHandlerCallBack(KEY_ID KeyID,KEY_ACTIVE Act,u8 Param);


#endif

