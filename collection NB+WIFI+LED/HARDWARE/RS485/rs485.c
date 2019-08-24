#include "sys.h"
#include "Includes.h"
#include "rs485.h"
#include "delay.h"
#include "cmd_queue.h"

#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
    switch(printf_num)
    {
    case 1:
        while((USART1->SR&0X40)==0);//循环发送,直到发送完毕
        USART1->DR = (u8) ch;
        return ch;
    case 2:
        while((USART2->SR&0X40)==0);//循环发送,直到发送完毕
        USART2->DR = (u8) ch;
        return ch;
    }
    return 0;
}

void _ttywrch(int ch)
{
    ch = ch;
}
#endif

//#ifdef EN_USART2_RX   	//如果使能了接收

static void UartIRQ(UART_T *_pUart);
/* 使能在源文件中使用uCOS-III的函数, 这里的源文件主要是指BSP驱动文件 */
#define uCOS_EN       1

#if uCOS_EN == 1
#include "os_cpu.h"

#define  ENABLE_INT()  	  OS_EXIT_CRITICAL();     /* 使能全局中断 */
#define  DISABLE_INT()    OS_ENTER_CRITICAL()    /* 禁止全局中断 */
#else
/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
#endif

/* 定义每个串口结构体变量 */
#if UART1_FIFO_EN == 1
UART_T g_tUart1;
uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART2_FIFO_EN == 1
UART_T g_tUart2;
uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART3_FIFO_EN == 1
UART_T g_tUart3;
uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART4_FIFO_EN == 1
UART_T g_tUart4;
uint8_t g_TxBuf4[UART4_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART5_FIFO_EN == 1
UART_T g_tUart5;
uint8_t g_TxBuf5[UART5_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if UART6_FIFO_EN == 1
UART_T g_tUart6;
static uint8_t g_TxBuf6[UART6_TX_BUF_SIZE];		/* 发送缓冲区 */
uint8_t g_RxBuf6[UART6_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

static void UartVarInit(void);
static void InitHardUart(void);
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
static void ConfigUartNVIC(void);
void RS485_InitTXE(void);
/*
*********************************************************************************************************
*	模块名称 : 串口中断+FIFO驱动模块
*	文件名称 : bsp_uart_fifo.c
*	版    本 : V1.0
*	说    明 : 采用串口中断+FIFO模式实现多个串口的同时访问
*********************************************************************************************************
*/
#include "Rs485.h"
/*
*********************************************************************************************************
*	函 数 名: bsp_InitUart
*	功能说明: 初始化串口硬件，并对全局变量赋初值.
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
    UartVarInit();		/* 必须先初始化全局变量,再配置硬件 */
    RS485_InitTXE();	/* 配置RS485芯片的发送使能硬件，配置为推挽输出 */
    InitHardUart();		/* 配置串口的硬件参数(波特率等) */
    ConfigUartNVIC();	/* 配置串口中断 */
}

/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为UART指针
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: uart指针
*********************************************************************************************************
*/
UART_T *ComToUart(COM_PORT_E _ucPort)
{
    if (_ucPort == COM1)
    {
#if UART1_FIFO_EN == 1
        return &g_tUart1;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM2)
    {
#if UART2_FIFO_EN == 1
        return &g_tUart2;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM3)
    {
#if UART3_FIFO_EN == 1
        return &g_tUart3;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM4)
    {
#if UART4_FIFO_EN == 1
        return &g_tUart4;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM5)
    {
#if UART5_FIFO_EN == 1
        return &g_tUart5;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM6)
    {
#if UART6_FIFO_EN == 1
        return &g_tUart6;
#else
        return 0;
#endif
    }
    else
    {
        /* 不做任何处理 */
        return 0;
    }
}
/*
*********************************************************************************************************
*	函 数 名: GetUartRxAvailableDataSize
*	功能说明: 获取串口接收缓冲区当前有效字节数 （用于主程序调用）
*	形    参: _ucPort: 端口号(COM1 - COM6)
*
*	返 回 值: 0 表示无数据  非0表示当前有效字节数
*********************************************************************************************************
*/
uint16_t comGetRxAvailableDataSize(COM_PORT_E _ucPort)
{
    UART_T *pUart;

    pUart = ComToUart(_ucPort);

    if (pUart == 0)
    {
        return 0;
    }
    else
    {
        return (pUart->usRxCount);
    }
}
/*
*********************************************************************************************************
*	函 数 名: ComToUart
*	功能说明: 将COM端口号转换为 USART_TypeDef* USARTx
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: USART_TypeDef*,  USART1, USART2, USART3, UART4, UART5
*********************************************************************************************************
*/
USART_TypeDef *ComToUSARTx(COM_PORT_E _ucPort)
{
    if (_ucPort == COM1)
    {
#if UART1_FIFO_EN == 1
        return USART1;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM2)
    {
#if UART2_FIFO_EN == 1
        return USART2;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM3)
    {
#if UART3_FIFO_EN == 1
        return USART3;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM4)
    {
#if UART4_FIFO_EN == 1
        return UART4;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM5)
    {
#if UART5_FIFO_EN == 1
        return UART5;
#else
        return 0;
#endif
    }
    else if (_ucPort == COM6)
    {
#if UART6_FIFO_EN == 1
        return USART6;
#else
        return 0;
#endif
    }
    else
    {
        /* 不做任何处理 */
        return 0;
    }
}
/*
*********************************************************************************************************
*	函 数 名: comSendBuf
*	功能说明: 向串口发送一组数据。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucaBuf: 待发送的数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen)
{
    UART_T *pUart;

    pUart = ComToUart(_ucPort);
    if (pUart == 0)
    {
        return;
    }
    //USART_GetFlagStatus(USART2,USART_FLAG_TC);//
    if (pUart->SendBefor != 0)
    {
        pUart->SendBefor();		/* 如果是RS485通信，可以在这个函数中将RS485设置为发送模式 */
    }

    UartSend(pUart, _ucaBuf, _usLen);
}
/*
*********************************************************************************************************
*	函 数 名: comSendChar
*	功能说明: 向串口发送1个字节。数据放到发送缓冲区后立即返回，由中断服务程序在后台完成发送
*	形    参: _ucPort: 端口号(COM1 - COM6)
*			  _ucByte: 待发送的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
    comSendBuf(_ucPort, &_ucByte, 1);
}
/*
*********************************************************************************************************
*	函 数 名: comGetChar
*	功能说明: 从接收缓冲区读取1字节，非阻塞。无论有无数据均立即返回。
*	形    参: _ucPort: 端口号(COM1 - COM5)
*			  _pByte: 接收到的数据存放在这个地址
*	返 回 值: 0 表示无数据, 1 表示读取到有效字节
*********************************************************************************************************
*/
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte)
{
    UART_T *pUart;

    pUart = ComToUart(_ucPort);
    if (pUart == 0)
    {
        return 0;
    }

    return UartGetChar(pUart, _pByte);
}
/*
*********************************************************************************************************
*	函 数 名: comClearTxFifo
*	功能说明: 清零串口发送缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearTxFifo(COM_PORT_E _ucPort)
{
    UART_T *pUart;

    pUart = ComToUart(_ucPort);
    if (pUart == 0)
    {
        return;
    }

    pUart->usTxWrite = 0;
    pUart->usTxRead = 0;
    pUart->usTxCount = 0;
}
/*
*********************************************************************************************************
*	函 数 名: comClearRxFifo
*	功能说明: 清零串口接收缓冲区
*	形    参: _ucPort: 端口号(COM1 - COM6)
*	返 回 值: 无
*********************************************************************************************************
*/
void comClearRxFifo(COM_PORT_E _ucPort)
{
    UART_T *pUart;

    pUart = ComToUart(_ucPort);
    if (pUart == 0)
    {
        return;
    }

    pUart->usRxWrite = 0;
    pUart->usRxRead = 0;
    pUart->usRxCount = 0;
}
/*
*********************************************************************************************************
*	函 数 名: comSetBaud
*	功能说明: 设置串口的波特率
*	形    参: _ucPort: 端口号(COM1 - COM5)
*			  _BaudRate: 波特率，0-4500000， 最大4.5Mbps
*	返 回 值: 无
*********************************************************************************************************
*/
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate)
{
    USART_TypeDef* USARTx;

    USARTx = ComToUSARTx(_ucPort);
    if (USARTx == 0)
    {
        return;
    }
    USART_SetBaudRate(USARTx, _BaudRate);
}
/*
*********************************************************************************************************
*	函 数 名: USART_SetBaudRate
*	功能说明: 修改波特率寄存器，不更改其他设置。如果使用 USART_Init函数, 则会修改硬件流控参数和RX,TX配置
*			  根据固件库中 USART_Init函数，将其中配置波特率的部分单独提出来封装为一个函数
*	形    参: USARTx : USART1, USART2, USART3, UART4, UART5
*			  BaudRate : 波特率，取值 0 - 4500000
*	返 回 值: 无
*********************************************************************************************************
*/
void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate)
{
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    RCC_ClocksTypeDef RCC_ClocksStatus;

    /* Check the parameters */
    assert_param(IS_USART_ALL_PERIPH(USARTx));
    assert_param(IS_USART_BAUDRATE(BaudRate));

    /*---------------------------- USART BRR Configuration -----------------------*/
    /* Configure the USART Baud Rate */
    RCC_GetClocksFreq(&RCC_ClocksStatus);

    if ((USARTx == USART1))
    {
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    }
    else
    {
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }

    /* Determine the integer part */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
    {
        /* Integer part computing in case Oversampling mode is 8 Samples */
        integerdivider = ((25 * apbclock) / (2 * (BaudRate)));
    }
    else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
    {
        /* Integer part computing in case Oversampling mode is 16 Samples */
        integerdivider = ((25 * apbclock) / (4 * (BaudRate)));
    }
    tmpreg = (integerdivider / 100) << 4;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

    /* Implement the fractional part in the register */
    if ((USARTx->CR1 & USART_CR1_OVER8) != 0)
    {
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
    }
    else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
    {
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    }

    /* Write to USART BRR register */
    USARTx->BRR = (uint16_t)tmpreg;
}

