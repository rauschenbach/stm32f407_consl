#ifndef _EEPROM_H
#define _EEPROM_H

#include "sensor.h"
#include "main.h"

#define EEPROM_ADDR_SHIFT 	sizeof(channel_params_settings)

/* адреса дл€ параметров каждого канала */
#define SENS0_ADDR  0x0000
#define SENS1_ADDR  (SENS0_ADDR + EEPROM_ADDR_SHIFT)
#define SENS2_ADDR  (SENS1_ADDR + EEPROM_ADDR_SHIFT)
#define SENS3_ADDR  (SENS2_ADDR + EEPROM_ADDR_SHIFT)
#define SENS4_ADDR  (SENS3_ADDR + EEPROM_ADDR_SHIFT)
#define SENS5_ADDR  (SENS4_ADDR + EEPROM_ADDR_SHIFT)
#define SENS6_ADDR  (SENS5_ADDR + EEPROM_ADDR_SHIFT)
#define SENS7_ADDR  (SENS6_ADDR + EEPROM_ADDR_SHIFT)


#define SENS_ADDR(x)	(x * EEPROM_ADDR_SHIFT)

#define MAX_ADDR (SENS7_ADDR + EEPROM_ADDR_SHIFT)



/**
 * „то будем хранить в псевдо EEPROM (84 байта)
 */
typedef union {
	/* Payload с этого места  */
	channel_params_settings sens;
	u8 data[sizeof(channel_params_settings)];
} EEPROM_VALUE_PACK;


void eeprom_init(void);
int  eeprom_write_pack(EEPROM_VALUE_PACK*, uint16_t);
int  eeprom_read_pack(EEPROM_VALUE_PACK*, uint16_t);

#endif /* eeprom.h */
