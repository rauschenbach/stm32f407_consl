#include <stdio.h>
#include <string.h>
#include "usart3.h"
#include "circbuf.h"
#include "crc16.h"
#include "main.h"
#include "led.h"
#include "com.h"


#define         TEST_USART        USART3
#define         TEST_IRQ_NUM      USART3_IRQn
#define         TEST_TX_PIN       GPIO_Pin_10 /* красный */
#define         TEST_RX_PIN       GPIO_Pin_11 /* желтый */


/**
  * @brief  Initialize USART interface for usart3ment sensor
  */
void usart3_init(int baud)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    /* Тактирование GPIO */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* Тактирование USART3  */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	/* for USART1 */

    /* Configure USART3 Tx as alternate function push-pull */
    gpio.GPIO_Pin = TEST_TX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &gpio);

    /* Configure USART3 Rx as alternate function push-pull */
    gpio.GPIO_Pin = TEST_RX_PIN;
    GPIO_Init(GPIOC, &gpio);

    /* Tx на USART3 */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);

    /* Rx на USART3  нога */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

    /* Сам усарт - скорость неправильная, проверить что приходит в делитель!  */
    usart.USART_BaudRate = baud;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(TEST_USART, &usart);
    
    /* Enable the TEST_USART Interrupt */
    nvic.NVIC_IRQChannel = TEST_IRQ_NUM;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    NVIC_EnableIRQ(TEST_IRQ_NUM);        

    USART_ITConfig(TEST_USART, USART_IT_RXNE, ENABLE);	/* Включаем прерывание */   

    /* Enable USART */
    USART_Cmd(TEST_USART, ENABLE);
}

void Send_symbol(uint8_t data) 
{
  while(!(TEST_USART->SR & USART_SR_TC));
  TEST_USART->DR = data; 
}

void usart3_start(void)
{
    USART_ITConfig(TEST_USART, USART_IT_TXE, ENABLE);	/* Включаем прерывание */   
}

void usart3_stop(void)
{
    USART_ITConfig(TEST_USART, USART_IT_TXE, DISABLE);	/* Включаем прерывание */   
}
