/************************************************************************
 * Передача данных по виртуальному COM порту
 * NB! Очень много копирования происходит
 * Может быть стоит это убрать?
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
 * Указатель на нашу структуру - пакет на прием и передачу. 
 * Небольшой размер буфера, поэтому память динамически не выделяем
 */
static XCHG_DATA_PACK xchg_buf;


/* Щетчики обмена */
static UART_COUNTS_PACK uart_cnt_a2341;	//-  ЗДЕСЬ МАЖЕТ ПО ПАМЯТИ!!!
static UART_COUNTS_PACK xchg_counts;

/* Временный буфер, куда буду помещать данные на отправление или прием */
static u8 temp_buf[256];

/* Семафор для получения данных передающей задачей */
static xSemaphoreHandle vcp_tx_sem = NULL;


/****************************** статические функции***************************/
static void vcp_rx_reset(int);
static void vcp_send_data(void *, int);
static void vcp_transmit_task(void *);

/**
 * Инициализация виртуального порта, создание задачи, которая будет принимать данные
 * "Ленивая"   задача
 */
void vcp_create_task(void)
{
    memset(&xchg_buf, 0, sizeof(xchg_buf));
    memset(&xchg_counts, 0, sizeof(UART_COUNTS_PACK));

    /* Создать семафор для передачи управления передающей задаче */
    vSemaphoreCreateBinary(vcp_tx_sem);

#if 0
    /* COM порт и SD карта в одном устройтсве */
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_CDC_cb, &USR_cb);
#else
    /* Только COM порт  */
    USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
