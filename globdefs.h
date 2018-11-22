#ifndef _GLOBDEFS_H
#define _GLOBDEFS_H


#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <float.h>
#include "stm32f4xx.h"


typedef uint64_t u64;
typedef int64_t s64;
typedef float f32;	/* long double �� �������������  */
typedef char  c8;

typedef unsigned char const    ucint8_t;
typedef volatile unsigned char vuint8_t;
typedef volatile unsigned long vuint32_t;

typedef enum {false, true} bool;


#if 0

#ifndef u8
#define u8 uint8_t
#endif


#ifndef c8
#define c8 char
#endif


#ifndef u32
#define u32 uint32_t
#endif


#ifndef s32
#define s32 int32_t
#endif


#ifndef bool
#define bool uint8_t
#endif


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#endif


#ifndef IDEF
#define IDEF static inline
#endif

/* �� ���� ����� ����� �������� ���� ���������� */
#define 	BROADCAST_ADDR	0xff

/* ������������ ����� ���� ������� ��� ���������  */
#define 	NUM_ALL_CHAN	       		8

/* ����� �������� �� LMP91�  */
#define 	NUM_LMP91K_SENS			4


/* ����� ��������� � ����� ������ ����� �� ������� */
#define 	NUM_ACQUIS_IN_PACK		5

#define		MAGIC				0x4b495245

/* �������� */
#define		TIMER_NS_DIVIDER	(1000000000UL)
#define		TIMER_US_DIVIDER	(1000000)
#define		TIMER_MS_DIVIDER	(1000)

/* ������� ������ ������� - ����������� �����. �� ������� � eeprom  */
#define		CAUSE_POWER_OFF		0x12345678	/* ���������� ������� */
#define		CAUSE_EXT_RESET		0xabcdef90	/* ������� ����� */
#define		CAUSE_BROWN_OUT		0xaa55aa55	/* �������� ������� */
#define		CAUSE_WDT_RESET		0x07070707	/* WDT reset (�� ����� �����������) */
#define		CAUSE_NO_LINK		0xE7E7E7E7	/* ��� ����� - �������������� ������� */
#define		CAUSE_UNKNOWN_RESET	0xFFFFFFFF	/* ����������� �������-��������� ������� */


/* ������� ������ */
#define		STATUS_NO_TIME		1
#define		STATUS_NO_CONST_EEPROM	2
#define		STATUS_VCP_CONNECTED	4
#define		STATUS_CMD_ERROR	8
#define		STATUS_DEV_DEFECT	16
#define		STATUS_MEM_OVERFLOW	32
#define		STATUS_DEV_TEST		64
#define		STATUS_DEV_RUN		128

/**
 * ��������� ��������
 */
typedef enum {
   SENS_SLEEP,
   SENS_RUN,
   SENS_STOPED,
   SENS_ERR = 0x0F,	 		
} SENS_STATE_TYPE;

/**
 * ��������� ��������. 8 ����
 */
typedef struct { 
   unsigned sens0 : 4;
   unsigned sens1 : 4;
   unsigned sens2 : 4;
   unsigned sens3 : 4;
   unsigned sens4 : 4;
   unsigned sens5 : 4;
   unsigned sens6 : 4;
   unsigned sens7 : 4;
} SENS_ERROR_t;



/**
 * ��������� ����� �������� ������� ����� � ������ ������ 
 */
typedef struct {
    uint32_t cmd_error;
    uint32_t read_timeout;
    uint32_t write_timeout;
    uint32_t any_error;
} SD_CARD_ERROR_STRUCT;


/**
 * ��� � ����� �������-8 ����
 */
typedef struct {
    uint8_t addr;		/* ����� ����� */
    uint8_t ver;		/* ������ ��: 1, 2, 3, 4 ��� */
    uint8_t rev;		/* ������� �� 0, 1, 2 ��� */
    uint8_t res0;		/* ������ */
    uint32_t cmpl_time;		/* ����� ���������� */
} DEV_ADDR_PACK;



/**
 * �������� ������� �� ����
 */
typedef struct {
    uint32_t rx_pack;		/* �������� ������ */
    uint32_t rx_cmd_err;	/* ������ � ������� */
    uint32_t rx_stat_err;	/* ������ ���������, ����� (��� ������) � �� */
    uint32_t rx_crc_err;	/* ������ ����������� ����� */
    uint32_t tx_pack;		/* ���������� ������ */
    uint32_t tx_err;
} UART_COUNTS_PACK;


/* ��� ����������� ����� */
typedef union {
  f32 f_val;  	
  u32 u_val;
} FLT_2_INT;


/** 
 * ������ � ������ ��������� �� ����������� 
 * �������� ������ (main_status) ���������� ��� ������ (������ 1 ����� + 1 len + 2 CRC16)
 * ������� �� ������� ��� �������������
 */
#pragma pack(4)
typedef struct {
    uint8_t main_status;	/* �������: ��� �������, ��� ��������, ��� SD, ������ � �������, ����������, ������������ ������, ������������, ������� */
    uint8_t self_test0;		/* ���������������� � ������0: 0 - ����, 1 - ������ �����������, 2 - ������ ��������, 3 - EEPROM, 4 - ����� SD, 5 - flash... */
    uint8_t self_test1;		/* ������ ����������: 0 - ����� */
#define devices self_test1
    uint8_t reset_cause;	/* ������� ����������� ������ */

    SENS_ERROR_t sensor_error;	/* ERROR ������-������� 8 ��������. ����� �������� */
    uint32_t eeprom;		/* ������ EEPROM. ������������ ��� - ������ �/� ��������� */
    int32_t  gs_time;		/* ����� �������. ������� ������� UNIX */

    int16_t temperature;	/* ����������� ������ ������� * 10 */
    int16_t humidity;		/* ������������� ��������� ������� * 10 % */

    uint32_t pressure;		/* �������� ��� */

    int16_t  batt_curr;		/* ��� �����������. �� */
    int16_t  batt_pow;		/* ���������� ������� �������. �� */
    uint32_t freq;		/* ������� ����� */

    DEV_ADDR_PACK    dev_addr;	/* ������/�������/����� ���������� */
    uint32_t rsvd[6];
} DEV_STATUS_PACK;


/* ��������������� malloc � free */
#define	MALLOC(x)	pvPortMalloc(x)
#define	FREE(x)	        vPortFree(x)


#endif				/* globdefs.h */