/*
*********************************************************************************************************
*	函 数 名: RS485_InitTXE
*	功能说明: 配置RS485发送使能口线 TXE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_14;	 //PB5 PB14端口配置
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 //推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //PC5端口配置
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 //推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
    }
}
/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_0_SendBefor(void)
{
    RS485_TX_EN0=1;
    //RS485_TX_1_EN();	/* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_0_SendOver(void)
{
    RS485_TX_EN0=0;	/* 切换RS485收发芯片为接收模式 */
}

/*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_1_SendBefor(void)
{
    RS485_TX_EN1=1;
    //RS485_TX_1_EN();	/* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_1_SendOver(void)
{
    RS485_TX_EN1=0;	/* 切换RS485收发芯片为接收模式 */
}


/*
*********************************************************************************************************
*	函 数 名: RS485_SendBefor
*	功能说明: 发送数据前的准备工作。对于RS485通信，请设置RS485芯片为发送状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendBefor = RS485_SendBefor
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_2_SendBefor(void)
{
    RS485_TX_EN2=1; /* 切换RS485收发芯片为发送模式 */
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendOver
*	功能说明: 发送一串数据结束后的善后处理。对于RS485通信，请设置RS485芯片为接收状态，
*			  并修改 UartVarInit()中的函数指针等于本函数名，比如 g_tUart2.SendOver = RS485_SendOver
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_2_SendOver(void)
{
    RS485_TX_EN2=0;	/* 切换RS485收发芯片为接收模式 */
}