#endif
    xTaskCreate(vcp_transmit_task, "Send Port Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}


/**
 * данные из COM-порта в приемный буфер. Это не функция прерывания, поэтому
 * можно передавать данные и и делать прочие вещи
 * Буфер према содержит только Payload - нужные данные, команда идет отдельно
 * FF 00 LEN CMD  PAYLOAD CRC16
 * Наверное стоит добавить таймер для сброса приема,
 * если пакет со стороны хоста неправильный
 * в функцию vApplicationTickHook
 */
void vcp_rx_data(uint8_t * buf, int len)
{
    while (len--) {

	volatile uint8_t rx_byte = *buf++;

	/* Вычисляем CRC16. При правильном приеме последние числа д.б. = 0 */
	xchg_buf.rx_ind = (uint8_t) ((xchg_buf.rx_crc16 >> 8) & 0xff);
	xchg_buf.rx_crc16 = (uint16_t) (((xchg_buf.rx_crc16 & 0xff) << 8) | (rx_byte & 0xff));
	xchg_buf.rx_crc16 ^= get_crc16_table(xchg_buf.rx_ind);


	/* Пришел самый первый байт посылки и наш адрес */
	if (xchg_buf.rx_beg == 0 && rx_byte == BROADCAST_ADDR) {	/* Первый байт приняли: СТАРТ - FF */
	    xchg_buf.rx_beg = 1;	/* Один байт приняли */
	    xchg_buf.rx_crc16 = rx_byte;	/* Контрольная сумма равна первому байту */
	    xchg_buf.rx_met = 0;	/* Метка для отладки */
	    xchg_buf.rx_cnt = 0;	/* Счетчик пакетов - первый байт приняли */
	} else {		/* Пришли последующие байты */
	    if (xchg_buf.rx_cnt == 1) {
		if (rx_byte != 0) {	/* рвем передачу */
		    vcp_rx_reset(RX_CMD_ERR);
		}
	    } else if (xchg_buf.rx_cnt == 2) {
		if (rx_byte == 0) {	/* рвем передачу-нуля быть не может */
		    vcp_rx_reset(RX_CMD_ERR);
		}
		xchg_buf.rx_len = rx_byte;	/* Второй байт это длина всей следующей посылки - не может быть 0! */
	    } else if (xchg_buf.rx_cnt == 3) {
		xchg_buf.rx_cmd = rx_byte;	/* Команда идет с 3-го байта(от 0-ля)  */
	    } else if (xchg_buf.rx_cnt > 3 && xchg_buf.rx_cnt < (xchg_buf.rx_len + 3)) {
		/* 3...все остальное считается как единая посылка если она есть */
		xchg_buf.rx_buf[xchg_buf.rx_cnt - 4] = rx_byte;	/* В приемный буфер принятую посылку */
	    } else if (xchg_buf.rx_cnt == (xchg_buf.rx_len + 3)) {	/* Ст. байт контрольной суммы  */
		xchg_buf.rx_met = (uint16_t) rx_byte << 8;	/* label BADD - для отладки */
	    } else if (xchg_buf.rx_cnt == (xchg_buf.rx_len + 4)) {	/* Мл. байт контрольной суммы  */
		xchg_buf.rx_met |= rx_byte;

		/*  Crc16 правильная ? */
		if (xchg_buf.rx_crc16 == 0 || xchg_buf.rx_met == RX_LABEL) {
		    vcp_rx_reset(RX_OK);
		} else {
		    vcp_rx_reset(RX_CRC_ERR);	/* Ошибка CRC16 */
		}
	    } else {
		vcp_rx_reset(RX_CMD_ERR);
	    }
	}
	xchg_buf.rx_cnt++;	/* Счетчик пакетов */
	if (xchg_buf.rx_cnt >= BUF_IN_SIZE) {
	    xchg_buf.rx_cnt = 0;
	}
    }
}

/**
 * Задача обработки данных из виртуального ком порта
 */
static void vcp_transmit_task(void *par)
{
    while (1) {

	/* Ждем семафора */
	if (xSemaphoreTake(vcp_tx_sem, portMAX_DELAY)) {
	    u32 temp1, temp0;

	    /* Разбор команд  */
	    switch (xchg_buf.rx_cmd) {

	    case UART_CMD_COMMAND_PC:
		status_get_full((DEV_STATUS_PACK *) temp_buf);	/* Получим полный статус */
		vcp_send_data(&((DEV_STATUS_PACK *) temp_buf)->dev_addr, sizeof(DEV_ADDR_PACK));
		break;

		/* Выдать полный статус устройства - будет в буфере передачи */
	    case UART_CMD_GET_DSP_STATUS:
		status_get_full((DEV_STATUS_PACK *) temp_buf);
		vcp_send_data(temp_buf, sizeof(DEV_STATUS_PACK));
		break;

		/* Счетчики обмена выдать */
	    case UART_CMD_GET_COUNTS:
		memcpy(temp_buf, &xchg_counts, sizeof(UART_COUNTS_PACK));
		vcp_send_data(temp_buf, sizeof(UART_COUNTS_PACK));
		break;

		/* Синхронизация часов RTC */
	    case UART_CMD_SYNC_RTC_CLOCK:
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(int));
		status_sync_time(temp0);
		temp1 = status_get_short();	/* Получили статус */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		status_set_short(temp1);	/* Сохраняем статус после передачи */
		break;

		/* Сброс платы  */
	    case UART_CMD_DSP_RESET:
		temp1 = status_get_short();	/* Получили статус */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		status_set_short(temp1);	/* Сохраняем статус после передачи */
		vTaskDelay(50);
		status_dev_reset();
		break;

		/* Прочитать EEPROM с адреса AB_CD */
	    case UART_CMD_GET_ADC_CONST:
		/* Адрес в 2-х первых байтах */
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(uint16_t));
		/* Если читаем хотя бы раз правильные данные с EEPROM -убираем статус "нет данных" */
		if (eeprom_read_pack((EEPROM_VALUE_PACK *) temp_buf, (uint16_t) temp0)) {
		    temp1 = status_get_short();	/* Получили статус */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		} else {
		    temp1 = status_get_short();	/* Получили статус */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;	/* Снимаем ошибку EEPROM */
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(temp_buf, sizeof(EEPROM_VALUE_PACK));
		}
		break;

		/* Включить насос */
	    case UART_CMD_PUMP_ON:
		status_pump_on();
		temp1 = status_get_short();	/* Получили статус сразу */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* вЫключить насос */
	    case UART_CMD_PUMP_OFF:
		status_pump_off();
		temp1 = status_get_short();	/* Получили статус сразу */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* Старт измерения */
	    case UART_CMD_DEV_START:
		sensor_start_aqusition();	/* Запустили */
		temp1 = status_get_short();	/* Получили статус сразу */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* Стоп измерений */
	    case UART_CMD_DEV_STOP:
		sensor_stop_aqusition();	/*Остановили */
		temp1 = status_get_short();	/* Получили статус сразу */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* Выдать пачку данных ЕСЛИ ОНИ ЕСТЬ, если нет - короткий статус */
	    case UART_CMD_GET_DATA:
		if (sensor_get_pack((live_data_pack*) temp_buf) == true) {
		    vcp_send_data(temp_buf, sizeof(live_data_pack));
		} else {
		    temp1 = status_get_short();	/* Получили статус */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;

		/* Запуск тестирования */
	    case UART_CMD_INIT_TEST:
		temp1 = status_get_short();	/* Получили статус сразу */
		vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		break;

		/* Записать в EEPROM константы №-го канала 
		 * в данных 0 байт - номер канала 0..7
		 * Не проверяем, чтобы размер данных соответсвовал.
		 * берем просто размер пакета, и если он будет меньше чем нужно,
		 * соответсвенно в верхнюю часть eeprom будет записан мусор 
		 * FF 00 52 05 AD_DR |____data____| CRC16 
		 */
	    case UART_CMD_SET_ADC_CONST:

		/* Адрес */
		memcpy(&temp0, xchg_buf.rx_buf, sizeof(uint16_t));

		/* Данные. Учитыаем длину 2 байта */
		memcpy(temp_buf, xchg_buf.rx_buf + 2, sizeof(EEPROM_VALUE_PACK));

		/* Записываем данные по адресу */
		if (eeprom_write_pack((EEPROM_VALUE_PACK *) temp_buf, (uint16_t) temp0)) {
		    temp1 = status_get_short();	/* Получили статус сразу */
		    temp1 |= STATUS_CMD_ERROR;	/* Ошибка ? */
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		} else {
		    temp1 = status_get_short();	/* Получили статус сразу */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;

	    case UART_CMD_WRITE_LMP91K:	/*  0x11 - Записать регистры микросхемы LMP91k - 20 байт */
		/* Данные. Учитыаем 20 байт */
		memcpy(temp_buf, xchg_buf.rx_buf, 20);
		if (sensor_write_regs((u8 *) temp_buf)) {
		    temp1 = status_get_short();	/* Получили статус сразу */
		    temp1 |= STATUS_CMD_ERROR;	/* Ошибка ? */
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(temp_buf, SHORT_STATUS_SIZE);	/* Шлем статус верхней программе */
		} else {
		    temp1 = status_get_short();	/* Получили статус сразу */
		    temp1 &= ~STATUS_NO_CONST_EEPROM;
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
		    vcp_send_data(&temp1, SHORT_STATUS_SIZE);
		}
		break;


	    case UART_CMD_READ_LMP91K:	/*  0x12 - Прочитать регистры микросхемы LMP91k */
		memset(temp_buf, 0, sizeof(temp_buf));
		if (sensor_read_regs(temp_buf) == 0) {

		    vcp_send_data(temp_buf, 20);	/* Шлем 20 значений */
		} else {
		    temp1 = status_get_short();	/* Получили статус */
		    temp1 |= STATUS_CMD_ERROR;
		    status_set_short(temp1);	/* Сохраняем статус после передачи */
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
 * Послать любые данные в порт
 */
static void vcp_send_data(void *data, int len)
{
    /* Скопируем в буфер вывода с 1-го символа */
    memcpy(xchg_buf.tx_buf + 1, data, len);
    xchg_buf.tx_buf[0] = len;	/* в 0-м байте длина посылки */
    add_crc16(xchg_buf.tx_buf);
    VCP_DataTx(xchg_buf.tx_buf, len + 3);
    xchg_counts.tx_pack++;
}


/**
 * Сброс приемных счетчиков и вызов передающей задачи
 */
static void vcp_rx_reset(int err)
{
    /* Всегда сбрасываем */
    xchg_buf.rx_beg = 0;
    xchg_buf.rx_cnt = 0;
    xchg_buf.rx_len = 0;

    switch (err) {
    case RX_OK:
	xchg_counts.rx_pack++;	/* Принятые пакеты */
	xSemaphoreGive(vcp_tx_sem);	/* Отдать семафор */
	break;

    case RX_CMD_ERR:
	xchg_counts.rx_cmd_err++;	/* Ошибка в команде */
	break;

    case RX_STAT_ERR:
	xchg_counts.rx_stat_err++;	/* Ошибка набегания и пр. */
	break;

    case RX_CRC_ERR:
	xchg_counts.rx_crc_err++;	/* Ошибка в CRC */
	break;

    default:
	break;
    }
}
