/************************************************************************
 * �������� ������ �� ������������ COM �����
 * NB! ����� ����� ����������� ����������
 * ����� ���� ����� ��� ������?
 ***********************************************************************/
#include <string.h>
#include "usbd_cdc_core.h"
#include "usbd_cdc_vcp.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "sensor.h"
#include "status.h"
#include "eeprom.h"
#include "main.h"
#include "crc16.h"
#include "vcp.h"

#define SHORT_STATUS_SIZE       1

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
extern USBD_Class_cb_TypeDef USBD_MSC_CDC_cb;

/** @defgroup APP_VCP_Private_Variables
  * @{
  */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )	/*!< IAR Compiler */
#pragma data_alignment=4
#endif
#endif				/* USB_OTG_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;


/**
 * ��������� �� ���� ��������� - ����� �� ����� � ��������. 
 * ��������� ������ ������, ������� ������ ����������� �� ��������
 */
static XCHG_DATA_PACK xchg_buf;


/* ������� ������ */
static UART_COUNTS_PACK uart_cnt_a2341;	//-  ����� ����� �� ������!!!
static UART_COUNTS_PACK xchg_counts;

/* ��������� �����, ���� ���� �������� ������ �� ����������� ��� ����� */
static u8 temp_buf[256];

/* ������� ��� ��������� ������ ���������� ������� */
static xSemaphoreHandle vcp_tx_sem = NULL;


/****************************** ����������� �������***************************/
static void vcp_rx_reset(int);
static void vcp_send_data(void *, int);
static void vcp_transmit_task(void *);

/**
 * ������������� ������������ �����, �������� ������, ������� ����� ��������� ������
 * "�������"   ������
 */
