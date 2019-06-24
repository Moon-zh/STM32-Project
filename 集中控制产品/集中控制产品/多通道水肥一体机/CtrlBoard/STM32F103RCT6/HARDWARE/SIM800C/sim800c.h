#ifndef __SIM800C_H__
#define __SIM800C_H__	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	   
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板 
//ATK-SIM800C GSM/GPRS模块驱动	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/4/1
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//******************************************************************************** 
//无
/////////////////////////////////////////////////////////////////////////////////// 	
  
#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8	//高低字节交换宏定义

typedef struct 
{
	u8 CurrSta;//当前状态
	u8 ErrSta;//出错误的状态
	u8 ErrTime;//出错的次数
}StateMonitor;

typedef struct
{
	u8 buf[256];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;

#define AT_TEST 1
#define AT_AE1 2
#define AT_IMEI 3
#define AT_AE0 4
#define AT_ATCOMMAND 5
#define AT_SIMCHECK 6
#define AT_CSQ 7
#define AT_POW 8
#define AT_SHUT 9
#define AT_CLOSED 10
#define AT_CLASS 11
#define AT_PDP 12
#define AT_ATTACHMENT 13
#define AT_CMNET 14
#define AT_DATADIS 15
#define AT_SENDMODE 16
#define AT_CONNECT 17

void sim_send_sms(u8*phonenumber,u8*msg);
void sim_at_response(u8 mode);	
u8* sim800c_check_cmd(u8 *str);
u8 sim800c_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 sim800c_wait_request(u8 *request ,u16 waittime);
u8 sim800c_chr2hex(u8 chr);
u8 sim800c_hex2chr(u8 hex);
void sim800c_unigbk_exchange(u8 *src,u8 *dst,u8 mode);
void sim800c_load_keyboard(u16 x,u16 y,u8 **kbtbl);
void sim800c_key_staset(u16 x,u16 y,u8 keyx,u8 sta);
u8 sim800c_get_keynum(u16 x,u16 y);
u8 sim800c_call_test(void);			 //拨号测试
void sim800c_sms_read_test(void);	 //读短信测试
void sim800c_sms_send_test(void);	 //发短信测试 
void sim800c_sms_ui(u16 x,u16 y);	 //短信测试UI界面函数
u8 sim800c_sms_test(void);			 //短信测试
void sim800c_spp_ui(u16 x,u16 y);    //蓝牙测试UI界面函数
u8 sim800c_spp_test(void);           //蓝牙spp测试
u8 sim800c_spp_mode(u8 mode);        //连接模式选择
void sim800c_mtest_ui(u16 x,u16 y);	 //sim800c主测试UI
u8 sim800c_gsminfo_show(void);//显示GSM模块信息
void ntp_update(void);               //网络同步时间
void sim800c_test(void);			 //sim800c主测试函数
void gprs_task(void *pdata);

#endif





