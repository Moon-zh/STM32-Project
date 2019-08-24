#ifndef _CMD_PROCESS_H
#define _CMD_PROCESS_H
#include "hmi_driver.h"

#define NOTIFY_TOUCH_PRESS      0X01   //����������֪ͨ
#define NOTIFY_TOUCH_RELEASE  0X03  //�������ɿ�֪ͨ
#define NOTIFY_WRITE_FLASH_OK  0X0C  //дFLASH�ɹ�
#define NOTIFY_WRITE_FLASH_FAILD  0X0D  //дFLASHʧ��
#define NOTIFY_READ_FLASH_OK  0X0B  //��FLASH�ɹ�
#define NOTIFY_READ_FLASH_FAILD  0X0F  //��FLASHʧ��
#define NOTIFY_MENU                        0X14  //�˵��¼�֪ͨ
#define NOTIFY_TIMER                       0X43  //��ʱ����ʱ֪ͨ
#define NOTIFY_CONTROL                0XB1  //�ؼ�����֪ͨ
#define NOTIFY_READ_RTC               0XF7  //��ȡRTCʱ��
#define MSG_GET_CURRENT_SCREEN 0X01//����ID�仯֪ͨ
#define MSG_GET_DATA                            0X11//�ؼ�����֪ͨ

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //�ӻ�����ȡ32λ����

enum CtrlType
{
	kCtrlUnknown=0x0,
	kCtrlButton=0x10,  //��ť
	kCtrlText,  //�ı�
	kCtrlProgress,  //������
	kCtrlSlider,    //������
	kCtrlMeter,  //�Ǳ�
	kCtrlDropList, //�����б�
	kCtrlAnimation, //����
	kCtrlRTC, //ʱ����ʾ
	kCtrlGraph, //����ͼ�ؼ�
	kCtrlTable, //���ؼ�
	kCtrlMenu,//�˵��ؼ�
	kCtrlSelector,//ѡ��ؼ�
	kCtrlQRCode,//��ά��
};

typedef struct
{
	uint8    cmd_head;  //֡ͷ

	uint8    cmd_type;  //��������(UPDATE_CONTROL)	
	uint8    ctrl_msg;   //CtrlMsgType-ָʾ��Ϣ������
	uint8    screen_id_high;  //������Ϣ�Ļ���ID
	uint8    screen_id_low;
	uint8    control_id_high;  //������Ϣ�Ŀؼ�ID
	uint8    control_id_low;
	uint8    control_type; //�ؼ�����

	uint8    param[256];//�ɱ䳤�Ȳ��������256���ֽ�

	uint8  cmd_tail[4];   //֡β
}CTRL_MSG,*PCTRL_MSG;

void ProcessMessage( PCTRL_MSG msg, uint16 size );

void NotifyScreen(uint16 screen_id);

void NotifyTouchXY(uint8 press,uint16 x,uint16 y);

void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state);

void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str);

void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value);

void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value);

void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value);

void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state);

void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item);

void NotifyTimer(uint16 screen_id, uint16 control_id);

void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length);

void NotifyWriteFlash(uint8 status);

void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second);

#endif
