/* ��������� �������: ������� ����, ���������� �������, ������ �� SD ����� */

#include "status.h"
#include "userfunc.h"
#include "log.h"
#include "main.h"
#include "ff.h"


#define	  	MAX_TIME_STRLEN		        26	/* ����� ������ �� ��������  */
#define   	MAX_DATA_FILE_SIZE              134217728	/* 128 ����� */
#define   	MAX_LOG_FILE_SIZE		1048576	/* 1 ����� */
#define   	MAX_START_FILE_SIZE		1048576	/* 1 ����� */

#define   	MAX_FILE_NAME_LEN	        31	/* ����� ����� ������� ��� ���������� � '\0' */
#define		LOG_FILE_NAME		        "file.log"
#define		START_FILE_NAME		        "start.log"
#define 	ERROR_LOG_NAME		        "error.log"


/*************************************************************************************
 *     ��� ���������� �� ������ � ������ ������ 
 *************************************************************************************/
static FATFS fatfs;		/* File system object - ����� ������ �� global? ���! */
static DIR dir;			/* ���������� ��� ������� ��� ����� - ����� ������ �� global? */
static FIL start_file;		/* Start file object */
static FIL log_file;		/* Log file object */
static FIL data_file;		/* File object ��� ��� */

/**
 * ������������� �������� �������
 * ��� ������������ ������ ������ � FLASH
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
 * ����� ����������� ��� ���?
 */
bool log_check_mounted(void)
{
    return (bool)fatfs.fs_type;
}


/**
 * ������� LOG ����, ���� ����� ������� ���������
 */
int log_open_log_file(void)
{
    int res = RES_OPEN_LOG_ERR;
    int i;

    /* ��������� ������ ��� � ������ ��������� �� ����� */
    do {
	if (log_check_mounted() == false) {
	    break;
	}

	/* ���� ��� �� - ������� log */
	res = f_open(&log_file, LOG_FILE_NAME, FA_WRITE | FA_READ | FA_OPEN_ALWAYS);
	if (res) {
	    break;
	}

	/* ��������� ������ � ����� ���������� */
	i = f_size(&log_file);
	if (i > MAX_LOG_FILE_SIZE) {
	    f_truncate(&log_file);
	    i = 0;
	}

	/* ���������� ��������� ����� */
	f_lseek(&log_file, i);
	res = RES_NO_ERROR;
    } while (0);
    return res;
}

/**
 * ������ ������ � ��� ����, ���������� ������� ��������. � �������� ������!
 * ����� �� 10 �����?
 */
int log_write_log_file(char *fmt, ...)
{
    char str[256];
    FRESULT res;		/* Result code */
    unsigned bw;		/* ��������� ��� �������� ����  */
    int i;
    va_list p_vargs;		/* return value from vsnprintf  */

    do {
	/* �� ����������� (��� �� - �������� �� PC), ��� � ��������� �������� */
	if (log_file.fs == NULL) {
	    res = (FRESULT)RES_MOUNT_ERR;
	    break;
	}

	/* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
	i = status_get_time();
        sec_to_str((long)i, str);

	/* ��������� ������ */
	va_start(p_vargs, fmt);
	i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);
	va_end(p_vargs);
	if (i < 0) {		/* formatting error?            */
	    res = (FRESULT)RES_FORMAT_ERR;
	    break;
	}

	/* ������� �������� ������ �� UNIX (�� � ������!) */
	for (i = MAX_TIME_STRLEN + 4; i < sizeof(str) - 3; i++) {
	    if (str[i] == 0x0d || str[i] == 0x0a) {
		str[i] = 0x0d;	// ������� ������
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

	/* ����������� �������! */
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
 * ������� ���-����
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
 * ����� ������ ���� � �������� � ������ ���� ������
 * ���� ������ ������ � ���� START
 */
int log_write_data_file(void *data, int len)
{
    unsigned bw;		/* ��������� ��� �������� ����  */
    FRESULT res;		/* Result code */

    res = f_write(&data_file, (char *) data, len, &bw);
    if (res) {
	return RES_WRITE_DATA_ERR;
    }
    return RES_NO_ERROR;	/* �������� OK */
}


/**
 * ������� ���� ���-����� ���� ������� ������ �� ���� 
 */
int log_close_data_file(void)
{
    FRESULT res;		/* Result code */

    if (data_file.fs == NULL) {	// ��� ����� ���
	return RES_CLOSE_DATA_ERR;
    }

    /* ����������� ������� */
    res = f_sync(&data_file);
    if (res) {
	return RES_CLOSE_DATA_ERR;
    }

    res = f_close(&data_file);
    if (res) {
	return RES_CLOSE_DATA_ERR;
    }
    return FR_OK;		/* ��� ���������! */
}
