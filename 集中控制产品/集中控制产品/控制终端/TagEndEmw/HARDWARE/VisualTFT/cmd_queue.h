//************************************��Ȩ����********************************************
//**                             ���ݴ�ʹ��Ƽ����޹�˾
//**                             http://www.gz-dc.com
//**-----------------------------------�ļ���Ϣ--------------------------------------------
//** �ļ�����:   cmd_queue.h
//** �޸�ʱ��:   2012-06-01
//** �ļ�˵��:   ָ����л�����
//** ����֧�֣�  Tel: 020-22077476  Email: hmi@gz-dc.com
// **------------------------------------------------------------------------------------

#ifndef _CMD_QUEUE
#define _CMD_QUEUE
#include "hmi_driver.h"

//��������С������Դ���������£��������ô�һЩ����ֹ���
#define QUEUE_MAX_SIZE 80

#define CMD_TOUCH_PRESS_RETURN   0X01   //����������֪ͨ
#define CMD_GIFMOVE_END          0x02
#define CMD_TOUCH_RELEASE_RETURN 0X03  //�������ɿ�֪ͨ

#define CMD_TYPE_UPDATE_CONTROL  0XB1  //�ؼ�����֪ͨ
#define CMD_TYPE_CHANGE_SCREEN   0XB2  //�����л�֪ͨ
#define CMD_GET_REV              0XFE
#define CMD_HAND_SHAKE           0x55

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //�ӻ�����ȡ16λ����
#define PTR2U16_BIT10(PTR) ((((uint8 *)(PTR))[1]<<8)|((uint8 *)(PTR))[0])  //�ӻ�����ȡ16λ����
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //�ӻ�����ȡ32λ����

typedef unsigned char qdata;
typedef unsigned short qsize;

#pragma pack(push)
#pragma pack(1)	//���ֽڶ���

typedef struct
{
    uint8    cmd_head;  //֡ͷ

    uint8    cmd_type;  //��������(UPDATE_CONTROL)
    uint8    ctrl_msg;   //CtrlMsgType-ָʾ��Ϣ������
    uint16   screen_id;  //������Ϣ�Ļ���ID
    uint16   control_id;  //������Ϣ�Ŀؼ�ID
    uint8    control_type; //�ؼ�����

    uint8    param[256];//�ɱ䳤�Ȳ��������256���ֽ�

    uint8  cmd_tail[4];   //֡β
} CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)

/****************************************************************************
* ��    �ƣ� queue_reset()
* ��    �ܣ� ��λָ����ջ��������������
* ��ڲ����� ��
* ���ڲ����� ��
****************************************************************************/
extern void queue_reset(void);

/****************************************************************************
* ��    �ƣ� queue_push()
* ��    �ܣ� ���һ�����ݵ�ָ�����
* ��ڲ����� _data-ָ������
* ���ڲ����� ��
****************************************************************************/
extern void queue_push(qdata _data);

/****************************************************************************
* ��    �ƣ� queue_find_cmd
* ��    �ܣ� ��ָ������в���һ��������ָ��
* ��ڲ����� poffset-ָ��Ŀ�ʼλ�ã�psize-ָ����ֽڴ�С
* ���ڲ����� ��
****************************************************************************/
extern qsize queue_find_cmd(qdata *buffer,qsize buf_len);
qsize soanar_queue_find_cmd(qdata *buffer,qsize buf_len);

#endif

