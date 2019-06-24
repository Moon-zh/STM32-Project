#ifndef USER_H
#define USER_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"
#include "cmd_queue.h"

#define RED_COLOR_HMI       0xf800
#define BLUE_COLO_HMI      	0x001f
#define GREEN_COLOR_HMI     0x07c0
#define ORANGE_COLOR_HMI    0xfb20
#define GRAY_COLOR_HMI    	0xC618
#define YELLOW_COLOR_HMI    0xffe0
#define WHITE_COLOR_HMI     0xffff

#define WARINGPAGE_EMERGENCY 0x00
#define WARINGPAGE_CHARGE	0x01
#define WARINGPAGE_LOCATIONDEFEAT 0x02
#define WARINGPAGE_CUERROR		 0x03

extern u8 g_u8WiFiRestart;

typedef enum _AGVSM_LCDPageNumber
{
	LCD_STANDBY_PAGE = 0,			//0 ��������
	LCD_START_PAGE = 1,				//1 ��������
	LCD_IRRSTEP1_PAGE = 2,			//2 ��Ȳ�����һ��
	LCD_WATERSTEP2_PAGE = 3, 		//3 ��ˮ�����ڶ���
	LCD_FERSTEP2_PAGE = 4,			//4	��ˮ�����ڶ���
	LCD_WATERSTEP3_PAGE = 5,		//5 ��ˮ����������
	LCD_FERSTEP3_PAGE = 6,			//6 ��ˮ����������
	LCD_FERSTEP4_PAGE = 7,			//7 ��ˮ�������Ĳ�
	LCD_WATERSHOW_PAGE = 8,			//8 ��ˮ��ʾ
	LCD_FERSHOW_PAGE = 9, 			//9 ��ˮ��ʾ
	LCD_SETSTEP1_PAGE = 10,			//10 ���õ�һ��
	LCD_SETSTEP2_PAGE = 11,			//11 ���õڶ���
	LCD_SETSTEP3_PAGE = 12,			//12 ���õ�����
	LCD_SETSTEP4_PAGE = 13,			//13 ���õ��Ĳ�
	LCD_SETSTEP5_PAGE = 14,			//14 ���õ��岽
	LCD_SETSTEP6_PAGE = 15,			//15 ���õ�����
	LCD_SETSTEP7_PAGE = 16, 		//16 ���õ��߲�
	LCD_SAVE1_PAGE = 17, 			//17 Ũ�ȱ���
	LCD_SAVE2_PAGE = 18, 			//18 �����������
	LCD_SAVE3_PAGE = 19, 			//19 ���Ϸʱ���
	LCD_SAVE4_PAGE = 20, 			//20 һ�׶α���
	LCD_SAVE5_PAGE = 21, 			//21 ���׶α���
	LCD_SAVE6_PAGE = 22, 			//22 ���׶α���
	LCD_SAVE7_PAGE = 23, 			//23 �Ľ׶α���
	LCD_USESELECT_PAGE = 24, 			//�û�ѡ�����
	LCD_GUESTLOAD_PAGE = 25, 			//�ÿ͵�¼
	LCD_GUESTLOADERROR_PAGE = 26, 			//�ÿ͵�¼����
	LCD_ADMINLOAD_PAGE = 27, 					//����Ա��¼
	LCD_ADMINLOADERROR_PAGE = 28, 			//����Ա��¼����
	LCD_TASKRECORD_PAGE = 29, 			//�����¼
	LCD_GUESTVERIYFY_PAGE = 30, 			//�ÿ�����У��
	LCD_NEWPASSWORD_PAGE = 31, 			//�޸�������
	LCD_PASSWORDSAVE_PAGE = 32, 					//���뱣��ɹ�
	LCD_UNCOMPLETWORK_PAGE = 33, 			//δ���������ʾ	
	LCD_PASSWORDERROR_PAGE = 34, 					//�������
	LCD_NOTSAVE_PAGE = 35,			//δ������ʾ
	LCD_SETBUTONN_PAGE =36,    //����ѹ����ť������
	LCD_ERROR_PAGE	=37,				//����������ʾ	
	LCD_WIFI_PAGE = 38,			//wifi���ý���
	LCD_WIFISAVE_PAGE =39		//wifi�������
} AGVSM_LCDPageNumber;


typedef union
{
	u16 WaterRecord[60];
	u8 ElseRecord[120];
}HmiRecord;

typedef union
{
	u16 WaterTime;
	u16	IrrPjNum;	//��1��ʼ
}IrrPara;

typedef struct
{
	u32 WaterPartition;
	u32 FerPartition;
	u16 starttime;//��ʼʱ��
	u16 lefttime;//ʣ��ʱ��
	IrrPara Para;
	
}WorkPara;

typedef struct
{
	u8 task_num;
	u16 start_year; //����ʼ��
	u8 start_month;	//����ʼ��
	u8 start_day;	//����ʼ��
	u8 start_hour;	//����ʼʱ
	u8 start_minitue;	//����ʼ��
	u8 start_second;	//����ʼ��
  u8 irrigated_mode;//���ģʽ
  u16 irrigated_time;//���ʱ��
  u8 formula;     //�䷽����
  u16 fertilizing_amount;//Ķ��ʩ����
	u8 select_zone;//ѡ�����
  u8 stop_mode; //ֹͣ��ʽ	

}__attribute__((packed))CtaskRecord;
typedef union
{
	CtaskRecord record;
	u8 task[16];
}__attribute__((packed))CtaskRecordunion; //����洢��ȡ�ṹ��
//����ִ�нṹ��
typedef struct
{
 
 u8 curtargetnum;//��ǰִ�е������
 u8 lasttarget_id;//��һ������id	
 u8 currenttarget_id;//��ǰ����ID
 u8 Ctrl_update_state; //��Ҫ���¿ؼ���״̬
 u8 targetnum[32];//�����Դ�ȡ32������
// u16 curtargetstarttime;//��ǰ������ʱ��
 u8 curtargetyear; 
 u8 curtargetmon;
 u8 curtargetday;
 u8 curtargethour;
 u8 curtargetmin;
 u8 curtargetsec;
 u8 pausehour;	//��ͣʱ��Сʱ
 u8 pausemin;		//��ͣʱ�ķ���
 u8 pausesec;		//��ͣʱ������
 u16 pausetime; //��ͣ��ʱ��
 u8 pause_flag;//��ͣ��־
 u16 tasktimeing;//����ʱ��
 u8 wateringprogram;//��ȳ���
 u8 fer_onoff;//ˮ�ÿ����ر�״̬
 u8 fer_chanle;//ʩ�ʷ�ͨ��
 u8 curtarget_mode;  //���ģʽ
 u16 curtarget_time;  //���ʱ��
 u8 curtarget_front_time;//ǰ�ý�ˮ
 u16 curtarget_fertilizing_time;//ʩ��ʱ��
 u8 curtarget_back_time;//���ý�ˮ
 u8 curtarget_stopmode;//ֹͣģʽ
 u8 formula; //�䷽
 u16 fertilizing_amount;//Ķ��ʩ����
 u16 currenttargetlefttime;//��ǰ����ʣ��ʱ��
 u8 curtarget_max_num;//��ǰ������������

}__attribute__((packed))Targetrecord;
typedef union
{
	Targetrecord trecord;
	u8 target[67];
}__attribute__((packed))Targetred; //����洢��ȡ�ṹ��
//���ô洢����
typedef struct
{
		u16 P_A;// A��Ũ��
		u16 P_B;// B��Ũ��
		u16 P_C;// C��Ũ��
		u16 P_D;// D��Ũ��   
    u16 Partition1;//1����Ķ�� 
    u16 Partition2;//2����Ķ��
    u16 Partition3;//3����Ķ��
    u16 Partition4;//1����Ķ��
    u16 Partition5;//5����Ķ��
    u16 Partition6;//6����Ķ��
    u16 Partition7;//7����Ķ�� 
    u16 Partition8;//8����Ķ��
    u16 Partition9;//9����Ķ��
    u16 Partition10;//10����Ķ��
    u16 Partition11;//11����Ķ��
    u16 Partition12;//12����Ķ��    
    u16 Partition13;//13����Ķ��
    u16 Partition14;//14����Ķ��
    u16 Partition15;//15����Ķ��
    u16 Partition16;//16����Ķ��
    u16 Partition17;//1����Ķ��  //��16������ʱ����
    u16 Partition18;//2����Ķ��
    u16 Partition19;//3����Ķ��
    u16 Partition20;//1����Ķ��
    u16 Partition21;//5����Ķ��
    u16 Partition22;//6����Ķ��
    u16 Partition23;//7����Ķ�� 
    u16 Partition24;//8����Ķ��
    u16 Partition25;//9����Ķ��
    u16 Partition26;//10����Ķ��
    u16 Partition27;//11����Ķ��
    u16 Partition28;//12����Ķ��    
    u16 Partition29;//13����Ķ��
    u16 Partition30;//14����Ķ��
    u16 Partition31;//15����Ķ��
    u16 Partition32;//16����Ķ��    
    u16 Complex_Front_time;//���Ϸ�ǰ�ý�ˮʱ��  
    u16 Complex_Fer_time; //���Ϸ�ʩ��ʱ��    
    u16 Complex_Back_time; //���Ϸʺ���ʱ��     
    u16 Complex_A_Fer;//A�޷���  
    u16 Complex_B_Fer; //B�޷���    
    u16 Complex_C_Fer; //C�޷���    
    u16 Complex_D_Fer;//D�޷���
    u16 Complex_Fer_amount;//����Ķ��ʩ����   
    u16 First_Front_time;//��һ�׶�ǰ�ý�ˮʱ��  
    u16 First_Fer_time; //��һ�׶�ʩ��ʱ��    
    u16 First_Back_time; //��һ�׶κ���ʱ��     
    u16 First_A_Fer;//��һ�׶�A�޷���  
    u16 First_B_Fer; //��һ�׶�B�޷���    
    u16 First_C_Fer; //��һ�׶�C�޷���    
    u16 First_D_Fer;//��һ�׶�D�޷���
    u16 First_Fer_amount;//��һ�׶�Ķ��ʩ����       
    u16 Second_Front_time;//�ڶ��׶�ǰ�ý�ˮʱ��  
    u16 Second_Fer_time; //�ڶ��׶�ʩ��ʱ��    
    u16 Second_Back_time; //�ڶ��׶κ���ʱ��     
    u16 Second_A_Fer;//�ڶ��׶�A�޷���  
    u16 Second_B_Fer; //�ڶ��׶�B�޷���    
    u16 Second_C_Fer; //�ڶ��׶�C�޷���    
    u16 Second_D_Fer;//�ڶ��׶�D�޷���
    u16 Second_Fer_amount;//�ڶ��׶�Ķ��ʩ����   
    u16 Third_Front_time;//�����׶�ǰ�ý�ˮʱ��  
    u16 Third_Fer_time; //�����׶�ʩ��ʱ��    
    u16 Third_Back_time; //�����׶κ���ʱ��        
    u16 Third_A_Fer;//�����׶�A�޷���  
    u16 Third_B_Fer; //�����׶�B�޷���    
    u16 Third_C_Fer; //�����׶�C�޷���    
    u16 Third_D_Fer;//�����׶�D�޷���
    u16 Third_Fer_amount;//�����׶�Ķ��ʩ����       
    u16 Four_Front_time;//���Ľ׶�ǰ�ý�ˮʱ��  
    u16 Four_Fer_time; //���Ľ׶�ʩ��ʱ��    
    u16 Four_Back_time; //���Ľ׶κ���ʱ��     
    u16 Four_A_Fer;//���Ľ׶�A�޷���  
    u16 Four_B_Fer; //���Ľ׶�B�޷���    
    u16 Four_C_Fer; //���Ľ׶�C�޷���    
    u16 Four_D_Fer;//���Ľ׶�D�޷���
    u16 Four_Fer_amount;//���Ľ׶�Ķ��ʩ����         
    u16 PartitionEnable[16];
}__attribute__((packed))SetPara;
typedef union
{
	SetPara set;
	u16 FlashPara[92];
}__attribute__((packed))FlashSet; //���ò�������
extern char g_u8SysTemPassword[9];

void Task_HMIMonitor ( void * parg );
void NumberToASCII ( u8 *Arry , u16 Number);
void DisCountDown(u16 RemainderMinute);
void MainPageDisSta(u8 State);
void WaterStaShow(u8 State);
void FertilizerStaShow(u8 State);
void SetRadiotube(u8 Num, u8 Value);
void SetSwitchAlarm(void);
void Refresh_ACTION_Page(void);
void Refresh_Main_Page(void);
void PasswordSetShow(void);
void BasicInfoShow(void);
void GreenHouseInfoShow(void);
void IrrProjectShow(void);
void AmountSetShow(void);
void DosingSetShow(void);
void WaterSetShow(void);
//void WorkStateShow(void);
void IrrDetailShow(void);
void SerialNumShow(u8 PageNum,u8 Addr,u8 Num,u8 StarNum);
void IrrProjectNumShow(void);
void IrrPartitonNumShow(void);
void Save_TaskRecord(u8 tasknum,u8 read,u8 write);
void time_copy(void);
void Water_Program(u8 program);
void Display_SetButonn(void);
#endif

