#ifndef _COM_H
#define _COM_H

#include "main.h"


/* Ошибки приема-передачи */
typedef enum {
    RX_OK,
    RX_CMD_ERR,
    RX_STAT_ERR,
    RX_CRC_ERR,
} RX_STATUS_ENUM;          


/* кольцевой буфер приема данных из СОМ - порта */
#define BUF_IN_SIZE             512	/* размер буфера приема */
#define BUF_OUT_SIZE            512	/* размер буфера передачи */
#define RX_LABEL                0xFEED	/* Отладочная метка,чтобы CRC16 не считать */

typedef struct {
    uint8_t rx_buf[BUF_IN_SIZE];	/* На прием */
    uint8_t tx_buf[BUF_OUT_SIZE];	/* И на передачу */

    uint8_t  rx_beg;			/* Начало пакета */
    uint8_t  rx_cmd;			/* Команда  */
    uint16_t rx_cnt;			/* Счетчик принятого */

    uint8_t  rx_len;			/* Всего принято */
    uint8_t  rx_fin;			/* Конец приема */
    uint16_t rx_ind;

    uint8_t tx_cnt;
    uint8_t tx_len;
    uint8_t tx_ind;
    uint8_t tx_fin;			/* Конец приема */
    
    uint16_t rx_met;
    uint16_t rx_crc16;			/* Контрольная сумма CRC16 */
} XCHG_DATA_PACK;



void com_create_task(void);
void com_rx_data(uint8_t);


#endif				/* com.h */