/*
*********************************************************************************************************
*	函 数 名: UartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
    g_tUart1.uart = USART1;						/* STM32 串口设备 */
    g_tUart1.pTxBuf = g_TxBuf1;					/* 发送缓冲区指针 */
    g_tUart1.pRxBuf = g_RxBuf1;					/* 接收缓冲区指针 */
    g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart1.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart1.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart1.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart1.usTxCount = 0;						/* 待发送的数据个数 */
    g_tUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
    g_tUart1.SendOver = 0;						/* 发送完毕后的回调函数 */
    g_tUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
#endif

#if UART2_FIFO_EN == 1
    g_tUart2.uart = USART2;						/* STM32 串口设备 */
    g_tUart2.pTxBuf = g_TxBuf2;					/* 发送缓冲区指针 */
    g_tUart2.pRxBuf = g_RxBuf2;					/* 接收缓冲区指针 */
    g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart2.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart2.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart2.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart2.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart2.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart2.usTxCount = 0;						/* 待发送的数据个数 */
    g_tUart2.SendBefor = 0;						/* 发送数据前的回调函数 */
    g_tUart2.SendOver = 0;						/* 发送完毕后的回调函数 */
    g_tUart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
#endif

#if UART3_FIFO_EN == 1
    g_tUart3.uart = USART3;						/* STM32 串口设备 */
    g_tUart3.pTxBuf = g_TxBuf3;					/* 发送缓冲区指针 */
    g_tUart3.pRxBuf = g_RxBuf3;					/* 接收缓冲区指针 */
    g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart3.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart3.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart3.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart3.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart3.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart3.usTxCount = 0;						/* 待发送的数据个数 */
    g_tUart3.SendBefor = RS485_0_SendBefor;		/* 发送数据前的回调函数 */
    g_tUart3.SendOver = RS485_0_SendOver;		/* 发送完毕后的回调函数 */
    g_tUart3.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
