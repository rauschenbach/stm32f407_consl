#ifndef _SPI_H
#define _SPI_H

#include "main.h"


/**
 * Дефиниции для ног на DISP
 */
/**
  * @brief  DISP SPI Interface pins
  */

/**** Пин RST на PB0. Перекинули с PA2 */
#define DISP_SPI_RESET_PIN             	   GPIO_Pin_0	/* PB0 */
#define DISP_SPI_RESET_GPIO_PORT           GPIOB	/* GPIOB */
#define DISP_SPI_RESET_GPIO_CLK            RCC_AHB1Periph_GPIOB


/**** Пин CD - данные или команда на PB1. Перекинули с PA3 */
#define DISP_SPI_CD_PIN                    GPIO_Pin_1	/* PB1 */
#define DISP_SPI_CD_GPIO_PORT              GPIOB	/* GPIOB */
#define DISP_SPI_CD_GPIO_CLK               RCC_AHB1Periph_GPIOB


#define DISP_SPI                           SPI2
#define DISP_SPI_CLK                       RCC_APB1Periph_SPI2
#define DISP_GPIO_AF	                   GPIO_AF_SPI2


/**** Пин SCK на PB10 */
#define DISP_SPI_SCK_PIN                   GPIO_Pin_10   /*PB10	 */
#define DISP_SPI_SCK_GPIO_PORT             GPIOB	/* GPIOB */
#define DISP_SPI_SCK_GPIO_PIN_SOURCE       GPIO_PinSource10	/* 10 */
#define DISP_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB

/* Не нужен  PB14 */
#define DISP_SPI_MISO_PIN                  GPIO_Pin_14	/* PB14 */
#define DISP_SPI_MISO_GPIO_PORT            GPIOB	/* GPIOB */
#define DISP_SPI_MISO_GPIO_PIN_SOURCE      GPIO_PinSource14	
#define DISP_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB


/**** Пин SDA/MOSI на PB15 */
#define DISP_SPI_MOSI_PIN                  GPIO_Pin_15	/* PB15 */
#define DISP_SPI_MOSI_GPIO_PORT            GPIOB	/* GPIOB */
#define DISP_SPI_MOSI_GPIO_PIN_SOURCE      GPIO_PinSource15	/* 15 */
#define DISP_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB



#define DISP_RESET_LOW()    do { GPIO_ResetBits(DISP_SPI_RESET_GPIO_PORT, DISP_SPI_RESET_PIN); } while(0)
#define DISP_RESET_HI()     do { GPIO_SetBits(DISP_SPI_RESET_GPIO_PORT, DISP_SPI_RESET_PIN); } while(0)

void spi_init(void);
u8   spi_out(u8 data);
void spi_cmd_mode(void);
void spi_data_mode(void);


#endif /* spi.h  */
