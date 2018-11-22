#include <string.h>
#include "userfunc.h"
#include "status.h"
#include "main.h"
#include "rtc.h"

/* Здесь описан статус и функции для него */
static DEV_STATUS_PACK status;


/**
 *  Инициализация в начале работы
 */
void status_init_first(void)
{
    status.main_status = 0x03;
    status.self_test0 = 0x1c;
    status.self_test1 = 0;
    status.reset_cause = 1;

    status.dev_addr.ver = 0;	/* Версия ПО: 1, 2, 3, 4 итд */
    status.dev_addr.rev = 121;	/* Ревизия ПО 0, 1, 2 итд */
    status.dev_addr.cmpl_time = parse_date_time();	/* Время компиляции */
    status.dev_addr.addr = 7;	/* Адрес платы */

    memset(&status.sensor_error, 0xffffffff, sizeof(status.sensor_error));

    status.freq = SystemCoreClock;
}

/**
 *  Выдать первые 4 байта для маскирования
 */
u32 status_get_short(void)
{
    u32 res;

    res = (u32) status.main_status | ((u32) status.self_test0 << 8) | ((u32) status.self_test1 << 16) | ((u32) status.reset_cause << 24);

    return res;
}


/**
 *  Установить первые 4 байта для маскирования
 */
void status_set_short(uint32_t s)
{
    status.main_status = s & 0xFF;
    status.self_test0 = (s >> 8) & 0xFF;
    status.self_test1 = (s >> 16) & 0xFF;
    status.reset_cause = (s >> 24) & 0xFF;
}


/**
 * Выдать статус для других модулей
 */
void status_get_full(DEV_STATUS_PACK * pack)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    memcpy(pack, &status, sizeof(DEV_STATUS_PACK));
}


/**
 * Выдать статусы сенсоров
 */
void status_get_sensor(void *s)
{
    memcpy(s, &status.sensor_error, sizeof(u32));
}


/**
 * Установить статусы сенсоров
 */
void status_set_sensor(void *s)
{
    memcpy(&status.sensor_error, s, sizeof(u32));
}

/**
 * Выдать время прибора
 */
int32_t status_get_time(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.gs_time;
}

/**
 * Установить время прибора
 * Записать в RTC
 */
void status_set_time(int32_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.main_status &= ~STATUS_NO_TIME;
    status.gs_time = t;		/* надо установить rtc! */
}


void status_sync_time(int t)
{
    struct tm t0;

    sec_to_tm(t, &t0);

    rtc_set_time(&t0);

    status.gs_time = t;		// надо установить rtc!             
    status.main_status &= ~STATUS_CMD_ERROR;
    status.main_status &= ~STATUS_NO_TIME;	/* Убираем нет времени */
}


/**
 * Выдать температуру прибора
 */
int16_t status_get_temp(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.temperature;
}


/**
 * Установить температуру прибора (датчиком)
 */
void status_set_temp(int16_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.temperature = t;
}



/**
 * Выдать относительную влажность воздуха
 */
int16_t status_get_humidity(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.humidity;
}


/**
 * Установить относительную влажность воздуха (датчиком)
 */
void status_set_humidity(int16_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.humidity = t;
}

/**
 * Давление выдать
 */
int32_t status_get_pressure(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.pressure;	/* Давление кПа */
}

/**
 * Давление записать (датчиком)
 */
void status_set_pressure(int32_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.pressure = t;
}


/**
 *
 * Ток потребления выдать. мА 
 */
int16_t status_get_current(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.batt_curr;
}

/**
 * Ток потребления записать. мА 
 */
void status_set_current(int16_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.batt_curr = t;
}

/**
 * Напряжение батареи питания. мВ
 */
int16_t status_get_power(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    return status.batt_pow;
}

/**
 * Напряжение батареи питания. мВ
 */
void status_set_power(int16_t t)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.batt_pow = t;
}


/**
 *  Включить насос - пока включить лампу
 */
void status_pump_on(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.devices |= 0x1;
}

/**
 *  вЫключить насос - пока вЫключить лампу
 */
void status_pump_off(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    status.devices &= ~0x1;
}

/* сброс CPU */
void status_dev_reset(void)
{
    status.main_status &= ~STATUS_CMD_ERROR;
    SCB->AIRCR = 0x05FA0004;
}

/**
 * Подсоединен по VCP
 */
void status_vcp_conn(void)
{
    status.main_status |= STATUS_VCP_CONNECTED;
}

/**
 * Отсоединен от VCP. Автономная работа
 */
void status_vcp_disconn(void)
{
    status.main_status &= ~STATUS_VCP_CONNECTED;
}

/**
 * Проверка: подсоединен/разъединен
 */
bool status_is_conn(void)
{
    return (status.main_status & STATUS_VCP_CONNECTED) ? true : false;
}
