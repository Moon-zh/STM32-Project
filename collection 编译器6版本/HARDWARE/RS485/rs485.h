#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"
/*
*********************************************************************************************************
*
*	ģ������ : �����ж�+FIFO����ģ��
*	�ļ����� : bsp_uart_fifo.h
*	˵    �� : ͷ�ļ�
*
*********************************************************************************************************
*/
#include <stdint.h>
#include "stm32f10x_usart.h"
#include <stdio.h>
#include <string.h>
/*
	�����Ҫ���Ĵ��ڶ�Ӧ�Ĺܽţ��������޸� bsp_uart_fifo.c�ļ��е� static void InitHardUart(void)����
*/

/* ����ʹ�ܵĴ���, 0 ��ʾ��ʹ�ܣ������Ӵ����С���� 1��ʾʹ�� */
	#define	UART1_FIFO_EN	1
	#define	UART2_FIFO_EN	1
	#define	UART3_FIFO_EN	1
	#define	UART4_FIFO_EN	1
	#define	UART5_FIFO_EN	1
	#define	UART6_FIFO_EN	0

	/* RS485оƬ����ʹ��GPIO, PB5 */
	#define RS485_TX_EN0	PBout(14)	//485ģʽ����.0,����;1,����.
	#define RS485_TX_EN1 	PBout(5)	//485ģʽ����.0,����;1,����.
	#define RS485_TX_EN2 	PCout(5)	//485ģʽ����.0,����;1,����.
	
/* ����˿ں� */
typedef enum
{
	COM1 = 0,	/* USART1  PA9, PA10 */
	COM2 = 1,	/* USART2, PA2,PA3  */
	COM3 = 2,	/* USART3, PB10, PB11 */
	COM4 = 3,	/* USART3, PC10, PC11 */
	COM5 = 4,	/* USART3, PC12, PD2 */
	COM6 = 5	/* USART6, PC6, PC7 */
}COM_PORT_E;

/* ���崮�ڲ����ʺ�FIFO��������С����Ϊ���ͻ������ͽ��ջ�����, ֧��ȫ˫�� */
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

/* �����豸�ṹ�� */
typedef struct
{
	USART_TypeDef *uart;		/* STM32�ڲ������豸ָ�� */
	uint8_t *pTxBuf;			/* ���ͻ����� */
	uint8_t *pRxBuf;			/* ���ջ����� */
	uint16_t usTxBufSize;		/* ���ͻ�������С */
	uint16_t usRxBufSize;		/* ���ջ�������С */
	__IO uint16_t usTxWrite;	/* ���ͻ�����дָ�� */
	__IO uint16_t usTxRead;		/* ���ͻ�������ָ�� */
	__IO uint16_t usTxCount;	/* �ȴ����͵����ݸ��� */

	__IO uint16_t usRxWrite;	/* ���ջ�����дָ�� */
	__IO uint16_t usRxRead;		/* ���ջ�������ָ�� */
	__IO uint16_t usRxCount;	/* ��δ��ȡ�������ݸ��� */

	void (*SendBefor)(void); 	/* ��ʼ����֮ǰ�Ļص�����ָ�루��Ҫ����RS485�л�������ģʽ�� */
	void (*SendOver)(void); 	/* ������ϵĻص�����ָ�루��Ҫ����RS485������ģʽ�л�Ϊ����ģʽ�� */
	void (*ReciveNew)(uint8_t _byte);	/* �����յ����ݵĻص�����ָ�� */
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
