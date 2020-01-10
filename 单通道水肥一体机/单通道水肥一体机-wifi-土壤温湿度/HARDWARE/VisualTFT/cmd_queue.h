

//#ifndef _CMD_QUEUE
//#define _CMD_QUEUE
//#include "hmi_driver.h"

////缓冲区大小，在资源容许的情况下，尽量设置大一些，防止溢出
//#define QUEUE_MAX_SIZE 80

//#define CMD_TOUCH_PRESS_RETURN   0X01   //触摸屏按下通知
//#define CMD_GIFMOVE_END          0x02
//#define CMD_TOUCH_RELEASE_RETURN 0X03  //触摸屏松开通知

//#define CMD_TYPE_UPDATE_CONTROL  0XB1  //控件更新通知
//#define CMD_TYPE_CHANGE_SCREEN   0XB2  //画面切换通知
//#define CMD_GET_REV              0XFE
//#define CMD_HAND_SHAKE           0x55

//#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //从缓冲区取16位数据
//#define PTR2U16_BIT10(PTR) ((((uint8 *)(PTR))[1]<<8)|((uint8 *)(PTR))[0])  //从缓冲区取16位数据
//#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //从缓冲区取32位数据

//typedef unsigned char qdata;
//typedef unsigned short qsize;

//#pragma pack(push)
//#pragma pack(1)	//按字节对齐

//typedef struct
//{
//    uint8    cmd_head;  //帧头

//    uint8    cmd_type;  //命令类型(UPDATE_CONTROL)
//    uint8    ctrl_msg;   //CtrlMsgType-指示消息的类型
//    uint16   screen_id;  //产生消息的画面ID
//    uint16   control_id;  //产生消息的控件ID
//    uint8    control_type; //控件类型

//    uint8    param[256];//可变长度参数，最多256个字节

//    uint8  cmd_tail[4];   //帧尾
//} CTRL_MSG,*PCTRL_MSG;

//#pragma pack(pop)




///****************************************************************************
//* 名    称： queue_reset()
//* 功    能： 复位指令接收缓冲区，清空数据
//* 入口参数： 无
//* 出口参数： 无
//****************************************************************************/
//extern void queue_reset(void);

///****************************************************************************
//* 名    称： queue_push()
//* 功    能： 添加一个数据到指令缓冲区
//* 入口参数： _data-指令数据
//* 出口参数： 无
//****************************************************************************/
//extern void queue_push(qdata _data);

///****************************************************************************
//* 名    称： queue_find_cmd
//* 功    能： 从指令缓冲区中查找一条完整的指令
//* 入口参数： poffset-指令的开始位置，psize-指令的字节大小
//* 出口参数： 无
//****************************************************************************/
//extern qsize queue_find_cmd(qdata *buffer,qsize buf_len);


//#endif





#ifndef _CMD_QUEUE
#define _CMD_QUEUE

#include "hmi_driver.h"

typedef unsigned char qdata;
typedef unsigned short qsize;    //16位数据



#define NOTIFY_TOUCH_PRESS         0X01  //触摸屏按下通知
#define NOTIFY_TOUCH_RELEASE       0X03  //触摸屏松开通知
#define NOTIFY_WRITE_FLASH_OK      0X0C  //写FLASH成功
#define NOTIFY_WRITE_FLASH_FAILD   0X0D  //写FLASH失败
#define NOTIFY_READ_FLASH_OK       0X0B  //读FLASH成功
#define NOTIFY_READ_FLASH_FAILD    0X0F  //读FLASH失败
#define NOTIFY_MENU                0X14  //菜单事件通知
#define NOTIFY_TIMER               0X43  //定时器超时通知
#define NOTIFY_CONTROL             0XB1  //控件更新通知
#define NOTIFY_READ_RTC            0XF7  //读取RTC时间
#define MSG_GET_CURRENT_SCREEN     0X01  //画面ID变化通知
#define MSG_GET_DATA               0X11  //控件数据通知
#define NOTIFY_HandShake           0X55  //握手通知

#define PTR2U16(PTR) ((((uint8 *)(PTR))[0]<<8)|((uint8 *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((uint8 *)(PTR))[0]<<24)|(((uint8 *)(PTR))[1]<<16)|(((uint8 *)(PTR))[2]<<8)|((uint8 *)(PTR))[3])  //从缓冲区取32位数据


enum CtrlType
{
    kCtrlUnknown=0x0,
    kCtrlButton=0x10,                     //按钮
    kCtrlText,                            //文本
    kCtrlProgress,                        //进度条
    kCtrlSlider,                          //滑动条
    kCtrlMeter,                            //仪表
    kCtrlDropList,                        //下拉列表
    kCtrlAnimation,                       //动画
    kCtrlRTC,                             //时间显示
    kCtrlGraph,                           //曲线图控件
    kCtrlTable,                           //表格控件
    kCtrlMenu,                            //菜单控件
    kCtrlSelector,                        //选择控件
    kCtrlQRCode,                          //二维码
};

#pragma pack(push)
#pragma pack(1)                           //按字节对齐

typedef struct
{
    uint8    cmd_head;                    //帧头

    uint8    cmd_type;                    //命令类型(UPDATE_CONTROL)    
    uint8    ctrl_msg;                    //CtrlMsgType-指示消息的类型
    uint16   screen_id;                   //产生消息的画面ID
    uint16   control_id;                  //产生消息的控件ID
    uint8    control_type;                //控件类型

    uint8    param[256];                  //可变长度参数，最多256个字节

    uint8  cmd_tail[4];                   //帧尾
}CTRL_MSG,*PCTRL_MSG;

#pragma pack(pop)




/*! 
*  \brief  清空指令数据
*/
extern void queue_reset(void);

/*! 
* \brief  添加指令数据
* \detial 串口接收的数据，通过此函数放入指令队列 
*  \param  _data 指令数据
*/
extern void queue_push(qdata _data);

/*! 
*  \brief  从指令队列中取出一条完整的指令
*  \param  cmd 指令接收缓存区
*  \param  buf_len 指令接收缓存区大小
*  \return  指令长度，0表示队列中无完整指令
*/
extern qsize queue_find_cmd(qdata *cmd,qsize buf_len);

#endif

