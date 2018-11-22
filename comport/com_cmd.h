#ifndef _COM_CMD_H
#define _COM_CMD_H

#include "globdefs.h"


/*******************************************************************
 *  Входящие команды
 *******************************************************************/
typedef enum {
    UART_CMD_NONE = 0,		/*  0x00 - Нет команды */
    UART_CMD_COMMAND_PC,	/*  0x01 - Перейти в режим работы с PC или ответить Who Are you? */
    UART_CMD_GET_DSP_STATUS,	/*  0x02 - Получить статус: тестирование, данные с батареи и температуру с давлением */
    UART_CMD_GET_COUNTS,	/*  0x03 - Выдай счетчики обменов UART */
    UART_CMD_GET_ADC_CONST,	/*  0x04 - Получить Константы EEPROM */
    UART_CMD_SET_ADC_CONST,	/*  0x05 - Установить / Записать в EEPROM константы */
    UART_CMD_INIT_TEST,		/*  0x06 - Запуск тестирования */
    UART_CMD_DSP_RESET,		/*  0x07 - Сброс DSP */
    UART_CMD_POWER_OFF,		/*  0x08 - Выключение */
    UART_CMD_PUMP_ON,		/*  0x09 - Включить насос */
    UART_CMD_PUMP_OFF,		/*  0x0A - вЫключить насос */
    UART_CMD_DEV_START,		/*  0x0B - Старт измерения */
    UART_CMD_DEV_STOP,		/*  0x0C - Стоп измерения  */
    UART_CMD_GET_DATA,		/*  0x0D - Выдать пачку данных */
    UART_CMD_ZERO_ALL_EEPROM,	/*  0x0E - занулить eeprom */
    UART_CMD_SET_DSP_ADDR,	/*  0x0F - Установить адрес станции */
    UART_CMD_SYNC_RTC_CLOCK,	/*  0x10 - Установить время прибора. Синхронизация RTC */
    UART_CMD_WRITE_LMP91K,	/*  0x11 - Записать регистры микросхемы LMP91k */
    UART_CMD_READ_LMP91K,	/*  0x12 - Прочитать регистры микросхемы LMP91k */
} UART_CMD_ENUM;


#endif				 /* _COM_CMD_H */
