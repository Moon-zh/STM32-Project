/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   hmi_user_uart.c
** 创建时间:   2011-05-18
** 文件说明:   大彩串口屏驱动函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com
** 修改时间   : 2013-11-28
** 1)          补全指令集里面的所有驱动函数
--------------------------------------------------------------------------------------*/


#include "hmi_driver.h"

//串口发送一字节数据
#define TX_8(P1) SendChar((P1)&0xFF);
//串口发送两个字节数据
#define TX_8X2(P1,P2) TX_8(P1)TX_8(P2)
//串口发送三个字节数据
#define TX_8X3(P1,P2,P3) TX_8X2(P1,P2)TX_8(P2)
//串口发送四个字节数据
#define TX_8X4(P1,P2,P3,P4) TX_8X2(P1,P2) TX_8X2(P3,P4)
//串口发送N个字节的数据
#define TX_8XN(P,N) SendNU8((uint8 *)P,N);

//串口发送一个WORD类型（16位）的数据
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1);
//串口发送二个WORD类型（16位）的数据
#define TX_16X2(P1,P2) TX_16(P1) TX_16(P2);
//串口发送三个WORD类型（16位）的数据
#define TX_16X3(P1,P2,P3) TX_16X2(P1,P2) TX_16(P3);
//串口发送四个WORD类型（16位）的数据
#define TX_16X4(P1,P2,P3,P4) TX_16X2(P1,P2) TX_16X2(P3,P4)
//串口发送五个WORD类型（16位）的数据
#define TX_16X5(P1,P2,P3,P4,P5) TX_16X3(P1,P2,P3) TX_16X2(P4,P5)
//串口发送六个WORD类型（16位）的数据
#define TX_16X7(P1,P2,P3,P4,P5,P6,P7) TX_16X4(P1,P2,P3,P4) TX_16X3(P5,P6,P7)
//串口发送七个WORD类型（16位）的数据
#define TX_16XN(P,N) SendNU16((uint16 *)P,N);
//串口发送N个WORD类型（16位）的数据
#define TX_32(P1) TX_16X2(((P1)>>16),P1)

//COMMAND_BODY-封装基本命令
#define COMMAND_BODY(CMD,BODY) \
	{SendBeginCMD();TX_8(CMD) BODY SendEndCmd();}

//CONTROL_BODY-封装控件更新指令
#define CONTROL_BODY(SUB_CMD,BODY) \
	{SendBeginCMD();TX_8(0xB1)TX_8(SUB_CMD)TX_16X2(screen_id,control_id) BODY SendEndCmd();}

void SendNU8 ( uint8 *pData, uint16 nDataLen )
{
	uint16 i = 0;

	for ( ; i < nDataLen; ++i )
	{
		TX_8 ( pData[i] )
	}
}

void SendNU16 ( uint16 *pData, uint16 nDataLen )
{
	uint16 i = 0;

	for ( ; i < nDataLen; ++i )
	{
		TX_16 ( pData[i] )
	}
}

void SendBeginCMD()
{
	TX_8 ( 0xEE )
}

void SendEndCmd()
{
	TX_32 ( 0xFFFCFFFF )
}
/*******************************************************
下面的函数 setHandShake_1() 和SetHandShake() 是一样的
实现了发送一个完整的串口指令帧 EE 04 FF FC FF FF 的功能
后者把发送的过程已经打包。前者可以看到每个数据的发送过程
********************************************************/
void setHandShake_1()
{
	SendBeginCMD();
	SendChar ( 0x04 );
	SendEndCmd();
}
void SetHandShake()
COMMAND_BODY ( 0x04, )

void SetDeviceReset ()
COMMAND_BODY ( 0x07, TX_8X4 ( 0x35, 0x5a, 0x53, 0xa5 ) )

void SetFcolor ( uint16 fcolor )
COMMAND_BODY ( 0x41, TX_16 ( fcolor ) )

void SetBcolor ( uint16 bcolor )
COMMAND_BODY ( 0x42, TX_16 ( bcolor ) )

void GUI_CleanScreen()
COMMAND_BODY ( 0x01, )

void SetTextSpace ( uint8 x_w, uint8 y_w )
COMMAND_BODY ( 0x43, TX_8X2 ( x_w, y_w ) )

void SetFont_Region ( uint8 enable, uint16 length, uint16 width )
COMMAND_BODY ( 0x45, TX_8 ( enable ) TX_16X2 ( length, width ) )

void SetFilterColor ( uint16 fillcolor_dwon, uint16 fillcolor_up )
COMMAND_BODY ( 0x44, TX_16X2 ( fillcolor_dwon, fillcolor_up ) )

