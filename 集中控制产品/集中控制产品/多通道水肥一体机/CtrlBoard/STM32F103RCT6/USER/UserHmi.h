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

#define IDLECOUNTNUM 3000 //5分钟 3000/60/10 = 5

typedef enum _AGVSM_LCDPageNumber
{
	LCD_MAIN_PAGE = 0,          //0 主界面
	LCD_ACTION_PAGE = 1,		//1 待机界面
	LCD_STARTERR_PAGE = 2,		//2 任务配置错误界面
	LCD_FRETERR_PAGE = 3,	//	 3 施肥电磁阀错误界面
	LCD_WORKOVERTIP_PAGE = 4, //4 任务完成提示界面
	LCD_WATERERR_PAGE = 5,		//5灌溉电磁阀错误界面
	LCD_MOTORERR_PAGE = 6			//6吸肥电机错误界面
} AGVSM_LCDPageNumber;
typedef struct
{
	u8 PumpWFlag;	//水泵是否开启
	u8 PumpFFlag;	//肥泵是否开启
	u8 Passageway[6]; //通道阀位控制
	u32 Zone;		//分区阀控制
	u8 WorkHour;	//工作时长(h)
	u8 WorkMinute;	//工作时长(m)
	u8 WorkDay;		//工作周几
	u8 StartHour;	//开始小时
	u8 StartMinute;	//开始分钟
}ProjectStruct;
typedef struct
{
u8 fertilizer_Solenoid_valve; //上电默认电磁阀需要都关闭
/*对应吸肥电磁阀
bit0  吸肥电磁阀1  0代表这个电磁阀要关闭 1代表要开启 开启顺序 1-->5
bit1  吸肥电磁阀2
bit2  吸肥电磁阀3
bit3  吸肥电磁阀4
bit4  吸肥电磁阀5

*/
u8 water_Solenoid_valve;//灌水电磁阀 0关闭 1开启
u8 water_select;//棚分区选择
u16 fertilizer_Solenoid1_time; //肥泵1开启时间
u16 fertilizer_Solenoid2_time; //肥泵2开启时间
u16 fertilizer_Solenoid3_time; //肥泵3开启时间
u16 fertilizer_Solenoid4_time; //肥泵4开启时间
u16 fertilizer_Solenoid5_time; //肥泵5开启时间
	
u16 water_Solenoid_time;      //水泵开启时间


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