#endif

#if UART4_FIFO_EN == 1
    g_tUart4.uart = UART4;						/* STM32 串口设备 */
    g_tUart4.pTxBuf = g_TxBuf4;					/* 发送缓冲区指针 */
    g_tUart4.pRxBuf = g_RxBuf4;					/* 接收缓冲区指针 */
    g_tUart4.usTxBufSize = UART4_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart4.usRxBufSize = UART4_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart4.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart4.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart4.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart4.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart4.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart4.usTxCount = 0;						/* 待发送的数据个数 */
    g_tUart4.SendBefor = RS485_1_SendBefor;		/* 发送数据前的回调函数 */
    g_tUart4.SendOver = RS485_1_SendOver;		/* 发送完毕后的回调函数 */
    //g_tUart4.SendBefor = 0;						/* 发送数据前的回调函数 */
    //g_tUart4.SendOver = 0;						/* 发送完毕后的回调函数 */
    g_tUart4.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
#endif

#if UART5_FIFO_EN == 1
    g_tUart5.uart = UART5;						/* STM32 串口设备 */
    g_tUart5.pTxBuf = g_TxBuf5;					/* 发送缓冲区指针 */
    g_tUart5.pRxBuf = g_RxBuf5;					/* 接收缓冲区指针 */
    g_tUart5.usTxBufSize = UART5_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart5.usRxBufSize = UART5_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart5.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart5.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart5.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart5.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart5.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart5.usTxCount = 0;						/* 待发送的数据个数 */
    //g_tUart5.SendBefor = RS485_2_SendBefor;		/* 发送数据前的回调函数 */
    //g_tUart5.SendOver  = RS485_2_SendOver;			/* 发送完毕后的回调函数 */
    g_tUart5.SendBefor = 0;						/* 发送数据前的回调函数 */
    g_tUart5.SendOver  = 0;						/* 发送完毕后的回调函数 */
    g_tUart5.ReciveNew =0;						/* 接收到新数据后的回调函数 */
#endif