void DisText ( uint16 x, uint16 y, uint8 back, uint8 font, uchar *strings )
COMMAND_BODY ( 0x20, TX_16X2 ( x, y ) TX_8X2 ( back, font ) SendStrings ( strings ); )

void DisCursor ( uint8 enable, uint16 x, uint16 y, uint8 length, uint8 width )
COMMAND_BODY ( 0x21, TX_8 ( enable ) TX_16X2 ( x, y ) TX_8X2 ( length, width ) )

void DisFull_Image ( uint16 image_id, uint8 masken )
COMMAND_BODY ( 0x31, TX_16 ( image_id ) TX_8 ( masken ) )

void DisArea_Image ( uint16 x, uint16 y, uint16 image_id, uint8 masken )
COMMAND_BODY ( 0x32, TX_16X3 ( x, y, image_id ) TX_8 ( masken ) )

void DisCut_Image ( uint16 x, uint16 y, uint16 image_id, uint16 image_x, uint16 image_y, uint16 image_l, uint16 image_w, uint8 masken )
COMMAND_BODY ( 0x33, TX_16X7 ( x, y, image_id, image_x, image_y, image_l, image_w ) TX_8 ( masken ) )

void DisFlashImage ( uint16 x, uint16 y, uint16 flashimage_id, uint8 enable, uint8 playnum )
COMMAND_BODY ( 0x80, TX_16X3 ( x, y, flashimage_id ) TX_8X2 ( enable, playnum ) )

void GUI_FcolorDot ( uint16 x, uint16 y )
COMMAND_BODY ( 0x50, TX_16X2 ( x, y ) )

void GUI_BcolorDot ( uint16 x, uint16 y )
COMMAND_BODY ( 0x58, TX_16X2 ( x, y ) )

void GUI_Line ( uint16 x0, uint16 y0, uint16 x1, uint16 y1 )
COMMAND_BODY ( 0x51, TX_16X4 ( x0, y0, x1, y1 ) )

void GUI_ConSpaceDots ( uint16 x, uint16 Xspace, uint8 *pDot, uint16 dot_cnt )
COMMAND_BODY ( 0x59, TX_16 ( x ) TX_8XN ( pDot, dot_cnt ) )


void GUI_FcolorConOffsetDots ( uint16 x, uint16 y, uint8 *dot_offset, uint16 dot_cnt )
COMMAND_BODY ( 0x75, TX_16X2 ( x, y ) TX_16XN ( dot_offset, dot_cnt ) )

void GUI_BcolorConOffsetDots ( uint16 x, uint16 y, uint8 *dot_offset, uint16 dot_cnt )
COMMAND_BODY ( 0x76, TX_16X2 ( x, y ) TX_16XN ( dot_offset, dot_cnt ) )


void GUI_FcolorConDots ( uint16 *dot, uint16 dot_cnt )
COMMAND_BODY ( 0x68, TX_16XN ( dot, dot_cnt * 2 ) )

void GUI_BcolorConDots ( uint16 *dot, uint16 dot_cnt )
COMMAND_BODY ( 0x69, TX_16XN ( dot, dot_cnt * 2 ) )

void GUI_Circle ( uint16 x, uint16 y, uint16 r )
COMMAND_BODY ( 0x52, TX_16X3 ( x, y, r ) )

void GUI_CircleFill ( uint16 x, uint16 y, uint16 r )
COMMAND_BODY ( 0x53, TX_16X3 ( x, y, r ) )

void GUI_Arc ( uint16 x, uint16 y, uint16 r, uint16 sa, uint16 ea )
COMMAND_BODY ( 0x67, TX_16X5 ( x, y, r, sa, ea ) )

void GUI_Rectangle ( uint16 x0, uint16 y0, uint16 x1, uint16 y1 )
COMMAND_BODY ( 0x54, TX_16X4 ( x0, y0, x1, y1 ) )

void GUI_RectangleFill ( uint16 x0, uint16 y0, uint16 x1, uint16 y1 )
COMMAND_BODY ( 0x55, TX_16X4 ( x0, y0, x1, y1 ) )

void GUI_Ellipse ( uint16 x0, uint16 y0, uint16 x1, uint16 y1 )
COMMAND_BODY ( 0x56, TX_16X4 ( x0, y0, x1, y1 ) )

void GUI_EllipseFill ( uint16 x0, uint16 y0, uint16 x1, uint16 y1 )
COMMAND_BODY ( 0x57, TX_16X4 ( x0, y0, x1, y1 ) )

