/******************************************************************************
 * Функции перевода дат, проверка контрольной суммы т.ж. здесь 
 * Все функции считают время от начала Эпохи (01-01-1970)
 * Все функции с маленькой буквы
 *****************************************************************************/
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "userfunc.h"
#include "status.h"
#include "main.h"


static const char *monthes[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

/* Таблица windows */
static const u8 cp1251_tab[] = "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";

/* Таблица koi8r */
static const u8 koi8r_tab[] =
    // а   б   в   г   д   е   ё   ж   з   и   й   к   л   м   н   о   п   р   с  т   у    ф   х   ц   ч   ш   щ   ъ   ы   ь   э   ю   я
    "\x8A\x8B\xA0\x90\x8D\x8E\x86\x9f\xA3\x92\x93\x94\x95\x96\x97\x98\x99\x9B\x9C\x9D\x9E\x8F\x91\x8C\xA7\xA4\xA6\xA8\xA2\xA1\xA5\x89\x9A"
    "\xAA\xAB\xC0\xB0\xAD\xAE\x87\xBF\xC3\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBB\xBC\xBD\xBE\xAF\xB1\xAC\xC7\xC4\xC6\xC8\xC2\xC1\xC5\xA9\xBA";

/**
 * Функция преобразования CP1251 в KOI8 
 * для использования KOI8 шрифта
 */
c8 *win_to_koi8(c8 * str)
{
    int len, i, j;
    len = strlen(str);

    for (i = 0; i < len; i++) {
	for (j = 0; j < 66; j++) {
	    if (str[i] == cp1251_tab[j]) {
		str[i] = koi8r_tab[j];
		break;
	    }
	}
    }
    return &str[0];
}

/**
 * Функция преобразования KOI8 в CP1251
 * для вывода на консоль
 */
c8 *koi8_to_win(const c8 * str)
{
    int len, i, j;
    len = strlen(str) % MAX_MENU_STRING;

    static char win_str[MAX_MENU_STRING];

    memcpy(win_str, str, len);

    for (i = 0; i < len; i++) {
	for (j = 0; j < 66; j++) {
	    if (win_str[i] == koi8r_tab[j]) {
		win_str[i] = cp1251_tab[j];
		break;
	    }
	}
    }
    win_str[i] = 0;
    return &win_str[0];
}


/**
 * Для PRINTF - печать на экране
*/
int log_printf(const char *fmt, ...)
{
    int ret = 0;
#if 0
    char buf[256];
    va_list list;

    va_start(list, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, list);
    va_end(list);

    if (ret < 0) {
	return ret;
    }


//    VCP_DataTx((uint8_t *)buf, ret);

    printf(buf);	
#endif
    return ret;
}

/* Строку в верхний регистр   */
void str_to_cap(char *str, int len)
{
    int i = 0;

    while (i++ < len)
	str[i] = (str[i] > 0x60) ? str[i] - 0x20 : str[i];
}



/* Выдапть число секунд */
long get_sec_ticks(void)
{
    return status_get_time();
}

void set_time(long t)
{
    status_sync_time(t);
}

/**
 * Переводит секунды (time_t) с начала Эпохи в формат struct tm
 */
int sec_to_tm(long ls, struct tm *time)
{
    struct tm *tm_ptr;

    if ((int) ls != -1 && time != NULL) {
	tm_ptr = gmtime((time_t *) & ls);

	/* Записываем дату, что получилось */
	memcpy(time, tm_ptr, sizeof(struct tm));
	return 0;
    } else
	return -1;
}

/**
 * Время struct tm в секунды 
 */
long tm_to_sec(struct tm *tm_time)
{
    long r;

    /* Делаем секунды */
    r = mktime(tm_time);
    return r;			/* -1 или нормальное число  */
}

/**
 * Записываем дату в формате: 08-11-15 - 08:57:22 
 */
int sec_to_str(long ls, char *str)
{
    struct tm t0;
    int res = 0;

    if (sec_to_tm((long) ls, &t0) != -1) {
	sprintf(str, "%02d-%02d-%02d %02d:%02d:%02d ", t0.tm_mday, t0.tm_mon + 1, (t0.tm_year - 100) % 99, t0.tm_hour, t0.tm_min, t0.tm_sec);
    } else {
	sprintf(str, " [set time error] ");
	res = -1;
    }
    return res;
}



/**
 * Разбор констант __TIME__ и __DATA__
 * Mar 14 2017 13:53:31 
 */
int parse_date_time(void)
{
    char buf[5];
    char *str_date = __DATE__;
    char *str_time = __TIME__;
    int i, len, x;
    struct tm time;

    len = strlen(str_date);

    for (i = 0; i < len; i++)
	if (isalpha(str_date[i]))	// найдем первую букву
	    break;

    if (i >= len)
	return -1;

    memset(buf, 0, 5);
    strncpy(buf, str_date + i, 3);	// 3 символа скопировали
    str_to_cap(buf, 4);
    for (i = 0; i < 12; i++) {
	if (strncmp(buf, monthes[i], 3) == 0) {
	    time.tm_mon = i;	// месяц нашли
	    break;
	}
    }
    if (time.tm_mon > 11) {
	return -1;
    }
    // ищем первую цыфру
    for (i = 0; i < len; i++)
	if (isdigit(*(str_date + i))) {
	    x = i;
	    break;
	}

    memset(buf, 0, 5);
    strncpy(buf, str_date + x, 2);	// 2 символа
    time.tm_mday = atoi(buf);

    if (time.tm_mday > 31)
	return -1;

    // ищем год, пробел после цыфр
    for (i = x; i < len; i++) {
	if (*(str_date + i) == 0x20) {
	    break;
	}
    }
    strncpy(buf, str_date + i + 1, 4);	// 4 символа
    time.tm_year = atoi(buf) - 1900;

    if (time.tm_year < 117)
	return -1;


    /* 3... Разбираем строку времени */
    memset(buf, 0, 5);

    /* часы */
    strncpy(buf, str_time, 2);
    time.tm_hour = atoi(buf);

    if (time.tm_hour > 24)
	return -1;


    /* минуты */
    strncpy(buf, str_time + 3, 2);
    time.tm_min = atoi(buf);
    if (time.tm_min > 60)
	return -1;


    /* секунды */
    strncpy(buf, str_time + 6, 2);
    time.tm_sec = atoi(buf);
    if (time.tm_sec > 60)
	return -1;

    return mktime(&time);
}
