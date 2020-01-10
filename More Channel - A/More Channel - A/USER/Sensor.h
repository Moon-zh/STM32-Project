#ifndef __sensor_H
#define __sensor_H
#include "sys.h"
#include "rs485.h"
#include "check.h"
#include "delay.h"
#include "main.h"

void	sendcmd_read(u8 addr,u8 CMD,u8 num)	//发送传感器读取指令
{
	u8 check_value[]={0x01,0x03,0x00,0x00,0x00,0x09,0xc4,0xb0};
	u8 i;
	u16 crc;
	comClearRxFifo(COM4);
	check_value[0]=addr;
	check_value[2]=CMD>>8;
	check_value[3]=CMD&0xff;
	check_value[5]=num;
	crc=mc_check_crc16(check_value,6);
	check_value[6]=crc>>8;
	check_value[7]=crc&0xff;
	for(i=0;i<8;i++)
	comSendChar(COM4,check_value[i]);
	delay_ms(60);
}

u16		readcmd1(u8 i)					//读取空气温度
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM4GetBuf(a,25);
	if(len<20)return 0;
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		sensor[i].airhumi=(a[3]<<8)|a[4];
		if(sensor[i].airhumi>990)sensor[i].airhumi=990;
		sensor[i].airtemp=(a[5]<<8)|a[6];
		sensor[i].CO2=(a[13]<<8)|a[14];
		sensor[i].light=(((a[17]<<8)|a[18])<<16)|((a[19]<<8)|a[20]);
		return 1;
	}
	return 0;
}

u16		readcmd2(u8 i)					//读取土壤温度
{
	u16 crc;
	u8 a[26];
	u8 len;
	len=COM4GetBuf(a,25);
	if(len<12)return 0;
	comClearRxFifo(COM4);
	crc=mc_check_crc16(a,len-2);
	if((a[len-2]==(crc>>8))&&(a[len-1]==(crc&0xff)))
	{
		sensor[i].soilhumi=(a[3]<<8)|a[4];
		if(sensor[i].soilhumi>990)sensor[i].soilhumi=990;
		sensor[i].soiltemp=(a[5]<<8)|a[6];
		sensor[i].EC=(a[9]<<8)|a[10];
		return 1;
	}
	return 0;
}

void	ReadValue()						//传感器读取
{
	u16 k;
	u8 	i,b;
	for(i=0;i<sensor_num;i++)
	{
		b=0;
		do{
			sendcmd_read(1+i,CMD_AIRHUMI,9);			//璇诲彇绌烘皵
			delay_ms(200);
			k=readcmd1(i);
			if(k)break;
			if(++b==5)break;
		}while(1);
		if(b==5)AIRERROR=i+1;
		else	AIRERROR=0;
		
		b=0;
		do{
			sendcmd_read(50+i,CMD_SOILHUMI,4);			//璇诲彇鍦熷￥
			delay_ms(200);
			k=readcmd2(i);
			if(k)break;
			if(++b==5)break;
		}while(1);
		if(b==5)SOILERROR=i+1;
		else	SOILERROR=0;
	}
}
#endif
