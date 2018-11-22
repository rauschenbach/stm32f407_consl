/* Сервисные функции: ведение лога, подстройка таймера, запись на SD карту */
#include "status.h"
#include "userfunc.h"
#include "log.h"
#include "main.h"
#include "ff.h"


#define	  	MAX_TIME_STRLEN		        26	/* Длина строки со временем  */
#define   	MAX_DATA_FILE_SIZE              134217728	/* 128 Мбайт */
#define   	MAX_LOG_FILE_SIZE		1048576	/* 1 Мбайт */
#define   	MAX_START_FILE_SIZE		1048576	/* 1 Мбайт */

#define   	MAX_FILE_NAME_LEN	        31	/* Длина файла включая имя директории с '\0' */
#define		LOG_FILE_NAME		        "file.log"
#define		START_FILE_NAME		        "start.log"
#define 	ERROR_LOG_NAME		        "error.log"


/*************************************************************************************
 *     Эти переменные не видимы в других файлах 
 *************************************************************************************/
static FATFS fatfs;		/* File system object - можно убрать из global? нет! */
static DIR dir;			/* Директория где храница все файло - можно убрать из global? */
static FIL start_file;		/* Start file object */
static FIL log_file;		/* Log file object */
static FIL data_file;		/* File object для АЦП */

/**
 * Инициализация файловой системы
 * При монтировании читаем данные с FLASH
 */
int log_mount_fs(void)
{
    int res = RES_MOUNT_ERR;

    // DS card init   sd_card_mux_config();

    do {
	f_mount(0, &fatfs);
	res = FR_OK;
    } while (0);

    return res;
}

/**
 * Карта монтирована или нет?
 */
bool log_check_mounted(void)
{
    return (bool)fatfs.fs_type;
}


/**
 * Открыть LOG файл, куда будем бросать сообщения
 */
int log_open_log_file(void)
{
    int res = RES_OPEN_LOG_ERR;
    int i;

    /* Открываем старый лог и ставим указатель на конец */
    do {
	if (log_check_mounted() == false) {
	    break;
	}

	/* Если все ОК - открыли log */
	res = f_open(&log_file, LOG_FILE_NAME, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	if (res) {
	    break;
	}

	/* Определим размер и будем дописывать */
	i = f_size(&log_file);
	if (i > MAX_LOG_FILE_SIZE) {
	    f_truncate(&log_file);
	    i = 0;
	}

	/* Переставим указатель файла */
	f_lseek(&log_file, i);
	res = RES_NO_ERROR;
    } while (0);
    return res;
}

/**
 * Запись строки в лог файл, возвращаем сколько записали. С временем ВСЕГДА!
 * Режем по 10 мБайт?
 */
int log_write_log_file(char *fmt, ...)
{
    char str[256];
    FRESULT res;		/* Result code */
    unsigned bw;		/* Прочитано или записано байт  */
    int i;
    va_list p_vargs;		/* return value from vsnprintf  */

    do {
	/* Не монтировано (нет фс - работаем от PC), или с карточкой проблемы */
	if (log_file.fs == NULL) {
	    res = (FRESULT)RES_MOUNT_ERR;
	    break;
	}

	/* Получаем текущее время - MAX_TIME_STRLEN символов с пробелом - всегда пишем */
	i = status_get_time();
        sec_to_str((long)i, str);

	/* Разбираем строку */
	va_start(p_vargs, fmt);
	i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);
	va_end(p_vargs);
	if (i < 0) {		/* formatting error?            */
	    res = (FRESULT)RES_FORMAT_ERR;
	    break;
	}

	/* Заменим переносы строки на UNIX (не с начала!) */
	for (i = MAX_TIME_STRLEN + 4; i < sizeof(str) - 3; i++) {
	    if (str[i] == 0x0d || str[i] == 0x0a) {
		str[i] = 0x0d;	// перевод строки
		str[i + 1] = 0x0a;	// Windows
		str[i + 2] = 0;
		break;
	    }
	}

	res = f_write(&log_file, str, strlen(str), &bw);
	if (res) {
	    res = (FRESULT)RES_WRITE_LOG_ERR;
	    break;
	}

	/* Обязательно запишем! */
	res = f_sync(&log_file);
	if (res) {
	    res = (FRESULT)RES_SYNC_LOG_ERR;
	    break;
	}
	res = (FRESULT)RES_NO_ERROR;
    } while (0);
    return res;
}


/**
 * Закрыть лог-файл
 */
int log_close_log_file(void)
{
    int res = RES_NO_ERROR;		/* Result code */

    res = f_close(&log_file);
    if (res) {
	res = RES_CLOSE_LOG_ERR;
    }
    return  res;
}

/**
 * Здесь должно быть и открытие и запись фала данных
 * плюс запись строки в файл START
 */
int log_write_data_file(void *data, int len)
{
    unsigned bw;		/* Прочитано или записано байт  */
    FRESULT res;		/* Result code */

    res = f_write(&data_file, (char *) data, len, &bw);
    if (res) {
	return RES_WRITE_DATA_ERR;
    }
    return RES_NO_ERROR;	/* Записали OK */
}


/**
 * Закрыть файл АЦП-перед этим сбросим буферы на диск 
 */
int log_close_data_file(void)
{
    FRESULT res;		/* Result code */

    if (data_file.fs == NULL) {	// нет файла еще
	return RES_CLOSE_DATA_ERR;
    }

    /* Обязательно запишем */
    res = f_sync(&data_file);
    if (res) {
	return RES_CLOSE_DATA_ERR;
    }

    res = f_close(&data_file);
    if (res) {
	return RES_CLOSE_DATA_ERR;
    }
    return FR_OK;		/* Все нормально! */
}
