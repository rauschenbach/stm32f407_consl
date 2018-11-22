#ifndef _COM_CMD_H
#define _COM_CMD_H

#include "globdefs.h"


/*******************************************************************
 *  �������� �������
 *******************************************************************/
typedef enum {
    UART_CMD_NONE = 0,		/*  0x00 - ��� ������� */
    UART_CMD_COMMAND_PC,	/*  0x01 - ������� � ����� ������ � PC ��� �������� Who Are you? */
    UART_CMD_GET_DSP_STATUS,	/*  0x02 - �������� ������: ������������, ������ � ������� � ����������� � ��������� */
    UART_CMD_GET_COUNTS,	/*  0x03 - ����� �������� ������� UART */
    UART_CMD_GET_ADC_CONST,	/*  0x04 - �������� ��������� EEPROM */
    UART_CMD_SET_ADC_CONST,	/*  0x05 - ���������� / �������� � EEPROM ��������� */
    UART_CMD_INIT_TEST,		/*  0x06 - ������ ������������ */
    UART_CMD_DSP_RESET,		/*  0x07 - ����� DSP */
    UART_CMD_POWER_OFF,		/*  0x08 - ���������� */
    UART_CMD_PUMP_ON,		/*  0x09 - �������� ����� */
    UART_CMD_PUMP_OFF,		/*  0x0A - ��������� ����� */
    UART_CMD_DEV_START,		/*  0x0B - ����� ��������� */
    UART_CMD_DEV_STOP,		/*  0x0C - ���� ���������  */
    UART_CMD_GET_DATA,		/*  0x0D - ������ ����� ������ */
    UART_CMD_ZERO_ALL_EEPROM,	/*  0x0E - �������� eeprom */
    UART_CMD_SET_DSP_ADDR,	/*  0x0F - ���������� ����� ������� */
    UART_CMD_SYNC_RTC_CLOCK,	/*  0x10 - ���������� ����� �������. ������������� RTC */
    UART_CMD_WRITE_LMP91K,	/*  0x11 - �������� �������� ���������� LMP91k */
    UART_CMD_READ_LMP91K,	/*  0x12 - ��������� �������� ���������� LMP91k */
} UART_CMD_ENUM;


#endif				 /* _COM_CMD_H */
