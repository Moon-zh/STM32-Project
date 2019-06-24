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
* ��    �ƣ� UartInit()
* ��    �ܣ� ���ڳ�ʼ��
* ��ڲ����� ��      
* ���ڲ����� ��
****************************************************************************/
void UartInit(uint16_t BaudRate);

/*****************************************************************
* ��    �ƣ� SendChar()
* ��    �ܣ� ����1���ֽ�
* ��ڲ����� t  ���͵��ֽ�       
* ���ڲ����� ��                  
 *****************************************************************/
void  SendChar(uchar t); 

/*****************************************************************
* ��    �ƣ� SendStrings()
* ��    �ܣ� �����ַ���
* ��ڲ����� str  ���͵��ַ���     
* ���ڲ����� ��               
 *****************************************************************/
void SendStrings(uchar *str);


#endif
