#include "sys.h"
#include "Includes.h"
#include "rs485.h"
#include "delay.h"
#include "cmd_queue.h"

#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
    int handle;

};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
    x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
    switch(printf_num)
    {
    case 1:
        while((USART1->SR&0X40)==0);//ѭ������,ֱ���������
        USART1->DR = (u8) ch;
        return ch;
    case 2:
        while((USART2->SR&0X40)==0);//ѭ������,ֱ���������
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

//#ifdef EN_USART2_RX   	//���ʹ���˽���

static void UartIRQ(UART_T *_pUart);
/* ʹ����Դ�ļ���ʹ��uCOS-III�ĺ���, �����Դ�ļ���Ҫ��ָBSP�����ļ� */
#define uCOS_EN       1

#if uCOS_EN == 1
#include "os_cpu.h"

#define  ENABLE_INT()  	  OS_EXIT_CRITICAL();     /* ʹ��ȫ���ж� */
#define  DISABLE_INT()    OS_ENTER_CRITICAL()    /* ��ֹȫ���ж� */
#else
/* ����ȫ���жϵĺ� */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */
#endif

/* ����ÿ�����ڽṹ����� */
#if UART1_FIFO_EN == 1
UART_T g_tUart1;
uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART2_FIFO_EN == 1
UART_T g_tUart2;
uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART3_FIFO_EN == 1
UART_T g_tUart3;
uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART4_FIFO_EN == 1
UART_T g_tUart4;
uint8_t g_TxBuf4[UART4_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART5_FIFO_EN == 1
UART_T g_tUart5;
uint8_t g_TxBuf5[UART5_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART6_FIFO_EN == 1
UART_T g_tUart6;
static uint8_t g_TxBuf6[UART6_TX_BUF_SIZE];		/* ���ͻ����� */
uint8_t g_RxBuf6[UART6_RX_BUF_SIZE];		/* ���ջ����� */
#endif

static void UartVarInit(void);
static void InitHardUart(void);
static void UartSend(UART_T *_pUart, uint8_t *_ucaBuf, uint16_t _usLen);
static uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
static void ConfigUartNVIC(void);
void RS485_InitTXE(void);
/*
*********************************************************************************************************
*	ģ������ : �����ж�+FIFO����ģ��
*	�ļ����� : bsp_uart_fifo.c
*	��    �� : V1.0
*	˵    �� : ���ô����ж�+FIFOģʽʵ�ֶ�����ڵ�ͬʱ����
*********************************************************************************************************
*/
#include "Rs485.h"
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitUart
*	����˵��: ��ʼ������Ӳ��������ȫ�ֱ�������ֵ.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
    UartVarInit();		/* �����ȳ�ʼ��ȫ�ֱ���,������Ӳ�� */
    RS485_InitTXE();	/* ����RS485оƬ�ķ���ʹ��Ӳ��������Ϊ������� */
    InitHardUart();		/* ���ô��ڵ�Ӳ������(�����ʵ�) */
    ConfigUartNVIC();	/* ���ô����ж� */
}

/*
*********************************************************************************************************
*	�� �� ��: ComToUart
*	����˵��: ��COM�˿ں�ת��ΪUARTָ��
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*	�� �� ֵ: uartָ��
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
        /* �����κδ��� */
        return 0;
    }
}
/*
*********************************************************************************************************
*	�� �� ��: GetUartRxAvailableDataSize
*	����˵��: ��ȡ���ڽ��ջ�������ǰ��Ч�ֽ��� ��������������ã�
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*
*	�� �� ֵ: 0 ��ʾ������  ��0��ʾ��ǰ��Ч�ֽ���
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
*	�� �� ��: ComToUart
*	����˵��: ��COM�˿ں�ת��Ϊ USART_TypeDef* USARTx
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*	�� �� ֵ: USART_TypeDef*,  USART1, USART2, USART3, UART4, UART5
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
        /* �����κδ��� */
        return 0;
    }
}
/*
*********************************************************************************************************
*	�� �� ��: comSendBuf
*	����˵��: �򴮿ڷ���һ�����ݡ����ݷŵ����ͻ��������������أ����жϷ�������ں�̨��ɷ���
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*			  _ucaBuf: �����͵����ݻ�����
*			  _usLen : ���ݳ���
*	�� �� ֵ: ��
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
        pUart->SendBefor();		/* �����RS485ͨ�ţ���������������н�RS485����Ϊ����ģʽ */
    }

    UartSend(pUart, _ucaBuf, _usLen);
}
/*
*********************************************************************************************************
*	�� �� ��: comSendChar
*	����˵��: �򴮿ڷ���1���ֽڡ����ݷŵ����ͻ��������������أ����жϷ�������ں�̨��ɷ���
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*			  _ucByte: �����͵�����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte)
{
    comSendBuf(_ucPort, &_ucByte, 1);
}
/*
*********************************************************************************************************
*	�� �� ��: comGetChar
*	����˵��: �ӽ��ջ�������ȡ1�ֽڣ��������������������ݾ��������ء�
*	��    ��: _ucPort: �˿ں�(COM1 - COM5)
*			  _pByte: ���յ������ݴ���������ַ
*	�� �� ֵ: 0 ��ʾ������, 1 ��ʾ��ȡ����Ч�ֽ�
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
*	�� �� ��: comClearTxFifo
*	����˵��: ���㴮�ڷ��ͻ�����
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*	�� �� ֵ: ��
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
*	�� �� ��: comClearRxFifo
*	����˵��: ���㴮�ڽ��ջ�����
*	��    ��: _ucPort: �˿ں�(COM1 - COM6)
*	�� �� ֵ: ��
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
*	�� �� ��: comSetBaud
*	����˵��: ���ô��ڵĲ�����
*	��    ��: _ucPort: �˿ں�(COM1 - COM5)
*			  _BaudRate: �����ʣ�0-4500000�� ���4.5Mbps
*	�� �� ֵ: ��
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
*	�� �� ��: USART_SetBaudRate
*	����˵��: �޸Ĳ����ʼĴ������������������á����ʹ�� USART_Init����, ����޸�Ӳ�����ز�����RX,TX����
*			  ���ݹ̼����� USART_Init���������������ò����ʵĲ��ֵ����������װΪһ������
*	��    ��: USARTx : USART1, USART2, USART3, UART4, UART5
*			  BaudRate : �����ʣ�ȡֵ 0 - 4500000
*	�� �� ֵ: ��
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
*	�� �� ��: RS485_InitTXE
*	����˵��: ����RS485����ʹ�ܿ��� TXE
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_14;	 //PB5 PB14�˿�����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 //�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //PC5�˿�����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 //�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
    }
}
/*
*********************************************************************************************************
*	�� �� ��: RS485_SendBefor
*	����˵��: ��������ǰ��׼������������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendBefor = RS485_SendBefor
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_0_SendBefor(void)
{
    RS485_TX_EN0=1;
    //RS485_TX_1_EN();	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendOver
*	����˵��: ����һ�����ݽ�������ƺ�������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendOver = RS485_SendOver
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_0_SendOver(void)
{
    RS485_TX_EN0=0;	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*********************************************************************************************************
*	�� �� ��: RS485_SendBefor
*	����˵��: ��������ǰ��׼������������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendBefor = RS485_SendBefor
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_1_SendBefor(void)
{
    RS485_TX_EN1=1;
    //RS485_TX_1_EN();	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendOver
*	����˵��: ����һ�����ݽ�������ƺ�������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendOver = RS485_SendOver
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_1_SendOver(void)
{
    RS485_TX_EN1=0;	/* �л�RS485�շ�оƬΪ����ģʽ */
}


