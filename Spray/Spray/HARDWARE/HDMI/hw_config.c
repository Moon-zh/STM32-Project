/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "hw_config.h"


ErrorStatus HSEStartUpStatus;
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USART_InitTypeDef USART_InitStructure;



/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Set_System
* Description    : Configures Main system clocks & power
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_System(void)
{


    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();
    /*使用内部的晶振*/
    RCC_HSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)
    {
    }
    if(1)
    {

        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        FLASH_SetLatency(FLASH_Latency_2);

        RCC_HCLKConfig(RCC_SYSCLK_Div1);    //AHB时钟

        RCC_PCLK2Config(RCC_HCLK_Div1);	 //APB2时钟

        RCC_PCLK1Config(RCC_HCLK_Div2);	//ABP1时钟

        //设置 PLL 时钟源及倍频系数
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_12);	 //4M*12 =48M


        //使能或者失能 PLL,这个参数可以取：ENABLE或者DISABLE
        RCC_PLLCmd(ENABLE);//如果PLL被用于系统时钟,那么它不能被失能
        //等待指定的 RCC 标志位设置成功 等待PLL初始化成功
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        //设置系统时钟（SYSCLK） 设置PLL为系统时钟源
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        //等待PLL成功用作于系统时钟的时钟源
        //  0x00：HSI 作为系统时钟
        //  0x04：HSE作为系统时钟
        //  0x08：PLL作为系统时钟
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
#
}



/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
void Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    /* Enable USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}




/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
