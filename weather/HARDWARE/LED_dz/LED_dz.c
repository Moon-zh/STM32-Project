#include "sys.h"	
#include "rs485.h"	 
#include "delay.h"
#include "LED_dz.h"

#define CRC1(crc,byte) (((crc) >> 8 ) ^ tabel[((crc) ^ (unsigned int) (byte)) & 0XFF])  
static const u16  tabel[256] = {
			0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
			0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440, 0XCC01, 0X0CC0, 0X0D80, 0XCD41, 
			0X0F00, 0XCFC1, 0XCE81, 0X0E40, 0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,    
			0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40, 0X1E00, 0XDEC1, 0XDF81, 0X1F40, 
			0XDD01, 0X1DC0, 0X1C80, 0XDC41, 0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,    
			0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040, 0XF001, 0X30C0, 0X3180, 0XF141, 
			0X3300, 0XF3C1, 0XF281, 0X3240, 0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,    
			0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41, 0XFA01, 0X3AC0, 0X3B80, 0XFB41, 
			0X3900, 0XF9C1, 0XF881, 0X3840, 0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,    
			0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40, 0XE401, 0X24C0, 0X2580, 0XE541, 
			0X2700, 0XE7C1, 0XE681, 0X2640, 0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,    
			0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240, 0X6600, 0XA6C1, 0XA781, 0X6740, 
			0XA501, 0X65C0, 0X6480, 0XA441, 0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,    
			0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840, 0X7800, 0XB8C1, 0XB981, 0X7940, 
			0XBB01, 0X7BC0, 0X7A80, 0XBA41, 0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,    
			0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640, 0X7200, 0XB2C1, 0XB381, 0X7340, 
			0XB101, 0X71C0, 0X7080, 0XB041, 0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,    
			0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440, 0X9C01, 0X5CC0, 0X5D80, 0X9D41, 
			0X5F00, 0X9FC1, 0X9E81, 0X5E40, 0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,    
			0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40, 0X4E00, 0X8EC1, 0X8F81, 0X4F40, 
			0X8D01, 0X4DC0, 0X4C80, 0X8C41, 0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,    
			0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040  }; 

u16 CalcCRC(u8 *data, u16 size)  
{  
	u16 i;  
	u16 crc = 0;  
	for (i = 0; i < size; i++) 
	crc = CRC1(crc, data[i]); 
	return crc;  
} 

//#include "check.h"
extern u16 CalcCRC(u8 *data, u16 size);
u8 tab_dz_hand[]={0XA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x02,0x05,0x00,0xA2,0x00,0x01,0x00,0x00,0xBB,0x5F,0x5A};
u8 tab_dz_set1[]={0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x02,0x0E,0x00,0xA1,0x05,0x01,0x00,0x00,0x01,0x50,0x30,0x30,0x31,0x79,0x00,0x00,0x00,0x12,0x7D,0x5A};
u8 tab_dz_set2[]={0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x02,0x0E,0x00,0xA1,0x05,0x01,0x00,0x00,0x01,0x50,0x30,0x30,0x32,0x79,0x00,0x00,0x00,0x56,0x7D,0x5A};	

u8 tab_dz1[]={0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x02,0x8B,0x00,0xA1,0x06,0x01,0x00,0x00,0x50,0x30,0x30,0x31,0x01,0x00,0x00,0x79,0x00,0x00,0x00,
	0x00,0x00,0x00,0x50,0x30,0x30,0x31,0x79,0x00,0x00,0x00,0xFF,0x05,0x00,0x05,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x01,0x5E,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x40,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x01,0x00,0x05,0x0A,0x3F,0x00,0x00,0x00,0xCE,0xC2,
	0xCA,0xD2,0x31,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x43,0x4F,0x32,0xA3,0xBA,0x33,0x31,0x35,0x32,0x70,0x70,0x6D,0x20,0x20,0xBF,0xD5,0xC6,0xF8,
	0xCE,0xC2,0xB6,0xC8,0xA3,0xBA,0x32,0x37,0xA1,0xE6,0xBF,0xD5,0xC6,0xF8,0xCA,0xAA,0xB6,0xC8,0xA3,0xBA,0x33,0x39,0x25,0x52,0x48,0x71,0x7C,0x68,0x6B,0x5A,
	0x00,0x00,0x00,0x00,0x00,0x5A};
