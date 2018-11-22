#ifndef _LOG_H
#define _LOG_H


#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "globdefs.h"
#include "status.h"


/** 
 * Здесь ощибки, которые могут быть возвражены функцией
 */
typedef enum {
    RES_NO_ERROR = 0,		// Нет ошибки
    RES_NO_LOG_FILE,		// Нет лок файла

    RES_MOUNT_ERR,		// Ошибка монтирования  
    RES_FORMAT_ERR,		// Ошибка формирования строки
    RES_OPEN_LOG_ERR,		// Ошибка открытия файла лога
    RES_WRITE_LOG_ERR,		// Ошибка записи в лог
    RES_SYNC_LOG_ERR,		// Ошибка записи в лог (sync)
    RES_CLOSE_LOG_ERR,		// Ошибка закрытия лога

    RES_OPEN_DATA_ERR,		// Ошибка открытия файла данных
    RES_WRITE_DATA_ERR,		// Ошибка записи файла данных
    RES_CLOSE_DATA_ERR,		// Ошибка закрытия файла данных
    RES_WRITE_HEADER_ERR,	// Ошибка записи минутного заголовка
    RES_SYNC_HEADER_ERR,	// Ошибка записи заголовка (sync)
    RES_REG_PARAM_ERR,		// Ошибка в файле параметров

    RES_OPEN_PARAM_ERR,		// Ошибка открытия файла параметров
    RES_READ_PARAM_ERR,		// Ошибка чтения файла параметров
    RES_CLOSE_PARAM_ERR,	// Ошибка закрытия файла параметров
    RES_TIME_PARAM_ERR,		// Ошибка в задании времени
    RES_FREQ_PARAM_ERR,		// Ошибка в задании частоты
    RES_CONSUMP_PARAM_ERR,	// Ошибка в задании енергопотребления
    RES_PGA_PARAM_ERR,		// Ошибка в задании усиления
    RES_MODEM_TYPE_ERR,		// Ошибка в задании числа байт
    RES_MKDIR_PARAM_ERR,	// Ошибка в создании папки
    RES_MAX_RUN_ERR,		// Исчерпаны запуски
    RES_CREATE_LOG_ERR,		// Ошибка создания лога
    RES_CREATE_ENV_ERR,		// Ошибка создания лога среды

    RES_READ_FLASH_ERR,		// Ошибка чтения flash
    RES_FORMAT_TIME_ERR,	// Ошибка форматирования времени
} ERROR_ResultEn;




int log_mount_fs(void);
bool log_check_mounted(void);

int log_open_log_file(void);
int log_write_log_file(char *, ...);
int log_close_log_file(void);

int log_open_data_file(void);
int log_write_data_file(void *, int);
int log_close_data_file(void);


#endif				/* log.h */