void SetBackLight ( uint8 light_level )
COMMAND_BODY ( 0x60, TX_8 ( light_level ) )

void SetPowerSaving ( uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8  bl_on_time )
COMMAND_BODY ( 0x77, TX_8X4 ( enable, bl_off_level, bl_on_level, bl_on_time ) )

void SetBuzzer ( uint8 time )
COMMAND_BODY ( 0x61, TX_8 ( time ) )

void Get_Revision ( void )
COMMAND_BODY ( 0xFE, )

void SetTouchScreen ( uint8 cmd )
COMMAND_BODY ( 0x70, TX_8 ( cmd ) )

void SetTouchScreen_Adj()
COMMAND_BODY ( 0x72, )

void TestTouchScreen()
COMMAND_BODY ( 0x73, )

void SetCommBps ( uint8 baudset )
COMMAND_BODY ( 0xA0, TX_8 ( baudset ) )

void SetMatrixKB ( uint8 cmd )
COMMAND_BODY ( 0x79, TX_8 ( cmd ) )

void WriteAddrdData ( uint32 Addr, uint8 *pData, uint16 dat_len )
COMMAND_BODY ( 0x87, TX_32 ( Addr ) TX_8XN ( pData, dat_len ) )

void ReadAddrdData ( uint32 Addr, uint16 Len )
COMMAND_BODY ( 0x88, TX_32 ( Addr ) TX_16 ( Len ) )

void WriteLayer ( uint8 layer )
COMMAND_BODY ( 0xA1, TX_8 ( layer ) )

void DisplyLayer ( uint8 layer )
COMMAND_BODY ( 0xA2, TX_8 ( layer ) )

void CopyLayer ( uint8 src_layer, uint8 dest_layer )
COMMAND_BODY ( 0xA4, TX_8X2 ( src_layer, dest_layer ) )

void ClearLayer ( uint8 layer )
COMMAND_BODY ( 0x05, TX_8 ( layer ) )


void AutoClearLayer_en ( uint8 enable )
COMMAND_BODY ( 0x06, TX_8 ( enable ) )

void SavePrintscreen_Into_Flash ( uint8 Pic_id )
COMMAND_BODY ( 0x46, TX_8 ( Pic_id ) )

void DisplayPrintscreen_In_Flash ( uint8 Pic_id )
COMMAND_BODY ( 0x47, TX_8 ( Pic_id ) )

void GUI_DispRTC ( uint8 enable, uint8 mode, uint8 font, uint16 color, uint16 x, uint16 y )
COMMAND_BODY ( 0x85, TX_8X3 ( enable, mode, font ) TX_16X3 ( color, x, y ) )

void SetRTCTime ( uint8 sec, uint8 min, uint8 hour, uint8 day, uint8 week, uint8  mon, uint8  year )
COMMAND_BODY ( 0x81, TX_8 ( sec ) TX_8 ( min ) TX_8 ( hour ) TX_8 ( day ) TX_8 ( week ) TX_8 ( mon ) TX_8 ( year ) )

void ReadRTCTime ( void )
COMMAND_BODY ( 0x82, )

void SetCountDown ( uint16 screen_id, uint16 control_id, uint32 value )
CONTROL_BODY ( 0x40, TX_32 ( value ) )


void CountDownStart ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x41, )

void CountDownStop ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x42, )


void Locksysconfig ( void )
COMMAND_BODY ( 0x08, TX_8X4 ( 0xa5, 0x5a, 0x5f, 0xf5 ) )

void Unlocksysconfig ( void )
COMMAND_BODY ( 0x09, TX_8X4 ( 0xde, 0xed, 0x13, 0x31 ) )

void GetScreen ( void )
COMMAND_BODY ( 0xB1, TX_8 ( 0x01 ) )

void SetScreen ( uint16 screen_id )
COMMAND_BODY ( 0xB1, TX_8 ( 0x00 ) TX_16 ( screen_id ) )

void SetScreenUpdate ( uint8 enable )
COMMAND_BODY ( 0xB3, TX_8 ( enable ) )

void ShowControl ( uint16 screen_id, uint16 control_id, uint8 visible )
CONTROL_BODY ( 0x03, TX_8 ( visible ) )

void SetButtonValue ( uint16 screen_id, uint16 control_id, uchar state )
CONTROL_BODY ( 0x10, TX_8 ( state ) )

void SetTextValue ( uint16 screen_id, uint16 control_id, uchar *str )
CONTROL_BODY ( 0x10, SendStrings ( str ); )