u8 tab_dz2[]={0x01,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0x02,0x8B,0x00,0xA1,0x06,0x01,0x00,0x00,0x50,0x30,0x30,0x32,0x01,0x00,0x00,0x79,0x00,0x00,0x00,
	0x00,0x00,0x00,0x50,0x30,0x30,0x32,0x79,0x00,0x00,0x00,0xFF,0x05,0x00,0x05,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x00,0x01,0x5E,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x40,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x02,0x01,0x00,0x05,0x0A,0x3F,0x00,0x00,0x00,0xCE,0xC2,
	0xCA,0xD2,0x31,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xCD,0xC1,0xC8,0xC0,0xCE,0xC2,0xB6,0xC8,0xA3,0xBA,0x32,0x33,0xA1,0xE6,0xCD,0xC1,0xC8,0xC0,
	0xCA,0xAA,0xB6,0xC8,0xA3,0xBA,0x34,0x36,0x25,0x52,0x48,0xB9,0xE2,0xD5,0xD5,0xA3,0xBA,0x32,0x35,0x34,0x31,0x38,0x4C,0x75,0x78,0xD6,0xFB,0x29,0xA8,0x5A,
	0x00,0x00,0x00,0x00,0x00,0x5A};
	
void	senddzcmd(u8 *data,u8 num)
{
	comSendBuf(COM3,data,num);
}

void	setdz1(Environmental data)//78
{
	u8 i=0;
	char *h1="温度:00℃ 无雨雪";	
	char *h2="湿度:00%RH      ";	
	char *h3="光照强度00000Lux";	
	char *h4="大气压: 0000hpa ";	
	u16 crc;

	if(data.snow)
	{
		h1="温度:00℃ 有雨雪";
	}
	else
	{
		h1="温度:00℃ 无雨雪";
	}
	
	for(;*h1;h1++)tab_dz1[88+i++]=*h1;
	for(;*h2;h2++)tab_dz1[88+i++]=*h2;
	for(;*h3;h3++)tab_dz1[88+i++]=*h3;
	for(;*h4;h4++)tab_dz1[88+i++]=*h4;
	
	tab_dz1[42]=10;
	
	tab_dz1[88+sizeof("温度:")-1]													=data.temp/100+0x30;
	tab_dz1[88+sizeof("温度:")]														=data.temp/10%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:")-1]									=data.humi%1000/100+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:")]										=data.humi%100/10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度")-1]					=data.light/10000+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度")]					=data.light/1000%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度")+1]					=data.light/100%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度")+2]					=data.light/10%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度")+3]					=data.light%10+0x30;
//	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度00000Lux大气压:")-1]	=0x30;//data.bmp/10000+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度00000Lux大气压:")]	=data.bmp/100000+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度00000Lux大气压:")+1]	=data.bmp/10000%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度00000Lux大气压:")+2]	=data.bmp/1000%10+0x30;
	tab_dz1[88+sizeof("温度:00℃ 无雨雪湿度:00%RH      光照强度00000Lux大气压:")+3]	=data.bmp/100%10+0x30;
	
	
	crc=CalcCRC(tab_dz1+32,sizeof(tab_dz1)-37);
	tab_dz1[sizeof(tab_dz1)-4]=crc>>8;
	tab_dz1[sizeof(tab_dz1)-5]=crc&0xff;
	
	crc=CalcCRC(tab_dz1,sizeof(tab_dz1)-3);
	tab_dz1[sizeof(tab_dz1)-2]=crc>>8;
	tab_dz1[sizeof(tab_dz1)-3]=crc&0xff;
	
	for(i=0;i<8;i++)comSendChar(COM3,0xA5);
	tab_dz1[sizeof(tab_dz1)-1]=0x5A;
	comSendBuf(COM3,tab_dz1,sizeof(tab_dz1));
}

