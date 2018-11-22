#ifndef _GLOBDEFS_H
#define _GLOBDEFS_H


#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <float.h>
#include "stm32f4xx.h"


typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;	/* long double не поддержываеца  */
typedef char  c8;

typedef unsigned char const    ucint8_t;
typedef volatile unsigned char vuint8_t;
typedef volatile unsigned long vuint32_t;

typedef enum {false, true} bool;


#if 0

#ifndef u8
#define u8 uint8_t
#endif


#ifndef c8
#define c8 char
#endif


#ifndef u32
#define u32 uint32_t
#endif


#ifndef s32
#define s32 int32_t
#endif


#ifndef bool
#define bool uint8_t
#endif


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#endif


#ifndef IDEF
#define IDEF static inline
#endif

/* На этот адрес будет отвечать наше устройство */
#define 	BROADCAST_ADDR	0xff

/* Максимальное число всех каналов для настройки  */
#define 	NUM_ALL_CHAN	       		8

/* Число датчиков на LMP91к  */
#define 	NUM_LMP91K_SENS			4


/* Число измерений в одном пакете даных за секунду */
#define 	NUM_ACQUIS_IN_PACK		5

#define		MAGIC				0x4b495245

/* Делители */
#define		TIMER_NS_DIVIDER	(1000000000UL)
#define		TIMER_US_DIVIDER	(1000000)
#define		TIMER_MS_DIVIDER	(1000)

/* Причины сброса прибора - перечислены здесь. Из запишем в eeprom  */
#define		CAUSE_POWER_OFF		0x12345678	/* Выключение питания */
#define		CAUSE_EXT_RESET		0xabcdef90	/* Внешний ресет */
#define		CAUSE_BROWN_OUT		0xaa55aa55	/* Снижение питания */
#define		CAUSE_WDT_RESET		0x07070707	/* WDT reset (во время регистрации) */
#define		CAUSE_NO_LINK		0xE7E7E7E7	/* Нет связи - самовыключение прибора */
#define		CAUSE_UNKNOWN_RESET	0xFFFFFFFF	/* Неизвестная причина-выдернули питание */


/* Главный статус */
#define		STATUS_NO_TIME		1
#define		STATUS_NO_CONST_EEPROM	2
#define		STATUS_VCP_CONNECTED	4
#define		STATUS_CMD_ERROR	8
#define		STATUS_DEV_DEFECT	16
#define		STATUS_MEM_OVERFLOW	32
#define		STATUS_DEV_TEST		64
#define		STATUS_DEV_RUN		128

/**
 * Состояние сенсоров
 */
typedef enum {
   SENS_SLEEP,
   SENS_RUN,
   SENS_STOPED,
   SENS_ERR = 0x0F,	 		
} SENS_STATE_TYPE;

/**
 * Состояние сенсоров. 8 штук
 */
typedef struct { 
   unsigned sens0 : 4;
   unsigned sens1 : 4;
   unsigned sens2 : 4;
   unsigned sens3 : 4;
   unsigned sens4 : 4;
   unsigned sens5 : 4;
   unsigned sens6 : 4;
   unsigned sens7 : 4;
} SENS_ERROR_t;



/**
 * Описывает когда случился таймаут карты и прочие ошибки 
 */
typedef struct {
    uint32_t cmd_error;
    uint32_t read_timeout;
    uint32_t write_timeout;
    uint32_t any_error;
} SD_CARD_ERROR_STRUCT;


/**
 * Имя и адрес прибора-8 байт
 */
typedef struct {
    uint8_t addr;		/* Адрес платы */
    uint8_t ver;		/* Версия ПО: 1, 2, 3, 4 итд */
    uint8_t rev;		/* Ревизия ПО 0, 1, 2 итд */
    uint8_t res0;		/* Резерв */
    uint32_t cmpl_time;		/* Время компиляции */
} DEV_ADDR_PACK;



/**
 * Счетчики обменов по УАРТ
 */
typedef struct {
    uint32_t rx_pack;		/* Принятые пакеты */
    uint32_t rx_cmd_err;	/* Ошибка в команде */
    uint32_t rx_stat_err;	/* Ошибки набегания, кадра (без стопов) и пр */
    uint32_t rx_crc_err;	/* Ошибки контрольной суммы */
    uint32_t tx_pack;		/* переданные пакеты */
    uint32_t tx_err;
} UART_COUNTS_PACK;


/* Для вещественых чисел */
typedef union {
  f32 f_val;  	
  u32 u_val;
} FLT_2_INT;


/** 
 * Статус и ошибки устройств на отправление 
 * Короткий статус (main_status) посылается при ошибке (только 1 байта + 1 len + 2 CRC16)
 * сместим на позицию для совместимости
 */
#pragma pack(4)
typedef struct {
    uint8_t main_status;	/* главный: нет времени, нет констант, нет SD, ошибка в команде, неисправен, переполнение памяти, тестирование, запущен */
    uint8_t self_test0;		/* Самотестирование и ошибки0: 0 - часы, 1 - датчик температуры, 2 - датчик давления, 3 - EEPROM, 4 - карта SD, 5 - flash... */
    uint8_t self_test1;		/* прочие устройства: 0 - насос */
#define devices self_test1
    uint8_t reset_cause;	/* Причина предыдущего сброса */

    SENS_ERROR_t sensor_error;	/* ERROR ошибки-статусы 8 сенсоров. Потом распишем */
    uint32_t eeprom;		/* Статус EEPROM. Выставленный бит - ошибка к/л параметра */
    int32_t  gs_time;		/* Время прибора. Секунды времени UNIX */

    int16_t temperature;	/* Температура внутри прибора * 10 */
    int16_t humidity;		/* Относительная влажность прибора * 10 % */

    uint32_t pressure;		/* Давление кПа */

    int16_t  batt_curr;		/* Ток потребления. мА */
    int16_t  batt_pow;		/* Напряжение батареи питания. мВ */
    uint32_t freq;		/* частота проца */

    DEV_ADDR_PACK    dev_addr;	/* Версия/ревизия/время компиляции */
    uint32_t rsvd[6];
} DEV_STATUS_PACK;


/* Переопределения malloc и free */
#define	MALLOC(x)	pvPortMalloc(x)
#define	FREE(x)	        vPortFree(x)


#endif				/* globdefs.h */
