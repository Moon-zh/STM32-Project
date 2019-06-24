#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "rtc.h" 		 	 
#include "string.h"
#include "Includes.h"
#include "Rs485.h"
#include "led.h"
#include "ZoneCtrl.h"
#include "timer.h"
#include "IO_BSP.h"
#include "W25qxx.h"
#include "GPS.h"
#include "UserCore.h" 
#include "Flowmeter.h" 
/************************************************
 ALIENTEK战舰STM32开发板实验53
 UCOSII实验3-消息队列、信号量集和软件定时器  实验 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			7 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);

//温度采集任务
//设置任务优先级
#define ZONE_TASK_PRIO       		 	6
//设置任务堆栈大小
#define ZONE_STK_SIZE  				128
//任务堆栈	
OS_STK ZONE_TASK_STK[ZONE_STK_SIZE];
//任务函数
void Temperature_task(void *pdata);

//主任务
//设置任务优先级
#define MASTER_TASK_PRIO       			4 
//设置任务堆栈大小
#define MASTER_STK_SIZE  					512
//任务堆栈	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//任务函数
void MasterCtrl_task(void *pdata);

//信号量集任务
//设置任务优先级
#define FLAGS_TASK_PRIO       			3 
//设置任务堆栈大小
#define FLAGS_STK_SIZE  		 		56
//任务堆栈	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//任务函数
void flags_task(void *pdata);

//GPRS任务
//设置任务优先级
#define GPRS_TASK_PRIO       			5 
//设置任务堆栈大小
#define GPRS_STK_SIZE  		 			512
//任务堆栈	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//任务函数
void gprs_task(void * pdata);

//界面任务
//设置任务优先级
#define HMI_TASK_PRIO       			8 
//设置任务堆栈大小
#define HMI_STK_SIZE  		 			256
//任务堆栈	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//任务函数
void Task_HMIMonitor(void * pdata);

//流量计任务
//设置任务优先级
#define FLOW_TASK_PRIO       			9 
//设置任务堆栈大小
#define FLOW_STK_SIZE  		 			128
//任务堆栈	
OS_STK FLOW_TASK_STK[FLOW_STK_SIZE];
//任务函数
void Task_Flowmeter(void * pdata);

//按键扫描任务
//设置任务优先级
#define KEY_TASK_PRIO       			2 
//设置任务堆栈大小
#define KEY_STK_SIZE  					56
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
 u8 count=0;   
OS_EVENT * msg_key;			//按键邮箱事件块	  
OS_EVENT * q_msg;			//消息队列
OS_TMR   * tmr1;			//软件定时器1
OS_TMR   * tmr2;			//软件定时器2
OS_TMR   * tmr3;			//软件定时器3
OS_FLAG_GRP * flags_key;	//按键信号量集
void * MsgGrp[256];			//消息队列存储地址,最大支持256个消息

extern MOCHINEStruct  MOCHINEDATE;//状态机执行所需条件
extern u8 horizon_test;
extern FlowmeterStruct FlowmeterCount; //流量计数组
//软件定时器3的回调函数				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	//u8* p;	 
} 


 int main(void)
 {	 		    
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	//uart_init(115200);	 	//串口初始化为115200
  	BSP_InitIO();				//初始化IO
  	GPS_Init();					//初始化GPS
//	my_mem_init(SRAMIN);		//初始化内部内存池
   	W25QXX_Init();
   	bsp_InitUart();				//初始化RS485
//	 TIM4_Config();//配置定时器4
	 MOCHINEDATE.FertilizerOnID =1; //暂时默认施肥泵ID为1 后期需要只需更改此参数就可以
	OSInit();  	 				//初始化UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	u8 str[24]={0,0,0};
//	u8 err;	    	    
	pdata = pdata; 	
	RTC_Init();
	OSStatInit();						//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();				//进入临界区(无法被中断打断)    
	//delay_ms(1000);
 /*	OSTaskCreate(led_task,(void *)0,
		(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],
		LED_TASK_PRIO);		*/
//	
OSTaskCreate(ZoneCtrl_task,(void *)0,
		(OS_STK*)&ZONE_TASK_STK[ZONE_STK_SIZE-1],
		ZONE_TASK_PRIO);	 	
//	
// 	OSTaskCreate(gprs_task,(void *)0,
//		(OS_STK*)&GPRS_TASK_STK[GPRS_STK_SIZE-1],
//		GPRS_TASK_PRIO);	 
//	
// 	OSTaskCreate(MasterCtrl_task,(void *)0,
//		(OS_STK*)&MASTER_TASK_STK[MASTER_STK_SIZE-1],
//		MASTER_TASK_PRIO);	 	
////	
// 	OSTaskCreate(Task_HMIMonitor,(void *)0,
//		(OS_STK*)&HMI_TASK_STK[HMI_STK_SIZE-1],
//		HMI_TASK_PRIO);	
	
	OSTaskCreate(Task_Flowmeter,(void *)0,
		(OS_STK*)&FLOW_TASK_STK[FLOW_STK_SIZE-1],
		FLOW_TASK_PRIO);		
 /*	OSTaskCreate(flags_task,(void *)0,
		(OS_STK*)&FLAGS_TASK_STK[FLAGS_STK_SIZE-1],
		FLAGS_TASK_PRIO);	 */				   
	
 	/*OSTaskCreate(key_task,(void *)0,
		(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
		KEY_TASK_PRIO);	 */				
	
	TIM3_Int_Init(9999,7199);//10Khz的计数频率，计数到10000为1s  
// 	OSTaskSuspend(START_TASK_PRIO);		//挂起起始任务.
	OS_EXIT_CRITICAL();					//退出临界区(可以被中断打断)
 while(1)
 {
  delay_ms(1000);
//	if(horizon_test==1)
//	{

//	FlowmeterCount.databuf[0]++;
//	if(FlowmeterCount.databuf[0]>250)
//	{
//		FlowmeterCount.databuf[0]=0;
//		FlowmeterCount.databuf[1]++;
//	
//	}
//	FlowmeterCount.databuf[4]++;
//	if(FlowmeterCount.databuf[4]>250)
//	{
//		FlowmeterCount.databuf[4]=0;
//		FlowmeterCount.databuf[5]++;
//	
//	}
//	
// 	FlowmeterCount.databuf[8]++;
//	if(FlowmeterCount.databuf[8]>250)
//	{
//		FlowmeterCount.databuf[8]=0;
//		FlowmeterCount.databuf[9]++;
//	
//	}
// 	FlowmeterCount.databuf[12]++;
//	if(FlowmeterCount.databuf[12]>250)
//	{
//		FlowmeterCount.databuf[12]=0;
//		FlowmeterCount.databuf[13]++;
//	
//	}	
//	}
 } 
}

