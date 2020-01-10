#ifndef __IO_H
#define __IO_H
#include "sys.h"
#include "rs485.h"
#include "check.h"
#include "delay.h"

u8	IObuf[]={1,2,4,8,0x10,0x20,0x40,0x80};

void	SetIO8(u8 addr,u8 value)				//ÉèÖÃIO8¼ÌµçÆ÷×´Ì¬
{
	u8 control_all[]={0xfe ,0x0F ,0x00 ,0x00 ,0x00 ,0x08 ,0x01 ,0xFF ,0xF1 ,0xD1};
	u16 crc;u8 buf[20];u8 i=0;
	control_all[0]=addr;
	control_all[7]=value;
	crc=mc_check_crc16(control_all,8);
	control_all[8]=crc>>8;
	control_all[9]=crc&0xff;
again:	
	comSendBuf(COM3,control_all,sizeof(control_all));
	delay_ms(200);
	if(COM3GetBuf(buf,7)<4)
	{
		if(++i==50)return;
		goto again;
	}
}

void	IO_OutSet(u8 OutPort, u8 Value)	//µç´Å·§¿ØÖÆ
{
	switch(OutPort)
	{
		case 1:
			if(Value == 0)
			{
				PCout(7) = 0;
			}
			else
			{
				PCout(7) = 1;
			}
			break;
		
		case 2:
			if(Value == 0)
			{
				PCout(8) = 0;
			}
			else
			{
				PCout(8) = 1;
			}
			break;
			
		case 3:
			if(Value == 0)
			{
				PCout(9) = 0;
			}
			else
			{
				PCout(9) = 1;
			}
			break;
		
		case 4:
			if(Value == 0)
			{
				PAout(8) = 0;
			}
			else
			{
				PAout(8) = 1;
			}
			break;
		default:
			break;
	}
}

#endif
