#ifndef _SENSOR_H
#define _SENSOR_H

#include "main.h"

#define MAX_GAS_NAME_LEN	32

/*  
 * Тип сенсора
*/
typedef enum {
    NO_SENSOR,			/* Нет сенсора */
    I2C_SENSOR,			/* На i2c электрохимия */
    MDB_SENSOR,			/* Modbus сенсор */
} sensor_type_en;

/* представление чисел: целые, с одной точкой и с 2-мя точками */
typedef enum {
    INT_MODE,
    TEN_MODE,
    HUN_MODE,
} digit_mode_en;

/* Единицы измерения: mv mg/m^3, ppm и %%  */
typedef enum {
    MV_MODE,	/* Милливольты - не пересчитываем */
    MMGM_MODE,  /* мг.м куб */
    PPM_MODE,
    PERCENT_MODE,
} units_mode_en;

/**
 * Пакет измерений со всех каналов для передачи. 168 байт 
 */
typedef struct SENSOR_DATA_PACK {
    int32_t time;		/* Время измерения первого пакета */
    int32_t freq;		/* Частота опроса, если запуск по таймеру или пауза между измерениями */
    struct {
	FLT_2_INT chan[NUM_ALL_CHAN];
    } data[NUM_ACQUIS_IN_PACK];
} live_data_pack;

/**
 * Сенсоры. Часть параметров берется из EEPROM - 64 байта
 * Параметры измерительного канала 
 */
typedef struct {
    sensor_type_en sens_type;		/* Включен.выключен или sensor type */
    units_mode_en type_units;
    digit_mode_en num_digits;
    u8  num_of_gas;		/* Номер газа в таблице */
    u8  reg_set[20];		/* Установки - регистры или что то еще */
    char formula[MAX_GAS_NAME_LEN];	/* Хим. формула */
    FLT_2_INT data;		/* Текущее измерение. Или расчитанное */
    float mult_coef;		/* Коэффициент умножения для сырых данных или gain */
#define		gain	mult_coef
    float zero_shift;		/* Смещение нуля  */
#define 	shift	zero_shift
    float max_range;		/* МАХ диапазон измерений */
    float thresh_min;		/* Порог чувствительности min */
    float thresh_max;		/* Порог чувствительности макс */
    float filter;
} channel_params_settings;

/* Доп параметры: насос, громкость и что то еще что добавим */
typedef struct {
    s8 pump_level;		/* Производительность насоса */
    s8 sound_level;
} extra_params_settings;


void sensor_init(void);
int  sensor_init_cb(void);
void sensor_create_task(void);
void sensor_start_aqusition(void);
void sensor_stop_aqusition(void);
bool sensor_get_pack(live_data_pack*);

int  sensor_write_regs(u8 *);
int  sensor_read_regs(u8 *);
void sensor_write_test_data(void);

#endif				/* sensor.h */
