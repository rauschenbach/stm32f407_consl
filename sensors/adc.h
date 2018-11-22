#ifndef _ADC_H
#define _ADC_H

#include "main.h"

void adc_init(void);
void adc_start(void);
void adc_stop(void);
u16  adc_get_data(void);
u16  adc_read_chan(u8);

#endif /* adc.h */
