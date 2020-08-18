#ifndef ALL_CONTROLLERS_H //此宏定义每个c文件不一样，一定要改
#define ALL_CONTROLLERS_H //此宏定义每个c文件不一样，一定要改


void NewControllerReg(void);
void TestControllerReg(void);
void QComControllerReg(void);//qwifi指令解析
void QWifiControllerReg(void);//qwifi指令处理
void CollControllerReg(void);//数据收集事件
void LoraControllerReg(void);//lora

#endif
