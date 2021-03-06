#ifndef _LEDS_MODE_H_
#define _LEDS_MODE_H_


typedef enum{
	LM_OFF=0,
	LM_ON,
	LM_ON_500MS,
	LM_ON_1S,
	LM_ON_2S,
	LM_FLASH_200MS,
	LM_FLASH_500MS,
	LM_FLASH_2S,
	LM_FLASH_200MS_L2S,//200ms闪烁，时长2秒钟

	LM2_OFF=0x10,
	LM2_ON,
	LM2_ON_500MS,
	LM2_ON_1S,
	LM2_ON_2S,	
	LM2_FLASH_200MS,
	LM2_FLASH_500MS,
	LM2_FLASH_2S,
	LM2_FLASH_200MS_L2S,
	
}LEDS_MODE;

#define LMO_IDLE LM2_OFF //闲置状态
#define LMO_CANCLE LM2_OFF //操作取消
#define LMO_WAIT_PAIR LM2_FLASH_200MS //准备配对
#define LMO_WAIT_BROTHER LM2_FLASH_2S //等待兄弟上线
#define LMO_WORK LM2_ON //进入工作状态

#define LMO_ERR LM_FLASH_200MS_L2S //操作错误
#define LMO_CMD_MODE LM_FLASH_2S //485数据进命令行模式
#define LMO_KEY_INDICATE LM_ON_500MS //按键关联指示

#define LedIndicate(x) SendEvent(EBF_LED_MODE,x,NULL)//错误灯


#endif