#if UART6_FIFO_EN == 1
    g_tUart6.uart = USART6;						/* STM32 串口设备 */
    g_tUart6.pTxBuf = g_TxBuf6;					/* 发送缓冲区指针 */
    g_tUart6.pRxBuf = g_RxBuf6;					/* 接收缓冲区指针 */
    g_tUart6.usTxBufSize = UART6_TX_BUF_SIZE;	/* 发送缓冲区大小 */
    g_tUart6.usRxBufSize = UART6_RX_BUF_SIZE;	/* 接收缓冲区大小 */
    g_tUart6.usTxWrite = 0;						/* 发送FIFO写索引 */
    g_tUart6.usTxRead = 0;						/* 发送FIFO读索引 */
    g_tUart6.usRxWrite = 0;						/* 接收FIFO写索引 */
    g_tUart6.usRxRead = 0;						/* 接收FIFO读索引 */
    g_tUart6.usRxCount = 0;						/* 接收到的新数据个数 */
    g_tUart6.usTxCount = 0;						/* 待发送的数据个数 */
    g_tUart6.SendBefor = RS485_2_SendBefor;		/* 发送数据前的回调函数 */
    g_tUart6.SendOver = RS485_2_SendOver;			/* 发送完毕后的回调函数 */
    g_tUart6.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: InitHardUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32-F4开发板
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardUart(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

#if UART1_FIFO_EN == 1		/* 串口1 TX = PA9   RX = PA10 */

    /* 第1步： 配置GPIO */
    /* 打开 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 打开 UART 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* 配置 USART Tx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;   //PA9
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 配置 USART Rx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);//初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(USART1, ENABLE);		/* 使能串口 */
    USART_ClearFlag(USART1, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART2_FIFO_EN == 1		/* 串口2 TX = PD5   RX = PD6 或  TX = PA2， RX = PA3  */
    /* 第1步： 配置GPIO */
    /* 打开 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* 打开 UART 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    /* 配置 USART Tx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   //PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 配置 USART Rx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);//初始化串口
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(USART2, ENABLE);		/* 使能串口 */
    USART_ClearFlag(USART2, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART3_FIFO_EN == 1			/* 串口3 TX = PB10   RX = PB11 */

    /* 打开 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 打开 UART 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_DeInit(USART3);  //复位串口3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                     //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                               //复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //初始化PB10

    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                          //浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //初始化PB11

    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(USART3, ENABLE);		/* 使能串口 */
    USART_ClearFlag(USART3, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART4_FIFO_EN == 1			/* 串口4 TX = PC10   RX = PC11 */
    /* 第1步： 配置GPIO */
    /* 打开 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* 打开 UART 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
    /* 配置 USART Tx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;   //PC10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* 配置 USART Rx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART4_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART4, &USART_InitStructure);//初始化串口
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(UART4, ENABLE);		/* 使能串口 */
    USART_ClearFlag(UART4, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART5_FIFO_EN == 1			/* 串口5 TX = PC12   RX = PD2 */
    /* 第1步： 配置GPIO */
    /* 打开 GPIO 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    /* 打开 UART 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);
    /* 配置 USART Tx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;   //PC12
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //复用推挽
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* 配置 USART Rx 为复用功能 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART5_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART5, &USART_InitStructure);//初始化串口
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(UART5, ENABLE);		/* 使能串口 */
    USART_ClearFlag(UART5, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif

#if UART6_FIFO_EN == 1			/* PG14/USART6_TX , PC7/USART6_RX,PG8/USART6_RTS, PG15/USART6_CTS */


    /* 第1步： 配置GPIO */

    /* 打开 GPIO 时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);

    /* 打开 UART 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    /* 将 PC6 映射为 USART6_TX */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

    /* 将 PC7 映射为 USART6_RX */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

    /* 配置 PG14/USART6_TX 为复用功能 */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* 输出类型为推挽 */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 内部上拉电阻使能 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* 复用模式 */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 配置 PC7/USART6_RX 为复用功能 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 第2步： 配置串口硬件参数 */
    USART_InitStructure.USART_BaudRate = UART6_BAUD;	/* 波特率 */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    //USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;	/* 选择硬件流控 */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	/* 不要硬件流控 */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART6, &USART_InitStructure);

    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);	/* 使能接收中断 */
    USART_Cmd(USART6, ENABLE);		/* 使能串口 */
    USART_ClearFlag(USART6, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */
#endif
}

/*
*********************************************************************************************************
*	函 数 名: ConfigUartNVIC
*	功能说明: 配置串口硬件中断.
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    /*	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  --- 在 bsp.c 中 bsp_Init() 中配置中断优先级组 */

#if UART1_FIFO_EN == 1
    /* 使能串口1中断 */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
    /* 使能串口2中断 */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //先占优先级2级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //从优先级2级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
    /* 使能串口3中断t */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART4_FIFO_EN == 1
    /* 使能串口4中断t */
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART5_FIFO_EN == 1
    /* 使能串口5中断t */
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART6_FIFO_EN == 1
    /* 使能串口6中断t */
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen)
{
    uint16_t i;

#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;
#endif

    for (i = 0; i < _usLen; i++)
    {
        /* 如果发送缓冲区已经满了，则等待缓冲区空 */
#if 0
        /*
        	在调试GPRS例程时，下面的代码出现死机，while 死循环
        	原因： 发送第1个字节时 _pUart->usTxWrite = 1；_pUart->usTxRead = 0;
        	将导致while(1) 无法退出
        */
        while (1)
        {
            uint16_t usRead;

            DISABLE_INT();
            usRead = _pUart->usTxRead;
            ENABLE_INT();

            if (++usRead >= _pUart->usTxBufSize)
            {
                usRead = 0;
            }

            if (usRead != _pUart->usTxWrite)
            {
                break;
            }
        }
#else
        /* 当 _pUart->usTxBufSize == 1 时, 下面的函数会死掉(待完善) */
        while (1)
        {
            __IO uint16_t usCount;

            DISABLE_INT();
            usCount = _pUart->usTxCount;
            ENABLE_INT();

            if (usCount < _pUart->usTxBufSize)
            {
                break;
            }
        }
#endif

        /* 将新数据填入发送缓冲区 */
        _pUart->pTxBuf[_pUart->usTxWrite] = _ucaBuf[i];

        DISABLE_INT();
        if (++_pUart->usTxWrite >= _pUart->usTxBufSize)
        {
            _pUart->usTxWrite = 0;
        }
        _pUart->usTxCount++;
        ENABLE_INT();
    }

    USART_ITConfig(_pUart->uart, USART_IT_TXE, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: UartGetChar
*	功能说明: 从串口接收缓冲区读取1字节数据 （用于主程序调用）
*	形    参: _pUart : 串口设备
*			  _pByte : 存放读取数据的指针
*	返 回 值: 0 表示无数据  1表示读取到数据
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
    uint16_t usCount;

#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;
#endif

    /* usRxWrite 变量在中断函数中被改写，主程序读取该变量时，必须进行临界区保护 */
    DISABLE_INT();
    usCount = _pUart->usRxCount;
    ENABLE_INT();

    /* 如果读和写索引相同，则返回0 */
    //if (_pUart->usRxRead == usRxWrite)
    if (usCount == 0)	/* 已经没有数据 */
    {
        return 0;
    }
    else
    {
        *_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* 从串口接收FIFO取1个数据 */

        /* 改写FIFO读索引 */
        DISABLE_INT();
        if (++_pUart->usRxRead >= _pUart->usRxBufSize)
        {
            _pUart->usRxRead = 0;
        }
        _pUart->usRxCount--;
        ENABLE_INT();
        return 1;
    }
}

/*
*********************************************************************************************************
*	函 数 名: UartIRQ
*	功能说明: 供中断服务程序调用，通用串口中断处理函数
*	形    参: _pUart : 串口设备
*	返 回 值: 无
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{

    /* 处理接收中断  */

    if (USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
    {
        /* 从串口接收数据寄存器读取数据存放到接收FIFO */
        uint8_t ch;
        USART_ClearITPendingBit(_pUart->uart,USART_IT_RXNE);
        ch = USART_ReceiveData(_pUart->uart);
        _pUart->pRxBuf[_pUart->usRxWrite] = ch;
        if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
        {
            _pUart->usRxWrite = 0;
        }
        if (_pUart->usRxCount < _pUart->usRxBufSize)
        {
            _pUart->usRxCount++;
        }

        if(_pUart->uart == UART5) queue_push(ch);
        /* 回调函数,通知应用程序收到新数据,一般是发送1个消息或者设置一个标记 */
        //if (_pUart->usRxWrite == _pUart->usRxRead)
        //if (_pUart->usRxCount == 1)
        {
            if (_pUart->ReciveNew)
            {
                _pUart->ReciveNew(ch);
            }
        }
    }
    if(USART_GetFlagStatus(_pUart->uart,USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(_pUart->uart,USART_FLAG_ORE);
        USART_ReceiveData(_pUart->uart);
    }

    /* 处理发送缓冲区空中断 */
    if (USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
    {
        //if (_pUart->usTxRead == _pUart->usTxWrite)
        if (_pUart->usTxCount == 0)
        {
            /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
            USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

            /* 使能数据发送完毕中断 */
            USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
        }
        else
        {
            /* 从发送FIFO取1个字节写入串口发送数据寄存器 */
            USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
            if (++_pUart->usTxRead >= _pUart->usTxBufSize)
            {
                _pUart->usTxRead = 0;
            }
            _pUart->usTxCount--;
        }

    }
    /* 数据bit位全部发送完毕的中断 */
    else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
    {
        //if (_pUart->usTxRead == _pUart->usTxWrite)
        if (_pUart->usTxCount == 0)
        {
            /* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
            USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
//			_pUart->usTxWrite = 0;
            /* 回调函数, 一般用来处理RS485通信，将RS485芯片设置为接收模式，避免抢占总线 */
            if (_pUart->SendOver)
            {
//				_pUart->usTxWrite = 0;
                _pUart->SendOver();
            }
        }
        else
        {
            /* 正常情况下，不会进入此分支 */

            /* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
            USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
            if (++_pUart->usTxRead >= _pUart->usTxBufSize)
            {
                _pUart->usTxRead = 0;
            }
            _pUart->usTxCount--;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler
*	功能说明: USART中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;
    OS_ENTER_CRITICAL();
    OSIntEnter();
    OS_EXIT_CRITICAL();
#endif

    UartIRQ(&g_tUart1);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;

    OS_ENTER_CRITICAL();
    OSIntEnter();
    OS_EXIT_CRITICAL();
#endif

    UartIRQ(&g_tUart2);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

#if UART3_FIFO_EN == 1
void USART3_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;

    OS_ENTER_CRITICAL();
    OSIntEnter();
    OS_EXIT_CRITICAL();
#endif

    UartIRQ(&g_tUart3);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

#if UART4_FIFO_EN == 1
void UART4_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;

    OS_ENTER_CRITICAL();
    OSIntEnter();
    OS_EXIT_CRITICAL();
#endif
    UartIRQ(&g_tUart4);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

#if UART5_FIFO_EN == 1
void UART5_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;
    OS_ENTER_CRITICAL();
    OSIntEnter();
    OS_EXIT_CRITICAL();
#endif
    UartIRQ(&g_tUart5);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

#if UART6_FIFO_EN == 1
void USART6_IRQHandler(void)
{
#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;

    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();
#endif

    UartIRQ(&g_tUart6);

#if uCOS_EN == 1
    OSIntExit();
#endif
}
#endif

uint8_t COM1GetBuf(u8* Buf,u16 size)
{
    uint8_t WriteP =0;
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;

    UARTTemp = ComToUart(COM1);
    if(UARTTemp == 0) return 0;
    totalCnt = UARTTemp->usRxCount;
	if(size < totalCnt)
	{
        for(u16Cnt = 0; u16Cnt < size; u16Cnt++)
        {
            comGetChar(COM1,&Buf[WriteP++]);
        }
	}
	else
	{
		if(totalCnt > 0)
		{
			delay_ms(15);
			totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
			for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
			{
				comGetChar(COM1,&Buf[WriteP++]);
			}
		}
    }
	return u16Cnt;
}

uint8_t COM2GetBuf(u8* Buf,u8 size)
{
    uint8_t WriteP =0;
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;

    UARTTemp = ComToUart(COM2);
    if(UARTTemp == 0)return 0;
    totalCnt = UARTTemp->usRxCount;
	if(size < totalCnt)
	{
        for(u16Cnt = 0; u16Cnt < size; u16Cnt++)
        {
            comGetChar(COM2,&Buf[WriteP++]);
        }
	}
	else
	{
		if(totalCnt > 0)
		{
			delay_ms(15);
			totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
			for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
			{
				comGetChar(COM2,&Buf[WriteP++]);
			}
		}
	}
    return u16Cnt;
}

uint8_t COM3GetBuf(u8* Buf,u8 size)
{
    uint8_t WriteP =0;
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;

    UARTTemp = ComToUart(COM3);
    if(UARTTemp == 0)return 0;
    totalCnt = UARTTemp->usRxCount;
	if(size < totalCnt)
	{
        for(u16Cnt = 0; u16Cnt < size; u16Cnt++)
        {
            comGetChar(COM3,&Buf[WriteP++]);
        }
	}
    else
    {
        if(totalCnt > 0)
        {
            delay_ms(15);
            totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
            for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
            {
                comGetChar(COM3,&Buf[WriteP++]);
            }
        }
    }
    return u16Cnt;
}

uint8_t COM4GetBuf(u8* Buf,u8 size)
{
    uint8_t WriteP =0;
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;

    UARTTemp = ComToUart(COM4);
    if(UARTTemp == 0)return 0;
    totalCnt = UARTTemp->usRxCount;
	if(size < totalCnt)
	{
        for(u16Cnt = 0; u16Cnt < size; u16Cnt++)
        {
            comGetChar(COM4,&Buf[WriteP++]);
        }
	}
	else
	{
		if(totalCnt > 0)
		{
			delay_ms(30);
			totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
			for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
			{
				comGetChar(COM4,&Buf[WriteP++]);
			}
		}
	}
    return u16Cnt;
}

uint8_t COM5GetBuf(u8* Buf,u8 size)
{
    uint8_t WriteP =0;
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;

    UARTTemp = ComToUart(COM5);
    if(UARTTemp == 0)return 0;
    totalCnt = UARTTemp->usRxCount;
    if(size < totalCnt)
    {
        for(u16Cnt = 0; u16Cnt < size; u16Cnt++)
        {
            comGetChar(COM5,&Buf[WriteP++]);
        }
    }
    else
    {
        if(totalCnt > 0)
        {
            delay_ms(15);
            totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
            for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
            {
                comGetChar(COM5,&Buf[WriteP++]);
            }
        }
    }
    return u16Cnt;
}

//功能:将串口新收到的数据拷贝到指定位置
//输入：端口号，目的地址,buf大小
//输出：0：无新数据 其他：有新数据，且返回数据字节数
uint8_t COMGetBuf(COM_PORT_E _ucPort , Ring* Circle,uint16_t len)
{
    uint16_t u16Cnt = 0;
    uint16_t totalCnt = 0;
    UART_T *UARTTemp;
    UARTTemp = ComToUart(_ucPort);
    if(UARTTemp == 0)
    {
        return u16Cnt;
    }
    totalCnt = UARTTemp->usRxCount;
    if(totalCnt > 0)
    {
        delay_ms(50);
        totalCnt = UARTTemp->usRxCount;//等待数据全部接收完
        for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
        {
            comGetChar(_ucPort,&Circle->buf[(Circle->WritePiont++)%len]);
        }
    }
    return u16Cnt;

}
//串口1,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
//u8  USART3_TX_BUF[256]; 			//发送缓冲,最大USART3_MAX_SEND_LEN字节

void u3_printf(char* fmt,...)
{
    u16 i,j;
    va_list ap;
    va_start(ap,fmt);
    vsprintf((char*)g_TxBuf1,fmt,ap);
    va_end(ap);
    i=strlen((const char*)g_TxBuf1);		//此次发送数据的长度
    for(j=0; j<i; j++)							//循环发送数据
    {
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕
        USART_SendData(USART1,g_TxBuf1[j]);
    }
}
//串口3，发送函数
void usart3send(u8* buf, u16 uLen)
{
    u16 u16Cnt = 0;
    if(buf == NULL || uLen == 0)
    {
        return;
    }
    for(u16Cnt = 0; u16Cnt < uLen ; u16Cnt++)
    {
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕
        USART_SendData(USART1,buf[u16Cnt]);
    }
}
/*
*********************************************************************************************************
*	函 数 名: fgetc
*	功能说明: 重定义getc函数，这样可以使用getchar函数从串口1输入数据
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* 从串口接收FIFO中取1个数据, 只有取到数据才返回 */
    uint8_t ucData;

    while(comGetChar(COM1, &ucData) == 0);

    return ucData;
#else
    /* 等待串口1输入数据 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

    return (int)USART_ReceiveData(USART1);
#endif
}
