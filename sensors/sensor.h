#ifndef _SENSOR_H
#define _SENSOR_H

#include "main.h"

#define MAX_GAS_NAME_LEN	32

/*  
 * ��� �������
*/
typedef enum {
    NO_SENSOR,			/* ��� ������� */
    I2C_SENSOR,			/* �� i2c ������������ */
    MDB_SENSOR,			/* Modbus ������ */
} sensor_type_en;

/* ������������� �����: �����, � ����� ������ � � 2-�� ������� */
typedef enum {
    INT_MODE,
    TEN_MODE,
    HUN_MODE,
} digit_mode_en;

/* ������� ���������: mv mg/m^3, ppm � %%  */
typedef enum {
    MV_MODE,	/* ����������� - �� ������������� */
    MMGM_MODE,  /* ��.� ��� */
    PPM_MODE,
    PERCENT_MODE,
} units_mode_en;

/**
 * ����� ��������� �� ���� ������� ��� ��������. 168 ���� 
 */
typedef struct SENSOR_DATA_PACK {
    int32_t time;		/* ����� ��������� ������� ������ */
    int32_t freq;		/* ������� ������, ���� ������ �� ������� ��� ����� ����� ����������� */
    struct {
	FLT_2_INT chan[NUM_ALL_CHAN];
    } data[NUM_ACQUIS_IN_PACK];
} live_data_pack;

/**
 * �������. ����� ���������� ������� �� EEPROM - 64 �����
 * ��������� �������������� ������ 
 */
typedef struct {
    sensor_type_en sens_type;		/* �������.�������� ��� sensor type */
    units_mode_en type_units;
    digit_mode_en num_digits;
    u8  num_of_gas;		/* ����� ���� � ������� */
    u8  reg_set[20];		/* ��������� - �������� ��� ��� �� ��� */
    char formula[MAX_GAS_NAME_LEN];	/* ���. ������� */
    FLT_2_INT data;		/* ������� ���������. ��� ����������� */
    float mult_coef;		/* ����������� ��������� ��� ����� ������ ��� gain */
#define		gain	mult_coef
    float zero_shift;		/* �������� ����  */
#define 	shift	zero_shift
    float max_range;		/* ��� �������� ��������� */
    float thresh_min;		/* ����� ���������������� min */
    float thresh_max;		/* ����� ���������������� ���� */
    float filter;
} channel_params_settings;

/* ��� ���������: �����, ��������� � ��� �� ��� ��� ������� */
typedef struct {
    s8 pump_level;		/* ������������������ ������ */
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
