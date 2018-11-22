#ifndef _COM_H
#define _COM_H

#include "main.h"


/* ������ ������-�������� */
typedef enum {
    RX_OK,
    RX_CMD_ERR,
    RX_STAT_ERR,
    RX_CRC_ERR,
} RX_STATUS_ENUM;          


/* ��������� ����� ������ ������ �� ��� - ����� */
#define BUF_IN_SIZE             512	/* ������ ������ ������ */
#define BUF_OUT_SIZE            512	/* ������ ������ �������� */
#define RX_LABEL                0xFEED	/* ���������� �����,����� CRC16 �� ������� */

typedef struct {
    uint8_t rx_buf[BUF_IN_SIZE];	/* �� ����� */
    uint8_t tx_buf[BUF_OUT_SIZE];	/* � �� �������� */

    uint8_t  rx_beg;			/* ������ ������ */
    uint8_t  rx_cmd;			/* �������  */
    uint16_t rx_cnt;			/* ������� ��������� */

    uint8_t  rx_len;			/* ����� ������� */
    uint8_t  rx_fin;			/* ����� ������ */
    uint16_t rx_ind;

    uint8_t tx_cnt;
    uint8_t tx_len;
    uint8_t tx_ind;
    uint8_t tx_fin;			/* ����� ������ */
    
    uint16_t rx_met;
    uint16_t rx_crc16;			/* ����������� ����� CRC16 */
} XCHG_DATA_PACK;



void com_create_task(void);
void com_rx_data(uint8_t);


#endif				/* com.h */