void vcp_create_task(void)
{
    memset(&xchg_buf, 0, sizeof(xchg_buf));
    memset(&xchg_counts, 0, sizeof(UART_COUNTS_PACK));

    /* ������� ������� ��� �������� ���������� ���������� ������ */
    vSemaphoreCreateBinary(vcp_tx_sem);

#if 0
    /* COM ���� � SD ����� � ����� ���������� */
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_CDC_cb, &USR_cb);
#else
    /* ������ COM ����  */
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
#endif
    xTaskCreate(vcp_transmit_task, "Send Port Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}


/**
 * ������ �� COM-����� � �������� �����. ��� �� ������� ����������, �������
 * ����� ���������� ������ � � ������ ������ ����
 * ����� ����� �������� ������ Payload - ������ ������, ������� ���� ��������
 * FF 00 LEN CMD  PAYLOAD CRC16
 * �������� ����� �������� ������ ��� ������ ������,
 * ���� ����� �� ������� ����� ������������
 * � ������� vApplicationTickHook
 */
void vcp_rx_data(uint8_t * buf, int len)
{
    while (len--) {

	volatile uint8_t rx_byte = *buf++;

	/* ��������� CRC16. ��� ���������� ������ ��������� ����� �.�. = 0 */
	xchg_buf.rx_ind = (uint8_t) ((xchg_buf.rx_crc16 >> 8) & 0xff);
	xchg_buf.rx_crc16 = (uint16_t) (((xchg_buf.rx_crc16 & 0xff) << 8) | (rx_byte & 0xff));
	xchg_buf.rx_crc16 ^= get_crc16_table(xchg_buf.rx_ind);


	/* ������ ����� ������ ���� ������� � ��� ����� */
	if (xchg_buf.rx_beg == 0 && rx_byte == BROADCAST_ADDR) {	/* ������ ���� �������: ����� - FF */
	    xchg_buf.rx_beg = 1;	/* ���� ���� ������� */
	    xchg_buf.rx_crc16 = rx_byte;	/* ����������� ����� ����� ������� ����� */
	    xchg_buf.rx_met = 0;	/* ����� ��� ������� */
	    xchg_buf.rx_cnt = 0;	/* ������� ������� - ������ ���� ������� */
	} else {		/* ������ ����������� ����� */
	    if (xchg_buf.rx_cnt == 1) {
		if (rx_byte != 0) {	/* ���� �������� */
		    vcp_rx_reset(RX_CMD_ERR);
		}
	    } else if (xchg_buf.rx_cnt == 2) {
		if (rx_byte == 0) {	/* ���� ��������-���� ���� �� ����� */
		    vcp_rx_reset(RX_CMD_ERR);
		}
		xchg_buf.rx_len = rx_byte;	/* ������ ���� ��� ����� ���� ��������� ������� - �� ����� ���� 0! */
	    } else if (xchg_buf.rx_cnt == 3) {
		xchg_buf.rx_cmd = rx_byte;	/* ������� ���� � 3-�� �����(�� 0-��)  */
	    } else if (xchg_buf.rx_cnt > 3 && xchg_buf.rx_cnt < (xchg_buf.rx_len + 3)) {
		/* 3...��� ��������� ��������� ��� ������ ������� ���� ��� ���� */
		xchg_buf.rx_buf[xchg_buf.rx_cnt - 4] = rx_byte;	/* � �������� ����� �������� ������� */
	    } else if (xchg_buf.rx_cnt == (xchg_buf.rx_len + 3)) {	/* ��. ���� ����������� �����  */
		xchg_buf.rx_met = (uint16_t) rx_byte << 8;	/* label BADD - ��� ������� */
	    } else if (xchg_buf.rx_cnt == (xchg_buf.rx_len + 4)) {	/* ��. ���� ����������� �����  */
		xchg_buf.rx_met |= rx_byte;

		/*  Crc16 ���������� ? */
		if (xchg_buf.rx_crc16 == 0 || xchg_buf.rx_met == RX_LABEL) {
		    vcp_rx_reset(RX_OK);
		} else {
		    vcp_rx_reset(RX_CRC_ERR);	/* ������ CRC16 */
		}
	    } else {
		vcp_rx_reset(RX_CMD_ERR);
	    }
	}
	xchg_buf.rx_cnt++;	/* ������� ������� */
	if (xchg_buf.rx_cnt >= BUF_IN_SIZE) {
	    xchg_buf.rx_cnt = 0;
	}
    }
}

/**
 * ������ ��������� ������ �� ������������ ��� �����
 */
static void vcp_transmit_task(void *par)
{
    while (1) {

	/* ���� �������� */
	if (xSemaphoreTake(vcp_tx_sem, portMAX_DELAY)) {
	    u32 temp1, temp0;

	    /* ������ ������  */
	    switch (xchg_buf.rx_cmd) {

	    case UART_CMD_COMMAND_PC:
		status_get_full((DEV_STATUS_PACK *) temp_buf);	/* ������� ������ ������ */
		vcp_send_data(&((DEV_STATUS_PACK *) temp_buf)->dev_addr, sizeof(DEV_ADDR_PACK));
		break;

		/* ������ ������ ������ ���������� - ����� � ������ �������� */
	    case UART_CMD_GET_DSP_STATUS:
		status_get_full((DEV_STATUS_PACK *) temp_buf);
		vcp_send_data(temp_buf, sizeof(DEV_STATUS_PACK));
		break;

		/* �������� ������ ������ */
	    case UART_CMD_GET_COUNTS:
		memcpy(temp_buf, &xchg_counts, sizeof(UART_COUNTS_PACK));
		vcp_send_data(temp_buf, sizeof(UART_COUNTS_PACK));
		break;

		/* ������������� ����� RTC */
	    case UART_CMD_SYNC_RTC_CLOCK:
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(int));
		status_sync_time(temp0);
		temp1 = status_get_short();	/* �������� ������ */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		status_set_short(temp1);	/* ��������� ������ ����� �������� */
		break;

		/* ����� �����  */
	    case UART_CMD_DSP_RESET:
		temp1 = status_get_short();	/* �������� ������ */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		status_set_short(temp1);	/* ��������� ������ ����� �������� */
		vTaskDelay(50);
		status_dev_reset();
		break;

		/* ��������� EEPROM � ������ AB_CD */
	    case UART_CMD_GET_ADC_CONST:
		/* ����� � 2-� ������ ������ */
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(uint16_t));
		/* ���� ������ ���� �� ��� ���������� ������ � EEPROM -������� ������ "��� ������" */
		if (eeprom_read_pack((EEPROM_VALUE_PACK *) temp_buf, (uint16_t) temp0)) {
		    temp1 = status_get_short();	/* �������� ������ */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		} else {
		    temp1 = status_get_short();	/* �������� ������ */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;	/* ������� ������ EEPROM */
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(temp_buf, sizeof(EEPROM_VALUE_PACK));
		}
		break;

		/* �������� ����� */
	    case UART_CMD_PUMP_ON:
		status_pump_on();
		temp1 = status_get_short();	/* �������� ������ ����� */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* ��������� ����� */
	    case UART_CMD_PUMP_OFF:
		status_pump_off();
		temp1 = status_get_short();	/* �������� ������ ����� */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* ����� ��������� */
	    case UART_CMD_DEV_START:
		sensor_start_aqusition();	/* ��������� */
		temp1 = status_get_short();	/* �������� ������ ����� */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* ���� ��������� */
	    case UART_CMD_DEV_STOP:
		sensor_stop_aqusition();	/*���������� */
		temp1 = status_get_short();	/* �������� ������ ����� */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* ������ ����� ������ ���� ��� ����, ���� ��� - �������� ������ */
	    case UART_CMD_GET_DATA:
		if (sensor_get_pack((live_data_pack*) temp_buf) == true) {
		    vcp_send_data(temp_buf, sizeof(live_data_pack));
		} else {
		    temp1 = status_get_short();	/* �������� ������ */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;

		/* ������ ������������ */
	    case UART_CMD_INIT_TEST:
		temp1 = status_get_short();	/* �������� ������ ����� */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* �������� � EEPROM ��������� �-�� ������ 
		 * � ������ 0 ���� - ����� ������ 0..7
		 * �� ���������, ����� ������ ������ �������������.
		 * ����� ������ ������ ������, � ���� �� ����� ������ ��� �����,
		 * ������������� � ������� ����� eeprom ����� ������� ����� 
		 * FF 00 52 05 AD_DR |____data____| CRC16 
		 */
	    case UART_CMD_SET_ADC_CONST:

		/* ����� */
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(uint16_t));

		/* ������. �������� ����� 2 ����� */
		memcpy(temp_buf, xchg_buf.rx_buf + 2, sizeof(EEPROM_VALUE_PACK));

		/* ���������� ������ �� ������ */
		if (eeprom_write_pack((EEPROM_VALUE_PACK *) temp_buf, (uint16_t) temp0)) {
		    temp1 = status_get_short();	/* �������� ������ ����� */
		    temp1 |= STATUS_CMD_ERROR;	/* ������ ? */
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		} else {
		    temp1 = status_get_short();	/* �������� ������ ����� */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;

	    case UART_CMD_WRITE_LMP91K:	/*  0x11 - �������� �������� ���������� LMP91k - 20 ���� */
		/* ������. �������� 20 ���� */
		memcpy(temp_buf, xchg_buf.rx_buf, 20);
		if (sensor_write_regs((u8 *) temp_buf)) {
		    temp1 = status_get_short();	/* �������� ������ ����� */
		    temp1 |= STATUS_CMD_ERROR;	/* ������ ? */
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(temp_buf, SHORT_STATUS_SIZE);	/* ���� ������ ������� ��������� */
		} else {
		    temp1 = status_get_short();	/* �������� ������ ����� */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;


	    case UART_CMD_READ_LMP91K:	/*  0x12 - ��������� �������� ���������� LMP91k */
		memset(temp_buf, 0, sizeof(temp_buf));
		if (sensor_read_regs(temp_buf) == 0) {

		    vcp_send_data(temp_buf, 20);	/* ���� 20 �������� */
		} else {
		    temp1 = status_get_short();	/* �������� ������ */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* ��������� ������ ����� �������� */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;


	    default:
		break;
	    }
	}

	vTaskDelay(5);
    }
}

/**
 * ������� ����� ������ � ����
 */
static void vcp_send_data(void *data, int len)
{
    /* ��������� � ����� ������ � 1-�� ������� */
    memcpy(xchg_buf.tx_buf + 1, data, len);
    xchg_buf.tx_buf[0] = len;	/* � 0-� ����� ����� ������� */
    add_crc16(xchg_buf.tx_buf);
    VCP_DataTx(xchg_buf.tx_buf, len + 3);
    xchg_counts.tx_pack++;
}


/**
 * ����� �������� ��������� � ����� ���������� ������
 */
static void vcp_rx_reset(int err)
{
    /* ������ ���������� */
    xchg_buf.rx_beg = 0;
    xchg_buf.rx_cnt = 0;
    xchg_buf.rx_len = 0;

    switch (err) {
    case RX_OK:
	xchg_counts.rx_pack++;	/* �������� ������ */
	xSemaphoreGive(vcp_tx_sem);	/* ������ ������� */
	break;

    case RX_CMD_ERR:
	xchg_counts.rx_cmd_err++;	/* ������ � ������� */
	break;

    case RX_STAT_ERR:
	xchg_counts.rx_stat_err++;	/* ������ ��������� � ��. */
	break;

    case RX_CRC_ERR:
	xchg_counts.rx_crc_err++;	/* ������ � CRC */
	break;

    default:
	break;
    }
}
