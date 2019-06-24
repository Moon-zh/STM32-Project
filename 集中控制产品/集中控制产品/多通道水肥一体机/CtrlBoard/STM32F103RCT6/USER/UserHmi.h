#ifndef USER_H
#define USER_H

//#include "DataTypes.h"
//#include "user_app.h"
#include "includes.h"
#include "cmd_queue.h"
//#include "cmd_process.h"

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

typedef enum _AGVSM_LCDPageNumber
{
	LCD_MAIN_PAGE = 0,          //0 ������
	LCD_ACTION_PAGE = 1,		//1 ��������
	LCD_STARTERR_PAGE = 2,		//2 �������ô������
	LCD_FRETERR_PAGE = 3,	//	 3 ʩ�ʵ�ŷ��������
	LCD_WORKOVERTIP_PAGE = 4, //4 ���������ʾ����
	LCD_WATERERR_PAGE = 5,		//5��ȵ�ŷ��������
	LCD_MOTORERR_PAGE = 6			//6���ʵ���������
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
typedef struct
{
u8 fertilizer_Solenoid_valve; //�ϵ�Ĭ�ϵ�ŷ���Ҫ���ر�
/*��Ӧ���ʵ�ŷ�
bit0  ���ʵ�ŷ�1  0���������ŷ�Ҫ�ر� 1����Ҫ���� ����˳�� 1-->5
bit1  ���ʵ�ŷ�2
bit2  ���ʵ�ŷ�3
bit3  ���ʵ�ŷ�4
bit4  ���ʵ�ŷ�5

*/
u8 water_Solenoid_valve;//��ˮ��ŷ� 0�ر� 1����
u8 water_select;//�����ѡ��
u16 fertilizer_Solenoid1_time; //�ʱ�1����ʱ��
u16 fertilizer_Solenoid2_time; //�ʱ�2����ʱ��
u16 fertilizer_Solenoid3_time; //�ʱ�3����ʱ��
u16 fertilizer_Solenoid4_time; //�ʱ�4����ʱ��
u16 fertilizer_Solenoid5_time; //�ʱ�5����ʱ��
	
u16 water_Solenoid_time;      //ˮ�ÿ���ʱ��


}HMIStruct;
void Task_HMIMonitor ( void * parg );
void NumberToASCII ( u8 *Arry , u16 Number);
void DisCountDown(u16 RemainderMinute);
void MainPageDisSta(u8 State);
void WaterStaShow(u8 State);
void FertilizerStaShow(u8 State);
void SetRadiotube(u8 Num, u8 Value);
void SetSwitchAlarm(void);
void Refresh_ACTION_Page(void);
#endif
