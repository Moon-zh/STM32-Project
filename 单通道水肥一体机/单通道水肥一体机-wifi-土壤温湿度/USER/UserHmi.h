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

#define IDLECOUNTNUM 3000 //5分钟 3000/60/10 = 5



//#define LCD_STANDBY_PAGE 0  //待机界面


typedef enum _AGVSM_LCDPageNumber	
{
  LCD_MAIN_PAGE = 7,        // 主界面
	LCD_STANDBY_PAGE = 0,		  // 待机界面
	LCD_STATESHOW_PAGE = 4, 	// 正在灌溉界面
	LCD_MODELSEL_PAGE = 1, 		// 模式选择
	LCD_ZONESEL_PAGE = 2, 		// 灌溉分区选择
	LCD_TIMESET_PAGE = 3, 		// 灌溉时长设置
	LCD_PARASET_PAGE = 5,		  // WIFI设备设置
	LCD_PARASAVE_PAGE = 6,		// 保存界面
	LCD_ZONESEL_PAGE_KONG = 7, //灌溉分区选择为空
	LCD_ZIDONG_MOSHI= 8 ,     //自动模式界面
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

void	HDMI_Init(void);	//触摸屏初始化

extern u8 DHCP;
extern u8 DHCP_cq;

extern u8 hmi_cmp_rtc[7];




#endif