/*
*********************************************************************************************************
*	�� �� ��: RS485_SendBefor
*	����˵��: ��������ǰ��׼������������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendBefor = RS485_SendBefor
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_2_SendBefor(void)
{
    RS485_TX_EN2=1; /* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: RS485_SendOver
*	����˵��: ����һ�����ݽ�������ƺ�������RS485ͨ�ţ�������RS485оƬΪ����״̬��
*			  ���޸� UartVarInit()�еĺ���ָ����ڱ������������� g_tUart2.SendOver = RS485_SendOver
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void RS485_2_SendOver(void)
{
    RS485_TX_EN2=0;	/* �л�RS485�շ�оƬΪ����ģʽ */
}

/*
*********************************************************************************************************
*	�� �� ��: UartVarInit
*	����˵��: ��ʼ��������صı���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
    g_tUart1.uart = USART1;						/* STM32 �����豸 */
    g_tUart1.pTxBuf = g_TxBuf1;					/* ���ͻ�����ָ�� */
    g_tUart1.pRxBuf = g_RxBuf1;					/* ���ջ�����ָ�� */
    g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart1.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart1.usTxRead = 0;						/* ����FIFO������ */
    g_tUart1.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart1.usRxRead = 0;						/* ����FIFO������ */
    g_tUart1.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart1.usTxCount = 0;						/* �����͵����ݸ��� */
    g_tUart1.SendBefor = 0;						/* ��������ǰ�Ļص����� */
    g_tUart1.SendOver = 0;						/* ������Ϻ�Ļص����� */
    g_tUart1.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif

#if UART2_FIFO_EN == 1
    g_tUart2.uart = USART2;						/* STM32 �����豸 */
    g_tUart2.pTxBuf = g_TxBuf2;					/* ���ͻ�����ָ�� */
    g_tUart2.pRxBuf = g_RxBuf2;					/* ���ջ�����ָ�� */
    g_tUart2.usTxBufSize = UART2_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart2.usRxBufSize = UART2_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart2.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart2.usTxRead = 0;						/* ����FIFO������ */
    g_tUart2.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart2.usRxRead = 0;						/* ����FIFO������ */
    g_tUart2.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart2.usTxCount = 0;						/* �����͵����ݸ��� */
    g_tUart2.SendBefor = 0;						/* ��������ǰ�Ļص����� */
    g_tUart2.SendOver = 0;						/* ������Ϻ�Ļص����� */
    g_tUart2.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif

#if UART3_FIFO_EN == 1
    g_tUart3.uart = USART3;						/* STM32 �����豸 */
    g_tUart3.pTxBuf = g_TxBuf3;					/* ���ͻ�����ָ�� */
    g_tUart3.pRxBuf = g_RxBuf3;					/* ���ջ�����ָ�� */
    g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart3.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart3.usTxRead = 0;						/* ����FIFO������ */
    g_tUart3.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart3.usRxRead = 0;						/* ����FIFO������ */
    g_tUart3.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart3.usTxCount = 0;						/* �����͵����ݸ��� */
    g_tUart3.SendBefor = RS485_0_SendBefor;		/* ��������ǰ�Ļص����� */
    g_tUart3.SendOver = RS485_0_SendOver;		/* ������Ϻ�Ļص����� */
    g_tUart3.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif

#if UART4_FIFO_EN == 1
    g_tUart4.uart = UART4;						/* STM32 �����豸 */
    g_tUart4.pTxBuf = g_TxBuf4;					/* ���ͻ�����ָ�� */
    g_tUart4.pRxBuf = g_RxBuf4;					/* ���ջ�����ָ�� */
    g_tUart4.usTxBufSize = UART4_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart4.usRxBufSize = UART4_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart4.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart4.usTxRead = 0;						/* ����FIFO������ */
    g_tUart4.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart4.usRxRead = 0;						/* ����FIFO������ */
    g_tUart4.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart4.usTxCount = 0;						/* �����͵����ݸ��� */
    g_tUart4.SendBefor = RS485_1_SendBefor;		/* ��������ǰ�Ļص����� */
    g_tUart4.SendOver = RS485_1_SendOver;		/* ������Ϻ�Ļص����� */
    //g_tUart4.SendBefor = 0;						/* ��������ǰ�Ļص����� */
    //g_tUart4.SendOver = 0;						/* ������Ϻ�Ļص����� */
    g_tUart4.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif

#if UART5_FIFO_EN == 1
    g_tUart5.uart = UART5;						/* STM32 �����豸 */
    g_tUart5.pTxBuf = g_TxBuf5;					/* ���ͻ�����ָ�� */
    g_tUart5.pRxBuf = g_RxBuf5;					/* ���ջ�����ָ�� */
    g_tUart5.usTxBufSize = UART5_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart5.usRxBufSize = UART5_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart5.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart5.usTxRead = 0;						/* ����FIFO������ */
    g_tUart5.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart5.usRxRead = 0;						/* ����FIFO������ */
    g_tUart5.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart5.usTxCount = 0;						/* �����͵����ݸ��� */
    //g_tUart5.SendBefor = RS485_2_SendBefor;		/* ��������ǰ�Ļص����� */
    //g_tUart5.SendOver  = RS485_2_SendOver;			/* ������Ϻ�Ļص����� */
    g_tUart5.SendBefor = 0;						/* ��������ǰ�Ļص����� */
    g_tUart5.SendOver  = 0;						/* ������Ϻ�Ļص����� */
    g_tUart5.ReciveNew =0;						/* ���յ������ݺ�Ļص����� */
#endif


#if UART6_FIFO_EN == 1
    g_tUart6.uart = USART6;						/* STM32 �����豸 */
    g_tUart6.pTxBuf = g_TxBuf6;					/* ���ͻ�����ָ�� */
    g_tUart6.pRxBuf = g_RxBuf6;					/* ���ջ�����ָ�� */
    g_tUart6.usTxBufSize = UART6_TX_BUF_SIZE;	/* ���ͻ�������С */
    g_tUart6.usRxBufSize = UART6_RX_BUF_SIZE;	/* ���ջ�������С */
    g_tUart6.usTxWrite = 0;						/* ����FIFOд���� */
    g_tUart6.usTxRead = 0;						/* ����FIFO������ */
    g_tUart6.usRxWrite = 0;						/* ����FIFOд���� */
    g_tUart6.usRxRead = 0;						/* ����FIFO������ */
    g_tUart6.usRxCount = 0;						/* ���յ��������ݸ��� */
    g_tUart6.usTxCount = 0;						/* �����͵����ݸ��� */
    g_tUart6.SendBefor = RS485_2_SendBefor;		/* ��������ǰ�Ļص����� */
    g_tUart6.SendOver = RS485_2_SendOver;			/* ������Ϻ�Ļص����� */
    g_tUart6.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: InitHardUart
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32-F4������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitHardUart(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

#if UART1_FIFO_EN == 1		/* ����1 TX = PA9   RX = PA10 */

    /* ��1���� ����GPIO */
    /* �� GPIO ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* �� UART ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    /* ���� USART Tx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;   //PA9
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* ���� USART Rx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);//��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(USART1, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(USART1, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART2_FIFO_EN == 1		/* ����2 TX = PD5   RX = PD6 ��  TX = PA2�� RX = PA3  */
    /* ��1���� ����GPIO */
    /* �� GPIO ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* �� UART ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    /* ���� USART Tx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;   //PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* ���� USART Rx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);//��ʼ������
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(USART2, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(USART2, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART3_FIFO_EN == 1			/* ����3 TX = PB10   RX = PB11 */

    /* �� GPIO ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* �� UART ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    USART_DeInit(USART3);  //��λ����3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                     //PB10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                               //�����������
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //��ʼ��PB10

    //USART3_RX	  PB11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                          //��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //��ʼ��PB11

    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(USART3, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(USART3, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART4_FIFO_EN == 1			/* ����4 TX = PC10   RX = PC11 */
    /* ��1���� ����GPIO */
    /* �� GPIO ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* �� UART ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
    /* ���� USART Tx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;   //PC10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* ���� USART Rx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC11
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART4_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART4, &USART_InitStructure);//��ʼ������
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(UART4, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(UART4, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART5_FIFO_EN == 1			/* ����5 TX = PC12   RX = PD2 */
    /* ��1���� ����GPIO */
    /* �� GPIO ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    /* �� UART ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);
    /* ���� USART Tx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;   //PC12
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    /* ���� USART Rx Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART5_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(UART5, &USART_InitStructure);//��ʼ������
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(UART5, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(UART5, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif

#if UART6_FIFO_EN == 1			/* PG14/USART6_TX , PC7/USART6_RX,PG8/USART6_RTS, PG15/USART6_CTS */


    /* ��1���� ����GPIO */

    /* �� GPIO ʱ�� */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);

    /* �� UART ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    /* �� PC6 ӳ��Ϊ USART6_TX */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);

    /* �� PC7 ӳ��Ϊ USART6_RX */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

    /* ���� PG14/USART6_TX Ϊ���ù��� */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ���� PC7/USART6_RX Ϊ���ù��� */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ��2���� ���ô���Ӳ������ */
    USART_InitStructure.USART_BaudRate = UART6_BAUD;	/* ������ */
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    //USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;	/* ѡ��Ӳ������ */
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	/* ��ҪӲ������ */
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART6, &USART_InitStructure);

    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
    USART_Cmd(USART6, ENABLE);		/* ʹ�ܴ��� */
    USART_ClearFlag(USART6, USART_FLAG_TC);     /* �巢����ɱ�־��Transmission Complete flag */
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: ConfigUartNVIC
*	����˵��: ���ô���Ӳ���ж�.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    /*	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  --- �� bsp.c �� bsp_Init() �������ж����ȼ��� */

