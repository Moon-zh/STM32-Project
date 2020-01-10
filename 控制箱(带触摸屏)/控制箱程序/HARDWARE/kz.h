#ifndef __kz_H
#define __kz_H
#include "delay.h"
#include "sys.h"
#include "includes.h"
#include "rs485.h"
#include "check.h"
/*0XFE为通用地址 本程序用0xfe与1作为通讯地址*/

u8 tabcmd[]={0XFD,0X05,0X00,0X00,0X00,0X00,0XD9,0XC5};

void	sendcmd_kz(u8 *cmd_kz)
{
	u8 i;
	u16 crc;
	crc=mc_check_crc16(cmd_kz,6);
	cmd_kz[6]=crc>>8;
	cmd_kz[7]=crc&0xff;
	for(i=0;i<8;i++)
	comSendChar(COM4,cmd_kz[i]);
	delay_ms(150);
}

void	curtainstop()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=0;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=1;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	curtainopen()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=0;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=1;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	curtainclose()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=0;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=1;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	fan1stop()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=2;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=3;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan1open()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=2;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=3;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan1close()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=2;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=3;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	fan2stop()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=4;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=5;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan2open()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=4;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=5;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan2close()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=4;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=5;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	fan3stop()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=6;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=7;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan3open()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=6;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=7;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	fan3close()
{
	tabcmd[0]=0xFD;
	tabcmd[3]=6;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
	
	tabcmd[3]=7;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	filllight1open()
{
	tabcmd[0]=1;
	tabcmd[3]=0;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	filllight1close()
{
	tabcmd[0]=1;
	tabcmd[3]=0;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	filllight2open()
{
	tabcmd[0]=1;
	tabcmd[3]=1;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	filllight2close()
{
	tabcmd[0]=1;
	tabcmd[3]=1;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	filllight3open()
{
	tabcmd[0]=1;
	tabcmd[3]=2;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	filllight3close()
{
	tabcmd[0]=1;
	tabcmd[3]=2;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	moregasopen()
{
	tabcmd[0]=1;
	tabcmd[3]=3;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	moregasclose()
{
	tabcmd[0]=1;
	tabcmd[3]=3;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

void	sprayopen()
{
	tabcmd[0]=1;
	tabcmd[3]=4;
	tabcmd[4]=0;
	sendcmd_kz(tabcmd);
}

void	sprayclose()
{
	tabcmd[0]=1;
	tabcmd[3]=4;
	tabcmd[4]=0xff;
	sendcmd_kz(tabcmd);
}

#endif
