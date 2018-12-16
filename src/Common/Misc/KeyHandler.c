#include "SysDefines.h"

#if PRODUCT_IS_JUMPER
#define KEY_FUNC1_DELAY_MS 3000 //功能1
#define KEY_FUNC2_DELAY_MS 6000 //功能2
static bool BeepStopFlag=FALSE;//停止鸣叫

typedef enum{
	KFT_NULL=0,
	KFT_UPDATE,
	KFT_UNBIND,
	KFT_DEF,
}KEY_FUNC_TYPE;

//按键按下时间提醒
void KeyTimeTip(int cnt,void *p)
{
	if(BeepStopFlag==TRUE) return;	

	if(IOIN_ReadIoStatus(IOIN_SYS_KEY)==0)
	{
		if(cnt)
		{
			if(cnt<3)
				BeepSecTip(0,NULL);//
			else if(cnt<6)
				BeepSecTip(100,NULL);
			else
				BeepSecTip(200,NULL);
		}

		AddOnceMsFunc(1000,KeyTimeTip,cnt+1,NULL);
	}	
}

void KeyHandler(KEY_ID KeyID,KEY_ACTIVE Act,u32 Ms)
{
	//Debug("KeyID:%d,KeyAct:%d,Ms:%d\n\r",KeyID,Act,Ms);

	if(Act==KEY_PRESS)//按下
	{
		BeepStopFlag=FALSE;
		KeyTimeTip(0,NULL);
	}
	else//松开
	{
		BeepStopFlag=TRUE;
		BeepOn(FALSE);
		
		if(Ms>KEY_FUNC2_DELAY_MS)	
		{
			SendEvent(EBF_KEY,(KeyID<<16)|(Act<<8)|KFT_DEF,NULL);
		}
		else	if(Ms>KEY_FUNC1_DELAY_MS)
		{
			SendEvent(EBF_KEY,(KeyID<<16)|(Act<<8)|KFT_UNBIND,NULL);
		}
		else
		{
			SendEvent(EBF_KEY,(KeyID<<16)|(Act<<8)|KFT_UPDATE,NULL);
		}
	}
}

void KeyHandlerCallBack(KEY_ID KeyID,KEY_ACTIVE Act,u8 Param)
{
	if(KeyID==KI_KEY1 && Param==KFT_UPDATE)//上报温度
	{
		SendVarsToHost(1,NULL);
	}
	else if(KeyID==KI_KEY1 && Param==KFT_UNBIND)//清空绑定
	{
		UnbindToHost(TRUE,NULL);
	}
	else	if(KeyID==KI_KEY1 && Param==KFT_DEF)//清空绑定并清空缓存
	{
		DefaultConfig();
	}
}
#elif PRODUCT_IS_F_LCD
static bool gSlideNow=FALSE;
void KeyHandler(KEY_ID KeyID,KEY_ACTIVE Act,u32 Ms)
{
	//Debug("KeyID:%d,KeyAct:%d,Ms:%d\n\r",KeyID,Act,Ms);
	
	if(gSlideNow==FALSE) SendEvent(EBF_KEY,KeyID,NULL);
}

void KeyStateChangeHandler(KEY_ID KeyID)
{
	switch(KeyID)
	{
		case KI_KEY1:
		case KI_KEY4:
			LedSet(IOOUT_LED_LEFT,TRUE);
			gSlideNow=TRUE;
			DispSlide(TRUE);
			gSlideNow=FALSE;
			LedSet(IOOUT_LED_LEFT,FALSE);
			break;
		case KI_KEY2:
		case KI_KEY3:
			LedSet(IOOUT_LED_RIGHT,TRUE);
			gSlideNow=TRUE;
			DispSlide(FALSE);
			gSlideNow=FALSE;
			LedSet(IOOUT_LED_RIGHT,FALSE);
			break;
	}
}

#endif

