#ifndef __SIM800C_H__
#define __SIM800C_H__	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	   
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������ 
//ATK-SIM800C GSM/GPRSģ������	  
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/4/1
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//******************************************************************************** 
//��
/////////////////////////////////////////////////////////////////////////////////// 	
  
#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8	//�ߵ��ֽڽ����궨��

typedef struct 
{
	u8 CurrSta;//��ǰ״̬
	u8 ErrSta;//�������״̬
	u8 ErrTime;//����Ĵ���
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
u8 sim800c_call_test(void);			 //���Ų���
void sim800c_sms_read_test(void);	 //�����Ų���
void sim800c_sms_send_test(void);	 //�����Ų��� 
void sim800c_sms_ui(u16 x,u16 y);	 //���Ų���UI���溯��
u8 sim800c_sms_test(void);			 //���Ų���
void sim800c_spp_ui(u16 x,u16 y);    //��������UI���溯��
u8 sim800c_spp_test(void);           //����spp����
u8 sim800c_spp_mode(u8 mode);        //����ģʽѡ��
void sim800c_mtest_ui(u16 x,u16 y);	 //sim800c������UI
u8 sim800c_gsminfo_show(void);//��ʾGSMģ����Ϣ
void ntp_update(void);               //����ͬ��ʱ��
void sim800c_test(void);			 //sim800c�����Ժ���
void gprs_task(void *pdata);

#endif





