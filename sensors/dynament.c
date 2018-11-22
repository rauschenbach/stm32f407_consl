/******************************************************************************
 *      ������� Dynament
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "dynament.h"
#include "circbuf.h"
#include "crc16.h"
#include "main.h"
#include "led.h"


#define         DYNA_BUF_SIZE          127


/* ����� �� UART1 - �������� ������ �� ��������� �� ���� ��������� */
static struct {
    u8 rx_buf[DYNA_BUF_SIZE];	/* ���� ������� ��� ������� */
    u8 rx_beg;
    u8 rx_cnt;
    u8 rx_len;
    u16 crc16;
    bool rx_ready;
} usart_rx_data;

        /* �������� �� USART  */
static struct {
    u8 tx_buf[DYNA_BUF_SIZE];	/* ���� ������� ��� �������� */
    c8 tx_cnt;
    c8 tx_len;
} usart_tx_data;


static void dyna_write_byte_isr(void);
static void dyna_read_byte_isr(u8 byte);



/**
  * @brief  Initialize USART2 interface for dynament sensor
  * ����� �� ����� SD ��������
  * ����:
  * ����� RX �� PA.02 
  * ������� Tx PA.03
  * ������ ����������� ��������� ��������!!!
  */
void sens_dyna_init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    /* ������������ GPIO */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* ������������ USART2  */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	/* for USART2 */

    /* Configure USART Tx as alternate function push-pull */
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);

    /* Configure USART Rx as alternate function push-pull */
    gpio.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &gpio);


    /* Tx �� USART2 2 ���� */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

    /* Rx �� USART2 3 ���� */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    /* ��� ����� - �������� ������������, ��������� ��� �������� � ��������!  */
    usart.USART_BaudRate = /*115200*/38400;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &usart);
    
    /* Enable the USART2 Interrupt */
    nvic.NVIC_IRQChannel = USART2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
    NVIC_EnableIRQ(USART2_IRQn);        

   // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* �������� ���������� */   

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);
}



/**
 * ����� ���� ������
 */
void dyna_start(void)
{
    memset(&usart_rx_data, 0, sizeof(usart_rx_data));	
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* ����� �� �������� ON */
}

/**
 * ���� ���� ������
 */
void dyna_stop(void)
{
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);	/* ����� �� �������� OFF */
}


/**
 * ������ ����� ������ ������� �������
 * ������� ������� 0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53
 * � ����� �����
 */
int dyna_get_live_data(void *data)
{
    volatile int ticks;
    int res = -1;
    u8 buf[] = { 0x10, 0x13, 0x01, 0x10, 0x1F, 0x00, 0x53 };   // ������� ����� Live Data 

    /* ����� ����� */
    dyna_start();
    
    led_toggle(LED3);

    ticks = timer2_get_ms_ticks();
    
    /* �������� ������� GET LIVE DATA */
    dyna_send_data(buf, sizeof(buf));

    /* ���� ����� �������� 250 �� */
    while(!usart_rx_data.rx_ready) {
      if(timer2_get_ms_ticks() - ticks > 250) {
        break;
      }
    }
    
     if(usart_rx_data.rx_buf[0] == 0x10 && usart_rx_data.rx_buf[1] == 0x1A && usart_rx_data.rx_buf[2] == sizeof(dyna_live_data_pack)) {
        memcpy(data, &usart_rx_data.rx_buf[3], sizeof(dyna_live_data_pack));
	res = 0;
     }  
    return res;
}

/**
 * ������ ��������� ��� ���?
 */
int dyna_check(void)
{
  return 0;
}


/**
 * ������� ����� ������� � ���� UART
 */
void dyna_send_data(u8 * data, u8 len)
{
    memcpy(usart_tx_data.tx_buf, data, len);	/* ������� ��������� ��������� */
    add_check_sum(usart_tx_data.tx_buf, len);	/* ������ ������� ��� ����� � ��������� */
    usart_tx_data.tx_len = len;
    usart_tx_data.tx_cnt = 0;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

/* �������� ���� ����, ����� ���������� ������ - ��������� ���������� */
static void dyna_write_byte_isr(void)
{
    volatile u8 byte;

    /* ��������, ���� ����� �� ������. ����� - ��������� ����������. */
    if (usart_tx_data.tx_len-- > 0) {
	byte = usart_tx_data.tx_buf[usart_tx_data.tx_cnt++];
	USART_SendData(USART2, byte);
    } else {
	usart_tx_data.tx_cnt = 0;
	usart_tx_data.tx_len = 0;
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
    }
}


/**
 * ��������� ����� 
 */
static void dyna_read_byte_isr(u8 byte)
{
    /*  ������ ����. ���������� ������ � DLE */
    if (byte == 0x10 && usart_rx_data.rx_beg == 0) {
	usart_rx_data.rx_buf[0] = byte;
	usart_rx_data.rx_beg = 1;
	usart_rx_data.rx_cnt = 1;
	usart_rx_data.rx_ready = false;

    } else if(byte == 0x10 && usart_rx_data.rx_beg == 1 && usart_rx_data.rx_cnt == 1) {
	usart_rx_data.rx_buf[0] = byte;
	usart_rx_data.rx_beg = 1;
	usart_rx_data.rx_cnt = 1;
    } else if (usart_rx_data.rx_beg == 1) {	/* ����������� ����� */

	/* ������ ����� ������� */
	if (usart_rx_data.rx_cnt == 2) {
	    usart_rx_data.rx_len = byte + 7;
	} else if(usart_rx_data.rx_cnt == usart_rx_data.rx_len - 1) {
		dyna_stop();/* ����� ������ */
                usart_rx_data.rx_beg = 0 ;
		usart_rx_data.rx_ready = true;                
        }
	usart_rx_data.rx_buf[usart_rx_data.rx_cnt++] = byte;
        
    } else {
      	dyna_stop();/* ����� ������ */
        usart_rx_data.rx_beg = 0 ;        
    }
}

/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler(void)
{
    long xHigherPriorityTaskWoken = pdFALSE;

    /* �������� �� ������� ������ - ���� ����� �� ���� */
    if (USART_GetITStatus(USART2, USART_IT_TXE) == SET) {
	dyna_write_byte_isr();
    }

    /* ����� */
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
	u8 rx_byte = USART_ReceiveData(USART2);
	dyna_read_byte_isr(rx_byte);
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
