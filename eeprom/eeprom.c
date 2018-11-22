#include <string.h>
#include "eeprom.h"
#include "sensor.h"
#include "eeprom_emul.h"



/**
 * Проверка адреса в EEPROM
 */
static int eeprom_check_addr(uint16_t addr)
{
    if ((addr != SENS0_ADDR) && (addr != SENS1_ADDR) &&
	(addr != SENS2_ADDR) && (addr != SENS3_ADDR) && 
	(addr != SENS4_ADDR) && (addr != SENS5_ADDR) && 
	(addr != SENS6_ADDR) && (addr != SENS7_ADDR)) {
	return -1;
    } else {
	return 0;
    }
}

/**
 * Unlock the Flash Program Erase controller 
 */
void eeprom_init(void)
{
    FLASH_Unlock();   
    EE_Init();
}


/**
 * Записать пакет в EEPROM
 * Адреса идут через 0x20 - минимум
 */
int eeprom_write_pack(EEPROM_VALUE_PACK * pack, uint16_t addr)
{
    int res;
    unsigned short buf;
    short i;

    if (eeprom_check_addr(addr)) {
	return -1;
    }

    for (i = 0; i < sizeof(EEPROM_VALUE_PACK) / 2; i++) {
	memcpy(&buf, (short *) pack + i, 2);
	res = EE_WriteVariable(addr, buf);

	/* Ошибка */
	if (res != FLASH_COMPLETE) {
	    break;
	}

	addr++;
    }
    if(res == FLASH_COMPLETE){
	res = 0;
     }

      return res;
}

/**
 * Прочитать данные с EEPROM
 * 0 - в случае успеха
 */
int eeprom_read_pack(EEPROM_VALUE_PACK * pack, uint16_t addr)
{
    int res;
    short i;
    unsigned short buf;

    if (eeprom_check_addr(addr)) {
	return -1;
    }

    for (i = 0; i < sizeof(EEPROM_VALUE_PACK) / 2; i++) {
	res = EE_ReadVariable(addr, &buf);

	/* Ошибка */
	if (res != 0) {
	    break;
	}

	memcpy((short *) pack + i, &buf, 2);
	addr++;
    }
    return res;
}
