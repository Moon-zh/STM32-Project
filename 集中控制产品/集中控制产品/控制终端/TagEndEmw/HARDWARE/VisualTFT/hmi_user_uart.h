#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f10x_usart.h"

#define uchar    unsigned char
#define uint8    unsigned char
#define uint16   unsigned short int
#define uint32   unsigned long

#define int16    short int

#define HMI_UART UART5


/****************************************************************************
* 名    称： UartInit()
* 功    能： 串口初始化
* 入口参数： 无      
* 出口参数： 无
****************************************************************************/
void UartInit(uint16_t BaudRate);

/*****************************************************************
* 名    称： SendChar()
* 功    能： 发送1个字节
* 入口参数： t  发送的字节       
* 出口参数： 无                  
 *****************************************************************/
void  SendChar(uchar t); 

/*****************************************************************
* 名    称： SendStrings()
* 功    能： 发送字符串
* 入口参数： str  发送的字符串     
* 出口参数： 无               
 *****************************************************************/
void SendStrings(uchar *str);


#endif
