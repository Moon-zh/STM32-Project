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
 ALIENTEKս��STM32������ʵ��53
 UCOSIIʵ��3-��Ϣ���С��ź������������ʱ��  ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//LED����
//�����������ȼ�
#define LED_TASK_PRIO       			7 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);

//�¶Ȳɼ�����
//�����������ȼ�
#define ZONE_TASK_PRIO       		 	6
//���������ջ��С
#define ZONE_STK_SIZE  				128
//�����ջ	
OS_STK ZONE_TASK_STK[ZONE_STK_SIZE];
//������
void Temperature_task(void *pdata);

//������
//�����������ȼ�
#define MASTER_TASK_PRIO       			4 
//���������ջ��С
#define MASTER_STK_SIZE  					512
//�����ջ	
OS_STK MASTER_TASK_STK[MASTER_STK_SIZE];
//������
void MasterCtrl_task(void *pdata);

//�ź���������
//�����������ȼ�
#define FLAGS_TASK_PRIO       			3 
//���������ջ��С
#define FLAGS_STK_SIZE  		 		56
//�����ջ	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//������
void flags_task(void *pdata);

//GPRS����
//�����������ȼ�
#define GPRS_TASK_PRIO       			5 
//���������ջ��С
#define GPRS_STK_SIZE  		 			512
//�����ջ	
OS_STK GPRS_TASK_STK[GPRS_STK_SIZE];
//������
void gprs_task(void * pdata);

//��������
//�����������ȼ�
#define HMI_TASK_PRIO       			8 
//���������ջ��С
#define HMI_STK_SIZE  		 			256
//�����ջ	
OS_STK HMI_TASK_STK[HMI_STK_SIZE];
//������
void Task_HMIMonitor(void * pdata);

//����������
//�����������ȼ�
#define FLOW_TASK_PRIO       			9 
//���������ջ��С
#define FLOW_STK_SIZE  		 			128
//�����ջ	
OS_STK FLOW_TASK_STK[FLOW_STK_SIZE];
//������
void Task_Flowmeter(void * pdata);

//����ɨ������
//�����������ȼ�
#define KEY_TASK_PRIO       			2 
//���������ջ��С
#define KEY_STK_SIZE  					56
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);
//////////////////////////////////////////////////////////////////////////////
 u8 count=0;   
OS_EVENT * msg_key;			//���������¼���	  
OS_EVENT * q_msg;			//��Ϣ����
OS_TMR   * tmr1;			//�����ʱ��1
OS_TMR   * tmr2;			//�����ʱ��2
OS_TMR   * tmr3;			//�����ʱ��3
OS_FLAG_GRP * flags_key;	//�����ź�����
void * MsgGrp[256];			//��Ϣ���д洢��ַ,���֧��256����Ϣ

extern MOCHINEStruct  MOCHINEDATE;//״̬��ִ����������
extern u8 horizon_test;
extern FlowmeterStruct FlowmeterCount; //����������
//�����ʱ��3�Ļص�����				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	//u8* p;	 
} 


 int main(void)
 {	 		    
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	//uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
  	BSP_InitIO();				//��ʼ��IO
  	GPS_Init();					//��ʼ��GPS
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
   	W25QXX_Init();
   	bsp_InitUart();				//��ʼ��RS485
//	 TIM4_Config();//���ö�ʱ��4
	 MOCHINEDATE.FertilizerOnID =1; //��ʱĬ��ʩ�ʱ�IDΪ1 ������Ҫֻ����Ĵ˲����Ϳ���
	OSInit();  	 				//��ʼ��UCOSII
  	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	  
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr=0;
	u8 str[24]={0,0,0};
//	u8 err;	    	    
	pdata = pdata; 	
	RTC_Init();
	OSStatInit();						//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();				//�����ٽ���(�޷����жϴ��)    
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
	
	TIM3_Int_Init(9999,7199);//10Khz�ļ���Ƶ�ʣ�������10000Ϊ1s  
// 	OSTaskSuspend(START_TASK_PRIO);		//������ʼ����.
	OS_EXIT_CRITICAL();					//�˳��ٽ���(���Ա��жϴ��)
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

