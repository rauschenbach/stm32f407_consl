#ifndef _LOG_H
#define _LOG_H


#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "globdefs.h"
#include "status.h"


/** 
 * ����� ������, ������� ����� ���� ���������� ��������
 */
typedef enum {
    RES_NO_ERROR = 0,		// ��� ������
    RES_NO_LOG_FILE,		// ��� ��� �����

    RES_MOUNT_ERR,		// ������ ������������  
    RES_FORMAT_ERR,		// ������ ������������ ������
    RES_OPEN_LOG_ERR,		// ������ �������� ����� ����
    RES_WRITE_LOG_ERR,		// ������ ������ � ���
    RES_SYNC_LOG_ERR,		// ������ ������ � ��� (sync)
    RES_CLOSE_LOG_ERR,		// ������ �������� ����

    RES_OPEN_DATA_ERR,		// ������ �������� ����� ������
    RES_WRITE_DATA_ERR,		// ������ ������ ����� ������
    RES_CLOSE_DATA_ERR,		// ������ �������� ����� ������
    RES_WRITE_HEADER_ERR,	// ������ ������ ��������� ���������
    RES_SYNC_HEADER_ERR,	// ������ ������ ��������� (sync)
    RES_REG_PARAM_ERR,		// ������ � ����� ����������

    RES_OPEN_PARAM_ERR,		// ������ �������� ����� ����������
    RES_READ_PARAM_ERR,		// ������ ������ ����� ����������
    RES_CLOSE_PARAM_ERR,	// ������ �������� ����� ����������
    RES_TIME_PARAM_ERR,		// ������ � ������� �������
    RES_FREQ_PARAM_ERR,		// ������ � ������� �������
    RES_CONSUMP_PARAM_ERR,	// ������ � ������� �����������������
    RES_PGA_PARAM_ERR,		// ������ � ������� ��������
    RES_MODEM_TYPE_ERR,		// ������ � ������� ����� ����
    RES_MKDIR_PARAM_ERR,	// ������ � �������� �����
    RES_MAX_RUN_ERR,		// ��������� �������
    RES_CREATE_LOG_ERR,		// ������ �������� ����
    RES_CREATE_ENV_ERR,		// ������ �������� ���� �����

    RES_READ_FLASH_ERR,		// ������ ������ flash
    RES_FORMAT_TIME_ERR,	// ������ �������������� �������
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
