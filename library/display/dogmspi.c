/*

  dogmspi.c
  
  spi abstraction layer
  
*/
#include "dogm128.h"
#include "main.h"
#include "spi.h"



/* setup hardware pins, prepare SPI subsystem */
void dog_spi_init(void)
{
	spi_init();
}


/* output a 8 bit value on the SPI bus */
unsigned char dog_spi_out(unsigned char data)
{
	/* Loop while DR register in not empty */
	return spi_out(data);
}

/* assign low to chip select line */
void dog_spi_enable_client(void)
{
    //GPIO_ResetBits(DISP_SPI_SCK_GPIO_PORT, DISP_SPI_SCK_PIN);	/*  RESET hi - не выбрано */
    //Delay(1);
}

/* assign high to chip select line */
void dog_spi_disable_client(void)
{
  //Delay(1);
}

/* ¬низ - команда. assign low to A0 line */
void dog_cmd_mode(void)
{
    spi_cmd_mode();
}

/* ¬верх - данные. assign high to A0 line */
void dog_data_mode(void)
{
    spi_data_mode();
}