void SetProgressValue ( uint16 screen_id, uint16 control_id, uint32 value )
CONTROL_BODY ( 0x10, TX_32 ( value ) )

void SetMeterValue ( uint16 screen_id, uint16 control_id, uint32 value )
CONTROL_BODY ( 0x10, TX_32 ( value ) )

void SetSliderValue ( uint16 screen_id, uint16 control_id, uint32 value )
CONTROL_BODY ( 0x10, TX_32 ( value ) )

void GetControlValue ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x11, )

void AnimationStart ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x20, )

void AnimationStop ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x21, )

void AnimationPause ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x22, )

void AnimationPlayFrame ( uint16 screen_id, uint16 control_id, uint8 frame_id )
CONTROL_BODY ( 0x23, TX_8 ( frame_id ) )

void AnimationPlayPrev ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x24, )


void AnimationPlayNext ( uint16 screen_id, uint16 control_id )
CONTROL_BODY ( 0x25, )

void GraphChannelAdd ( uint16 screen_id, uint16 control_id, uint8 channel, uint16 color )
CONTROL_BODY ( 0x30, TX_8 ( channel ) TX_16 ( color ) )

void GraphChannelDel ( uint16 screen_id, uint16 control_id, uint8 channel )
CONTROL_BODY ( 0x31, TX_8 ( channel ) )

void GraphChannelDataAdd ( uint16 screen_id, uint16 control_id, uint8 channel, uint8 *pData, uint16 nDataLen )
CONTROL_BODY ( 0x32, TX_8 ( channel ) TX_16 ( nDataLen ) TX_8XN ( pData, nDataLen ) )

void GraphChannelDataInsert ( uint16 screen_id, uint16 control_id, uint8 channel, uint8 *pData, uint16 nDataLen )
CONTROL_BODY ( 0x35, TX_8 ( channel ) TX_16 ( nDataLen ) TX_8XN ( pData, nDataLen ) )

void GraphChannelDataClear ( uint16 screen_id, uint16 control_id, uint8 channel )
CONTROL_BODY ( 0x33, TX_8 ( channel ) )

void GraphSetViewport ( uint16 screen_id, uint16 control_id, int16 x_offset, uint16 x_mul, int16 y_offset, uint16 y_mul )
CONTROL_BODY ( 0x34, TX_16X4 ( x_offset, x_mul, y_offset, y_mul ) )

void SetTextBlink ( uint16 screen_id, uint16 control_id, uint8 period )
CONTROL_BODY( 0x15, TX_8(period) )

void SetTexFrontColor(uint16 screen_id,uint16 control_id,uint16 color)//设置文本颜色
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x19);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	SendEndCmd();
}

void SetSliderFrontColor(uint16 screen_id,uint16 control_id,uint16 color)//设置进度条颜色
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x19);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	SendEndCmd();
}

void SetTimer(uint16 screen_id,uint16 control_id,uint32 sec)//设置定时器
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x40);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(sec);
	SendEndCmd();
}

void SatrtTimer(uint16 screen_id,uint16 control_id)//启动
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x41);
	TX_16(screen_id);
	TX_16(control_id);
	SendEndCmd();
}

void 	StopTimer(uint16 screen_id,uint16 control_id)//停止
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x42);
	TX_16(screen_id);
	TX_16(control_id);
	SendEndCmd();
}

void 	PauseTimer(uint16 screen_id,uint16 control_id)//暂停
{
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x44);
	TX_16(screen_id);
	TX_16(control_id);
	SendEndCmd();
}
void SetMulSerialText(uint16 screen_id,uint16 control_id,uint8 Num,uint32* Data)
{
	uint8 i=0;
	SendBeginCMD();
	TX_8(0xB1);
	TX_8(0x12);
	TX_16(screen_id);
	for(i = 0;i<Num; i++)
	{
		TX_16(control_id+i);
		if((Data[i]&0xff000000)>0)
		{
			TX_8(00);
			TX_8(04);
			TX_32(Data[i]);
		}
		else if((Data[i]&0x00ff0000)>0)
		{
			TX_8(00);
			TX_8(03);
			TX_8((u8)(Data[i]>>16));
			TX_16((u16)Data[i]);
		}
		else if((Data[i]&0x0000ff00)>0)
		{
			TX_8(00);
			TX_8(02);
			TX_16((u16)Data[i]);
		}
		else if((Data[i]&0x000000ff)>0)
		{
			TX_8(00);
			TX_8(01);
			TX_8((u8)Data[i]);
		}
	}
	SendEndCmd();
}

