#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"
/*
*********************************************************************************************************
*
*	模块名称 : 串口中断+FIFO驱动模块
*	文件名称 : bsp_uart_fifo.h
*	说    明 : 头文件
*
*********************************************************************************************************
*/
#include <stdint.h>
#include "stm32f10x_usart.h"
#include <stdio.h>
#include <string.h>
/*
	如果需要更改串口对应的管脚，请自行修改 bsp_uart_fifo.c文件中的 static void InitHardUart(void)函数
*/

/* 定义使能的串口, 0 表示不使能（不增加代码大小）， 1表示使能 */
	#define	UART1_FIFO_EN	1
	#define	UART2_FIFO_EN	1
	#define	UART3_FIFO_EN	1
	#define	UART4_FIFO_EN	1
	#define	UART5_FIFO_EN	1
	#define	UART6_FIFO_EN	0

	/* RS485芯片发送使能GPIO, PB5 */
	#define RS485_TX_EN0	PBout(14)	//485模式控制.0,接收;1,发送.
	#define RS485_TX_EN1 	PBout(5)	//485模式控制.0,接收;1,发送.
	#define RS485_TX_EN2 	PCout(5)	//485模式控制.0,接收;1,发送.
	
/* 定义端口号 */
typedef enum
{
	COM1 = 0,	/* USART1  PA9, PA10 */
	COM2 = 1,	/* USART2, PA2,PA3  */
	COM3 = 2,	/* USART3, PB10, PB11 */
	COM4 = 3,	/* USART3, PC10, PC11 */
	COM5 = 4,	/* USART3, PC12, PD2 */
	COM6 = 5	/* USART6, PC6, PC7 */
}COM_PORT_E;

/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */
#if UART1_FIFO_EN == 1
	#define UART1_BAUD			115200
	#define UART1_TX_BUF_SIZE	1*256
	#define UART1_RX_BUF_SIZE	350
#endif

#if UART2_FIFO_EN == 1
	#define UART2_BAUD			115200
	#define UART2_TX_BUF_SIZE	200
	#define UART2_RX_BUF_SIZE	200
#endif

#if UART3_FIFO_EN == 1
	#define UART3_BAUD			9600
	#define UART3_TX_BUF_SIZE	250
	#define UART3_RX_BUF_SIZE	250
#endif

#if UART4_FIFO_EN == 1
	#define UART4_BAUD			9600
	#define UART4_TX_BUF_SIZE	1*256
	#define UART4_RX_BUF_SIZE	1*256
#endif

#if UART5_FIFO_EN == 1
  	#define UART5_BAUD			9600
	#define UART5_TX_BUF_SIZE	1*20
	#define UART5_RX_BUF_SIZE	1*256
#endif

#if UART6_FIFO_EN == 1
	#define UART6_BAUD			9600
	#define UART6_TX_BUF_SIZE	1*10
	#define UART6_RX_BUF_SIZE	1*10
#endif

typedef struct
{
	u8 buf[256];
	u8 WritePiont;
	u8 ReadPiont;
}Ring;

/* 串口设备结构体 */
typedef struct
{
	USART_TypeDef *uart;		/* STM32内部串口设备指针 */
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	__IO uint16_t usTxWrite;	/* 发送缓冲区写指针 */
	__IO uint16_t usTxRead;		/* 发送缓冲区读指针 */
	__IO uint16_t usTxCount;	/* 等待发送的数据个数 */

	__IO uint16_t usRxWrite;	/* 接收缓冲区写指针 */
	__IO uint16_t usRxRead;		/* 接收缓冲区读指针 */
	__IO uint16_t usRxCount;	/* 还未读取的新数据个数 */

	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(uint8_t _byte);	/* 串口收到数据的回调函数指针 */
}UART_T;

void bsp_InitUart(void);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comClearTxFifo(COM_PORT_E _ucPort);
void comClearRxFifo(COM_PORT_E _ucPort);
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate);
uint16_t comGetRxAvailableDataSize(COM_PORT_E _ucPort);
void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate);
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen);
void RS485_SendStr(char *_pBuf);
void RS485_SetBaud(uint32_t _baud);
uint8_t COMGetBuf(COM_PORT_E _ucPort , Ring* Circle,uint16_t len);
void u3_printf(char* fmt,...);
void usart3send(u8* buf, u16 uLen);
uint8_t COM5GetBuf(u8* Buf,u8 size);
uint8_t COM4GetBuf(u8* Buf,u8 size);
uint8_t COM3GetBuf(u8* Buf,u8 size);
uint8_t COM1GetBuf(u8* Buf,u16 size);
uint8_t COM2GetBuf(u8* Buf,u8 size);
void Uart1VarInit(void);
extern unsigned char printf_num;

#endif	   