void	setdz2(Environmental data)
{
	u8 i=0;
	char *h1="风向：西南偏北  ";	
	char *h2="风速:00.0m/s    ";	
	char *h3="CO2:0000ppm     ";	
	char *h4="雨量:00mm       ";	
	u16 crc;
	
	switch(data.winddir)
	{
		case 0:	h1="风向：东北偏北  ";break;
		case 1:	h1="风向：东北      ";break;
		case 2:	h1="风向：东北偏东  ";break;
		case 3:	h1="风向：正东      ";break;
		case 4:	h1="风向：东南偏东  ";break;
		case 5:	h1="风向：东南      ";break;
		case 6:	h1="风向：东南偏南  ";break;
		case 7:	h1="风向：正南      ";break;
		case 8:	h1="风向：西南偏南  ";break;
		case 9:	h1="风向：西南      ";break;
		case 10:h1="风向：西南偏西  ";break;
		case 11:h1="风向：正西      ";break;
		case 12:h1="风向：西北偏西  ";break;
		case 13:h1="风向：西北      ";break;
		case 14:h1="风向：西北偏北  ";break;
		case 15:h1="风向：正北      ";break;
	}

	for(;*h1;h1++)tab_dz2[88+i++]=*h1;
	for(;*h2;h2++)tab_dz2[88+i++]=*h2;
	for(;*h3;h3++)tab_dz2[88+i++]=*h3;
	for(;*h4;h4++)tab_dz2[88+i++]=*h4;
	
	tab_dz2[42]=10;
	
	tab_dz2[88+sizeof("风向  西南偏北  风速:")-1]									=data.windspeed/100+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:")]										=data.windspeed/10%10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:")+2]									=data.windspeed%10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:")-1]					=data.co2/1000+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:")]						=data.co2/100%10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:")+1]					=data.co2/10%10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:")+2]					=data.co2%10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:0000ppm     雨量:")-1]	=data.rain/10+0x30;
	tab_dz2[88+sizeof("风向  西南偏北  风速:000m/s     CO2:0000ppm     雨量:")]		=data.rain%10+0x30;
	
	crc=CalcCRC(tab_dz2+32,sizeof(tab_dz2)-37);
	tab_dz2[sizeof(tab_dz2)-4]=crc>>8;
	tab_dz2[sizeof(tab_dz2)-5]=crc&0xff;
	
	crc=CalcCRC(tab_dz2,sizeof(tab_dz2)-3);
	tab_dz2[sizeof(tab_dz2)-2]=crc>>8;
	tab_dz2[sizeof(tab_dz2)-3]=crc&0xff;
	
	for(i=0;i<8;i++)comSendChar(COM3,0xA5);
	tab_dz2[sizeof(tab_dz2)-1]=0x5A;
	comSendBuf(COM3,tab_dz2,sizeof(tab_dz2));
}

void	setprogram1(Environmental data)
{
	u8 i;
	u8 a[40];
	i=0;
	do
	{
		senddzcmd(tab_dz_set1,sizeof(tab_dz_set1));
		delay_ms(500);
		if(COM3GetBuf(a,35)>30)break;
		if(++i==10)return;
	}while(1);delay_ms(100);
	
	i=0;
	comClearRxFifo(COM3);
	do
	{
		setdz1(data);
		delay_ms(500);
		if(COM3GetBuf(a,35)>30)break;
		if(++i==10)return;
	}
	while(1);delay_ms(100);
}

void	setprogram2(Environmental data)
{
	u8 a[40];
	u8 i;
	i=0;
	do
	{
		senddzcmd(tab_dz_set2,sizeof(tab_dz_set2));
		delay_ms(500);
		if(COM3GetBuf(a,35)>30)break;
		if(++i==10)return;
	}while(1);delay_ms(100);
	
	comClearRxFifo(COM3);
	i=0;
	do
	{
		setdz2(data);
		delay_ms(300);
		if(COM3GetBuf(a,35)>30)break;
		if(++i==10)return;
	}while(1);delay_ms(100);
}

void	setdz()
{
	u8 i=0;
	u8 a[40];
	do
	{
		senddzcmd(tab_dz_hand,sizeof(tab_dz_hand));
		delay_ms(500);
		if(COM3GetBuf(a,35)>30)break;
		if(++i==10)return;
	}while(1);delay_ms(100);
}
