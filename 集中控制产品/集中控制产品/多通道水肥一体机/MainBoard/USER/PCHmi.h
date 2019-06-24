#ifndef __PCHMI_H
#define __PCHMI_H	
#include "sys.h"	 

//数据量的最大值定义
#define PC_COIL_NUM  	88//必须为8的整数倍
#define PC_INSTA_NUM 	48	//MAX_INSTA_NUM /8不能大于 256-9  帧长度不能大于256 必须为8的整数倍
#define PC_REG_NUM		255
#define PC_INREG_NUM	20

typedef struct
{
	u8 RevType;//0 接收错误 1 从机接收 2 主机接收  
	u8 SlaveTxFlag;//从机发送标志 1 需要发送
	u8 MasterState;//主机状态 
	u16 MasterErrCnt;//主机错误计数
	u8 Off_Line;
} PCCtrlStruct;
void PCHmi_task(void *pdata);
#endif	   

