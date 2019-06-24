//************************************版权申明********************************************
//**                             广州大彩光电科技有限公司
//**                             http://www.gz-dc.com
//**-----------------------------------文件信息--------------------------------------------
//** 文件名称:   cmd_queue.h
//** 修改时间:   2012-06-01
//** 文件说明:   指令队列缓冲区
//** 技术支持：  Tel: 020-22077476  Email: hmi@gz-dc.com
// **------------------------------------------------------------------------------------

#ifndef _CMD_QUEUE
#define _CMD_QUEUE
#include "hmi_driver.h"

//缓冲区大小，在资源容许的情况下，尽量设置大一些，防止溢出
#define QUEUE_MAX_SIZE 80

#define CMD_TOUCH_PRESS_RETURN   0X01   //触摸屏按下通知
#define CMD_GIFMOVE_END          0x02
#define CMD_TOUCH_RELEASE_RETURN 0X03  //触摸屏松开通知

#define CMD_TYPE_UPDATE_CONTROL  0XB1  //控件更新通知
#define CMD_TYPE_CHANGE_SCREEN   0XB2  //画面切换通知
#define CMD_GET_REV              0XFE
#define CMD_HAND_SHAKE           0x55

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U16_BIT10(PTR) ((((uint8 *)(PTR))[1]<<8)|((uint8 *)(PTR))[0])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //从缓冲区取32位数据

typedef unsigned char qdata;
typedef unsigned short qsize;

#pragma pack(push)
#pragma pack(1)	//按字节对齐

typedef struct
{
    uint8    cmd_head;  //帧头

    uint8    cmd_type;  //命令类型(UPDATE_CONTROL)
    uint8    ctrl_msg;   //CtrlMsgType-指示消息的类型
    uint16   screen_id;  //产生消息的画面ID
    uint16   control_id;  //产生消息的控件ID
    uint8    control_type; //控件类型

    uint8    param[256];//可变长度参数，最多256个字节

    uint8  cmd_tail[4];   //帧尾
} CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)

/****************************************************************************
* 名    称： queue_reset()
* 功    能： 复位指令接收缓冲区，清空数据
* 入口参数： 无
* 出口参数： 无
****************************************************************************/
extern void queue_reset(void);

/****************************************************************************
* 名    称： queue_push()
* 功    能： 添加一个数据到指令缓冲区
* 入口参数： _data-指令数据
* 出口参数： 无
****************************************************************************/
extern void queue_push(qdata _data);

/****************************************************************************
* 名    称： queue_find_cmd
* 功    能： 从指令缓冲区中查找一条完整的指令
* 入口参数： poffset-指令的开始位置，psize-指令的字节大小
* 出口参数： 无
****************************************************************************/
extern qsize queue_find_cmd(qdata *buffer,qsize buf_len);
qsize soanar_queue_find_cmd(qdata *buffer,qsize buf_len);

#endif

