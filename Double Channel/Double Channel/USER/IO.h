#ifndef __IO_H
#define __IO_H
#include "sys.h"
#include "main.h"
#include "rs485.h"
#include "check.h"
#include "delay.h"
void	CheckIO8()						//¶ÁÈ¡IO8ÊäÈë×´Ì¬
{
	u8 check_in[]	={0xfe ,0x02 ,0x00 ,0x00 ,0x00 ,0x08 ,0x6D ,0xC3};
	u8 buf[9];
	u16 crc;
	memset(buf,0,8);
	comClearRxFifo(COM3);
	comSendBuf(COM3,check_in,sizeof(check_in));
	delay_ms(200);
	COM3GetBuf(buf,6);
	crc=mc_check_crc16(buf,4);
	if((buf[4]==(crc>>8))&&(buf[5]==(crc&0xff)))
	IO8STATE=buf[3];
}

void	SetIO8(u8 value)				//ÉèÖÃIO8¼ÌµçÆ÷×´Ì¬
{
	u8 control_all[]={0xfe ,0x0F ,0x00 ,0x00 ,0x00 ,0x08 ,0x01 ,0xFF ,0xF1 ,0xD1};
	u16 crc;
	control_all[7]=value;
	crc=mc_check_crc16(control_all,8);
	control_all[8]=crc>>8;
	control_all[9]=crc&0xff;
	comSendBuf(COM3,control_all,sizeof(control_all));
	delay_ms(200);
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