#if UART1_FIFO_EN == 1
    /* ʹ�ܴ���1�ж� */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
    /* ʹ�ܴ���2�ж� */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //��ռ���ȼ�2��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //�����ȼ�2��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
    /* ʹ�ܴ���3�ж�t */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART4_FIFO_EN == 1
    /* ʹ�ܴ���4�ж�t */
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART5_FIFO_EN == 1
    /* ʹ�ܴ���5�ж�t */
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#if UART6_FIFO_EN == 1
    /* ʹ�ܴ���6�ж�t */
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: UartSend
*	����˵��: ��д���ݵ�UART���ͻ�����,�����������жϡ��жϴ�����������Ϻ��Զ��رշ����ж�
*	��    ��:  ��
*	�� �� ֵ: ��
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
        /* ������ͻ������Ѿ����ˣ���ȴ��������� */
#if 0
        /*
        	�ڵ���GPRS����ʱ������Ĵ������������while ��ѭ��
        	ԭ�� ���͵�1���ֽ�ʱ _pUart->usTxWrite = 1��_pUart->usTxRead = 0;
        	������while(1) �޷��˳�
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
        /* �� _pUart->usTxBufSize == 1 ʱ, ����ĺ���������(������) */
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

        /* �����������뷢�ͻ����� */
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
*	�� �� ��: UartGetChar
*	����˵��: �Ӵ��ڽ��ջ�������ȡ1�ֽ����� ��������������ã�
*	��    ��: _pUart : �����豸
*			  _pByte : ��Ŷ�ȡ���ݵ�ָ��
*	�� �� ֵ: 0 ��ʾ������  1��ʾ��ȡ������
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte)
{
    uint16_t usCount;

#if uCOS_EN == 1
    OS_CPU_SR cpu_sr=0;
#endif

    /* usRxWrite �������жϺ����б���д���������ȡ�ñ���ʱ����������ٽ������� */
    DISABLE_INT();
    usCount = _pUart->usRxCount;
    ENABLE_INT();

    /* �������д������ͬ���򷵻�0 */
    //if (_pUart->usRxRead == usRxWrite)
    if (usCount == 0)	/* �Ѿ�û������ */
    {
        return 0;
    }
    else
    {
        *_pByte = _pUart->pRxBuf[_pUart->usRxRead];		/* �Ӵ��ڽ���FIFOȡ1������ */

        /* ��дFIFO������ */
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
*	�� �� ��: UartIRQ
*	����˵��: ���жϷ��������ã�ͨ�ô����жϴ�����
*	��    ��: _pUart : �����豸
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{

    /* ��������ж�  */

    if (USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
    {
        /* �Ӵ��ڽ������ݼĴ�����ȡ���ݴ�ŵ�����FIFO */
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
        /* �ص�����,֪ͨӦ�ó����յ�������,һ���Ƿ���1����Ϣ��������һ����� */
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

    /* �����ͻ��������ж� */
    if (USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
    {
        //if (_pUart->usTxRead == _pUart->usTxWrite)
        if (_pUart->usTxCount == 0)
        {
            /* ���ͻ�������������ȡ��ʱ�� ��ֹ���ͻ��������ж� ��ע�⣺��ʱ���1�����ݻ�δ����������ϣ�*/
            USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

            /* ʹ�����ݷ�������ж� */
            USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
        }
        else
        {
            /* �ӷ���FIFOȡ1���ֽ�д�봮�ڷ������ݼĴ��� */
            USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
            if (++_pUart->usTxRead >= _pUart->usTxBufSize)
            {
                _pUart->usTxRead = 0;
            }
            _pUart->usTxCount--;
        }

    }
    /* ����bitλȫ��������ϵ��ж� */
    else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
    {
        //if (_pUart->usTxRead == _pUart->usTxWrite)
        if (_pUart->usTxCount == 0)
        {
            /* �������FIFO������ȫ��������ϣ���ֹ���ݷ�������ж� */
            USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);
//			_pUart->usTxWrite = 0;
            /* �ص�����, һ����������RS485ͨ�ţ���RS485оƬ����Ϊ����ģʽ��������ռ���� */
            if (_pUart->SendOver)
            {
//				_pUart->usTxWrite = 0;
                _pUart->SendOver();
            }
        }
        else
        {
            /* ��������£��������˷�֧ */

            /* �������FIFO�����ݻ�δ��ϣ���ӷ���FIFOȡ1������д�뷢�����ݼĴ��� */
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
*	�� �� ��: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler
*	����˵��: USART�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
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
			totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
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
			totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
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
            totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
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
			totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
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
            totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
            for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
            {
                comGetChar(COM5,&Buf[WriteP++]);
            }
        }
    }
    return u16Cnt;
}

//����:���������յ������ݿ�����ָ��λ��
//���룺�˿ںţ�Ŀ�ĵ�ַ,buf��С
//�����0���������� �������������ݣ��ҷ��������ֽ���
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
        totalCnt = UARTTemp->usRxCount;//�ȴ�����ȫ��������
        for(u16Cnt = 0; u16Cnt < totalCnt; u16Cnt++)
        {
            comGetChar(_ucPort,&Circle->buf[(Circle->WritePiont++)%len]);
        }
    }
    return u16Cnt;

}
//����1,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
//u8  USART3_TX_BUF[256]; 			//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�

void u3_printf(char* fmt,...)
{
    u16 i,j;
    va_list ap;
    va_start(ap,fmt);
    vsprintf((char*)g_TxBuf1,fmt,ap);
    va_end(ap);
    i=strlen((const char*)g_TxBuf1);		//�˴η������ݵĳ���
    for(j=0; j<i; j++)							//ѭ����������
    {
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������
        USART_SendData(USART1,g_TxBuf1[j]);
    }
}
//����3�����ͺ���
void usart3send(u8* buf, u16 uLen)
{
    u16 u16Cnt = 0;
    if(buf == NULL || uLen == 0)
    {
        return;
    }
    for(u16Cnt = 0; u16Cnt < uLen ; u16Cnt++)
    {
        while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������
        USART_SendData(USART1,buf[u16Cnt]);
    }
}
/*
*********************************************************************************************************
*	�� �� ��: fgetc
*	����˵��: �ض���getc��������������ʹ��getchar�����Ӵ���1��������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int fgetc(FILE *f)
{

#if 1	/* �Ӵ��ڽ���FIFO��ȡ1������, ֻ��ȡ�����ݲŷ��� */
    uint8_t ucData;

    while(comGetChar(COM1, &ucData) == 0);

    return ucData;
#else
    /* �ȴ�����1�������� */
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

    return (int)USART_ReceiveData(USART1);
#endif
}
