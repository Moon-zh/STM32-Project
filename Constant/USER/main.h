#include "led.h"
#include "delay.h"
#include "sys.h"	 
#include "Rs485.h"
#include "check.h"
#include "includes.h"
#include "check.h"
#include "stmflash.h"
#include "Emw3060.h"

extern char ssid[20];			//wifi ID
extern char password[20];		//wifi ����

extern char ProductKey1[20];	//���豸��Ԫ��
extern char DeviceName1[50];
extern char DeviceSecret1[50];

extern char *ProductKeyw;
extern char *DeviceNamew;
extern char *DeviceSecretw;


#define FLASH_SAVE_ADDR  0X0802DE00		//�������ô洢��ַ
#define FLASH_THREE_ADDR  0X0802FE00	//��Ԫ��洢��ַ

//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//�����������ȼ�
#define LED_TASK_PRIO       			1 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ	
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void LED_task(void *pdata);

//�����������ȼ�
#define SaveThree_TASK_PRIO       		5 
//���������ջ��С
#define SaveThree_STK_SIZE  		    1024
//�����ջ	
__align(8)OS_STK SaveThree_TASK_STK[SaveThree_STK_SIZE];
//������
void SaveThree_task(void *pdata);

//�����������ȼ�
#define UpyunWF_TASK_PRIO       		7 
//���������ջ��С
#define UpyunWF_STK_SIZE  		    	1024
//�����ջ	
__align(8)OS_STK UpyunWF_TASK_STK[UpyunWF_STK_SIZE];
//������
void UpyunWF_task(void *pdata);

//�����������ȼ�
#define Converter_TASK_PRIO       		2 
//���������ջ��С
#define Converter_STK_SIZE  		    512
//�����ջ	
__align(8)OS_STK Converter_TASK_STK[Converter_STK_SIZE];
//������
void Converter_task(void *pdata);

//�����������ȼ�
#define VM_TASK_PRIO       		2 
//���������ջ��С
#define VM_STK_SIZE  		    512
//�����ջ	
__align(8)OS_STK VM_TASK_STK[VM_STK_SIZE];
//������
void VM_task(void *pdata);

u8	READBZ=1;			//�ж��Ƿ���һ���ϴ����ڽ��������ڱ����Ƿ��ϴ�
u8	LED_BZ=0;			//LED��ʾ״̬�л���־
unsigned char printf_num=1;	//printf ����ָ���־ 1ָ��uart1 2ָ��uart2
unsigned char Emw_B=0;		//wifiģ���ʼ����ɱ�־
unsigned char emw_set=1;	//wifi�������̻߳����־
unsigned int  up_t1=2;	//�����ϴ�Ƶ��
unsigned int  up_t2=1;	//�����ϴ�Ƶ��
