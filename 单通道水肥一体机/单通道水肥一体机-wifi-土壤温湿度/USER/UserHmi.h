#ifndef USER_H
#define USER_H


#include "includes.h"
#include "cmd_queue.h"





#define RED_COLOR_HMI       0xf800
#define BLUE_COLO_HMI      0x001f
#define GREEN_COLOR_HMI     0x07c0
#define YELLOW_COLOR_HMI    0xffe0
#define WHITE_COLOR_HMI     0xffff

#define WARINGPAGE_EMERGENCY 0x00
#define WARINGPAGE_CHARGE	0x01
#define WARINGPAGE_LOCATIONDEFEAT 0x02
#define WARINGPAGE_CUERROR		 0x03

#define IDLECOUNTNUM 3000 //5���� 3000/60/10 = 5



//#define LCD_STANDBY_PAGE 0  //��������


typedef enum _AGVSM_LCDPageNumber	
{
  LCD_MAIN_PAGE = 7,        // ������
	LCD_STANDBY_PAGE = 0,		  // ��������
	LCD_STATESHOW_PAGE = 4, 	// ���ڹ�Ƚ���
	LCD_MODELSEL_PAGE = 1, 		// ģʽѡ��
	LCD_ZONESEL_PAGE = 2, 		// ��ȷ���ѡ��
	LCD_TIMESET_PAGE = 3, 		// ���ʱ������
	LCD_PARASET_PAGE = 5,		  // WIFI�豸����
	LCD_PARASAVE_PAGE = 6,		// �������
	LCD_ZONESEL_PAGE_KONG = 7, //��ȷ���ѡ��Ϊ��
	LCD_ZIDONG_MOSHI= 8 ,     //�Զ�ģʽ����
} AGVSM_LCDPageNumber;


typedef struct
{
	u8 PumpWFlag;	//ˮ���Ƿ���
	u8 PumpFFlag;	//�ʱ��Ƿ���
	u8 Passageway[6]; //ͨ����λ����
	u32 Zone;		//����������
	u8 WorkHour;	//����ʱ��(h)
	u8 WorkMinute;	//����ʱ��(m)
	u8 WorkDay;		//�����ܼ�
	u8 StartHour;	//��ʼСʱ
	u8 StartMinute;	//��ʼ����
}ProjectStruct;



void Task_HMIMonitor ( void * parg );
void NumberToASCII ( u8 *Arry , u16 Number);
void DisCountDown(u16 RemainderMinute);
void MainPageDisSta(u8 State);
void WaterStaShow(u8 State);
void FertilizerStaShow(u8 State);
void SetRadiotube(u8 Num, u8 Value);
void SetSwitchAlarm(void);
void WorkModelShow(u8 Value,u8 ZoneNum);
void WorkZoneShow(u8 Value);
void WorkCountShow(u8 hour,u8 minute);
void ClearStateShow(void);
void FerStageShow(u8 Stage);
void FerStateShow(u8 State);

void	HDMI_Init(void);	//��������ʼ��

extern u8 DHCP;
extern u8 DHCP_cq;

extern u8 hmi_cmp_rtc[7];




#endif
