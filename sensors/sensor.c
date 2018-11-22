/***********************************************************************************
 * Проверка всех сенсоров на разных шинах и их запуск в задаче
 ***********************************************************************************/
#include <string.h>
#include "main.h"
#include "sensor.h"
#include "status.h"
#include "eeprom.h"
#include "circbuf.h"
#include "dynament.h"
#include "lmp91k.h"
#include "menu.h"
#include "adc.h"

#define		SENSOR_I2C_ADDR		0x90
#define		SENSOR_GPIO_PORT      	GPIOD	/* GPIOD */
#define 	SENSOR_GPIO_CLK       	RCC_AHB1Periph_GPIOD
#define 	CIRC_BUF_SIZE		10	/* Кольцевой буфер на 10 значений (10 секунд) */
#define 	ACQUIS_PAUSE_MS		200	/* 200 мс между измерениями. 5 измерений в секунду */

/**
 * Данные EEPROM всех сенсоров
 * Каналы для настройки с клавиатуры - 
 * в других модулях делаю их extern !!!
 */
channel_params_settings chan_set[NUM_ALL_CHAN];
extra_params_settings extra_set;


static live_data_pack live_data;
static dyna_live_data_pack dyna_data;
static CircularBuffer cb;	/* Кольцевой буфер */
static bool cir_buf_init_ok = false;

/* Регистры настроек для всех веществ 
 * На нашем газоанализаторе такие настройки LMP: 
 * NH3 TIA_GAIN 4, RLOAD 1, INT_Z 0, BIAS_SIGN 0, BIAS 0
 * NO2 TIA_GAIN 5, RLOAD 1, INT_Z 2, BIAS_SIGN 0, BIAS 0
 * O2  TIA_GAIN 4, RLOAD 2, INT_Z 2, BIAS_SIGN 0, BIAS 0
 * CO  TIA_GAIN 3, RLOAD 0, INT_Z 2, BIAS_SIGN 0, BIAS 0
*/
static const struct {
    u8 tiacn;
    u8 refcn;
    u8 modecn;
} sensor_regs[] = {
	  {0x11, 0xC0, 0x03 }, /* NH3 Аммиак  */
	  {0x10, 0xC0, 0x03 }, /* H2  Водород */
	  {0x10, 0xC0, 0x03 }, /* SF6 Гексафторид серы  */
	  {0x14, 0xd0, 0x03 }, /* NO2 */
	  {0x10, 0xC0, 0x03 }, /* SO2 Диокс. серы */
	  {0x12, 0xD0, 0x03 }, /* O2 Кислород */
	  {0x10, 0xC0, 0x03 }, /* RSH Меркапт. */
	  {0x10, 0xC0, 0x03 }, /* O3 Озон */
	  {0x10, 0xC0, 0x03 }, /* NO Окс. азота */
	  {0x10, 0xC0, 0x03 }, /* H2S Сероводород  */
	  {0x10, 0xC0, 0x03 }, /* HCN Син. кислота */
	  {0x18, 0x90, 0x03 }, /* CO Окс. углерода */
	  {0x10, 0xC0, 0x03 }, /* CO2 Диок. углерода */
	  {0x10, 0xC0, 0x03 }, /* H2CO2 Формальдегид */
	  {0x10, 0xC0, 0x03 }, /* HF Фтор. водород */
	  {0x10, 0xC0, 0x03 }, /* CHC1F2 Хладон  */
	  {0x10, 0xC0, 0x03 }, /* Cl2 Хлор */
	  {0x10, 0xC0, 0x03 }, /* HCl Хлороводород */
	  {0x10, 0xC0, 0x03 }, /* C2H5OH Этанол */
	  {0x10, 0xC0, 0x03 }, /* C6H6 Бензол */
	  {0x10, 0xC0, 0x03 }, /* C4H10 Бутан  */
	  {0x10, 0xC0, 0x03 }, /* C6H14 Гексан */
	  {0x10, 0xC0, 0x03 }, /* CH4 Метан */
	  {0x10, 0xC0, 0x03 }, /* C3H8 Пропан */
	  {0x10, 0xC0, 0x03 }, /* C2H4 Этилен  */
};

/* Задача опроса АЦП */
static void sensor_task(void *);
static void wait_status(u8);
static bool is_bad_data(u8 d)
{
	return (d == 0x00 || d == 0xFF)? true : false;
}

/**
 * Инициализация всех каналов АЦП
 * Проверка EEPROM
 */
void sensor_init(void)
{
    int i;
    static int k = 0, gas = 0;
    int err = 0;
    u32 st;
    u8 wr_regs[20] = { 0 };
    u8 rd_regs[20] = { 0 };

    /* Датчики на шине I2C */
    lmp91k_port_init();

    /* порт для датчика dynament 4 по счету с нуля */
    sens_dyna_init();

    /* Читаем EEPROM для сенсоров если EEPROM работает и убираем выбор модуля */
    if (!k) {
//      k = 1;                  // в самом начале. стоит ли всегда их читать ?
	for (i = 0; i < NUM_ALL_CHAN; i++) {
	    memset(&chan_set[i], 0, sizeof(channel_params_settings));

	    /* Если читаем хотя бы раз правильные данные с EEPROM - убираем статус "нет данных eeprom" */
	    if (eeprom_read_pack((EEPROM_VALUE_PACK *) & chan_set[i], SENS_ADDR(i))) {
		err += 1;
	    }
	}
    }
    /* Убираем ошибку - "нет констант каналов" */
    if (err == 0) {
	st = status_get_short();
	st &= ~STATUS_NO_CONST_EEPROM;
	status_set_short(st);
    }

    /* Протестируем сенсоры I2C и RS232
     * какие сенсоры включены и работают */
    status_get_sensor(&err);
    for (i = 0; i < NUM_LMP91K_SENS; i++) {

	/* Если нет сенсора - не опрашиваем по второму разу */
	if (chan_set[i].sens_type == NO_SENSOR) {
	    continue;
	}

	lmp91k_on(i);
	log_printf("Sensor %d: \n", i);

	/* Копируем 20 регистров */
	memcpy(wr_regs, chan_set[i].reg_set, 20);
	gas = chan_set[i].num_of_gas;

	gas = chan_set[i].num_of_gas;

	/* Rtia 14кОм + Rload 50 */
	if(is_bad_data(wr_regs[TIACN_REG]) && (gas <= C2H4)) {
		wr_regs[TIACN_REG] = sensor_regs[gas].tiacn;
	}
           
	/* INT_Z - 50% от 2.5 V external ref, BIAS - 0 */
	if(is_bad_data(wr_regs[REFCN_REG]) && (gas <= C2H4)) {
		wr_regs[REFCN_REG] = sensor_regs[gas].refcn;
	}
        
	/* Считаем как 3-ножечный датчик */
	if(is_bad_data(wr_regs[MODECN_REG]) && (gas <= C2H4)) {
		wr_regs[MODECN_REG] = sensor_regs[gas].modecn;
	}

	log_printf("Rtia = %3.2fk\n", lmp91k_get_tia_gain(wr_regs[TIACN_REG])/1000.);
	log_printf("Rload = %d\n", lmp91k_get_rload(wr_regs[TIACN_REG]));
	log_printf("BIAS = %d%%\n", lmp91k_get_bias(wr_regs[REFCN_REG]));
	log_printf("Zero = %d%%\n", lmp91k_get_int_zero(wr_regs[REFCN_REG]));

	/* Настраиваем и отмечаем рабочие - если ошибка, выключаем сенсор? */
	if (lmp91k_config(wr_regs) == 0 && lmp91k_check(rd_regs) == 0) {
	    err &= ~(1 << i);	/* ставим бит если ответ 0 */
	    chan_set[i].sens_type = I2C_SENSOR;
	}
    }

    /* На 5 бит */
    i = dyna_get_live_data(&dyna_data);
    i = 0;			/* Заплатка: первый раз прерывания еще не запускаются, считаем что датчик есть */
    if (i < 0) {
	err |= (1 << 4);
    } else {
	err &= ~(1 << 4);
	chan_set[4].sens_type = MDB_SENSOR;
    }

    /* Отмечаем нерабочие */
    status_set_sensor(&err);
}



/**
 * Просто установка бита на старт
 * Запускаем сразу все каналы одновременно
 */
void sensor_start_aqusition(void)
{
    u32 stat;			/* Короткий статус */

    sensor_init();
    adc_start();		/* АЦП запустить */
    cb_clear(&cb);		/* Очистим кольцевой буфер */

    stat = status_get_short();
    stat |= STATUS_DEV_RUN;
    stat &= ~STATUS_MEM_OVERFLOW;	/* Снимем переполнение если оно было */
    status_set_short(stat);
}

/**
 * Просто снятие бита на старт и сброс переполнения памяти
 */
void sensor_stop_aqusition(void)
{
    u32 stat;
    SENS_ERROR_t err;

    cb_clear(&cb);		/* Очистим кольцевой буфер */
    memset(&live_data, 0, sizeof(live_data_pack));
    stat = status_get_short();
    stat &= ~STATUS_DEV_RUN;
    stat &= ~STATUS_MEM_OVERFLOW;	/* Снимем переполнение если оно было */
    status_set_short(stat);

    for (int i = 0; i < NUM_ALL_CHAN; i++) {
	if (chan_set[i].sens_type == I2C_SENSOR) {
	    lmp91k_on(i);
	    lmp91k_stop();
	    adc_stop();
	} else if (chan_set[i].sens_type == MDB_SENSOR) {
	    dyna_stop();
	}

	/* первый сенсор вЫключен */
	status_get_sensor(&err);
	if (i == 0) {
	    err.sens0 = SENS_STOPED;
	} else if (i == 1) {
	    err.sens1 = SENS_STOPED;
	} else if (i == 2) {
	    err.sens2 = SENS_STOPED;
	} else if (i == 3) {
	    err.sens3 = SENS_STOPED;
	} else if (i == 4) {
	    err.sens4 = SENS_STOPED;
	} else if (i == 5) {
	    err.sens5 = SENS_STOPED;
	} else if (i == 6) {
	    err.sens6 = SENS_STOPED;
	} else if (i == 7) {
	    err.sens7 = SENS_STOPED;
	}
    }
    status_set_sensor(&err);
}

/**
 * Чтение любого канала в зависимости от настройки
 */
int sensor_read_data(u8 num)
{
    FLT_2_INT val;
    f32 data;

    val.u_val = -1;

    if (num > 7) {
	return val.u_val;
    }

    /* Если включен */
    if (chan_set[num].sens_type == I2C_SENSOR) {
	lmp91k_on(num);		/* Выбираем канал */
	data = (float) adc_read_chan(num);	/* Получаем мV */
	val.f_val = data;
    } else if (chan_set[num].sens_type == MDB_SENSOR) {
	/* Сенсор Dynament дает вывод в процентах */
	dyna_get_live_data(&dyna_data);
	val.f_val = dyna_data.Reading.f_val;
    }

    return val.u_val;
}

/**
 * Создаем задачу сенсоров
 */
void sensor_create_task(void)
{
    /* Кольцевой буфер  */
    sensor_init_cb();

    /* АЦП для сенсора I2C */
    adc_init();

    sensor_init();

    /* Создаем задачу */
    xTaskCreate(sensor_task, "SensorTask", SENSOR_TASK_STACK_SIZE, NULL, SENSOR_TASK_PRIORITY, NULL);
}

/* Задача опроса сенсоров - читаем АЦП */
static void sensor_task(void *p)
{
    int j = 0, i, type, k;
    int data;
    u32 st;
    SENS_ERROR_t err;
    char str[32];


    /* первый сенсор включен   */
    status_get_sensor(&err);
    err.sens1 = SENS_SLEEP;
    status_set_sensor(&err);


    while (1) {

	/* Ждем бита статуса на запуск или открытия семафора */
	wait_status(STATUS_DEV_RUN);

	/* Ставим время в секундах с 1970 года */
	live_data.time = get_sec_ticks();

	/* 200 мс */
	live_data.freq = ACQUIS_PAUSE_MS;


	/* В пачку если выводим в USB-VCP
	 * все прочие каналы - данные будут обработаны 
	 */
	k = 0;
	for (j = 0; j < NUM_ACQUIS_IN_PACK; j++) {

	    /* Опрашиваем со всех каналов - получаем секундное измерение */
	    for (i = 0; i < NUM_ALL_CHAN; i++) {

		/*  нет сенсора */
		type = chan_set[i].sens_type;
		if (type == NO_SENSOR) {
		    continue;
		}
		/* Получили в милливольтах. число float в виде u32 */
		data = sensor_read_data(i);

		/* Перевели в плавающее  */
		live_data.data[j].chan[i].u_val = data;
	    }
	    vTaskDelay(ACQUIS_PAUSE_MS);
	    k++;
	}

	st = status_get_short();

	/* ставим статус - "буфер полон". Очищаем при чтении. */
	if (cb_is_full(&cb)) {
	    st |= STATUS_MEM_OVERFLOW;
	    status_set_short(st);
	}

	/* Отсылаем в кольцевой буфер в любом случае */
	cb_write(&cb, &live_data);

	/* Записываем данные для вывода на экран (после фильтра или как то еще) */
	for (i = 0; i < NUM_ALL_CHAN; i++) {
	    chan_set[i].data.u_val = live_data.data[2].chan[i].u_val;
	}

	sec_to_str(live_data.time, str);
	log_printf("%s: t °C: %d, %d\n", str, (int) chan_set[0].data.f_val, k);
    }
}

/**
 *  Эта функция откачивает данные из пакета
 *  Получить данные - если false, данных нет 
 */
bool sensor_get_pack(live_data_pack * buf)
{
    u32 st;
    bool res = false;

    /* Убираем статус "переполнение" */
    if (!cb_is_empty(&cb) && buf != NULL) {
	st = status_get_short();
	cb_read(&cb, buf);
	st &= ~STATUS_MEM_OVERFLOW;
	status_set_short(st);
	res = true;
    }
    return res;			/*  "Данные не готовы"? */
}

/**
 * Записать регистры - для теста
 */
int sensor_write_regs(u8 * regs)
{
    return lmp91k_config(regs);
}


/**
 * Прочитать регистры - для теста
 */
int sensor_read_regs(u8 * regs)
{
    return lmp91k_check(regs);
}

/**
 * Инициализация кольцевого буфера. Один раз вызывается
 */
int sensor_init_cb(void)
{
    u32 st;
    int res = 0;

    /* Уже инициализирован */
    if (cir_buf_init_ok == true) {
	return res;
    }

    st = status_get_short();

    /* Не можем выделить память! */
    if (cb_init(&cb, CIRC_BUF_SIZE)) {
	st |= STATUS_DEV_DEFECT;	/* Ставим ошибка прибора */
	res = -1;
	cir_buf_init_ok = false;
    } else {
	st &= ~STATUS_DEV_DEFECT;	/* Уберем "ошибка прибора" */
	cir_buf_init_ok = true;
    }

    status_set_short(st);
    return res;
}


/**
 * Ждем бита статуса на запуск или открытия семафора
 */
static void wait_status(u8 stat)
{
    while (!(status_get_short() & stat)) {
	vTaskDelay(100);
    }
}


/**
 * Записать в eeprom данные которые сответсвуют таблице для каждого вещества 
 */
void sensor_write_test_data(void)
{
	int f, i;
        
	for (i = 0; i < NUM_ALL_CHAN; i++) {
            f = chan_set[i].num_of_gas;            
            chan_set[i].reg_set[TIACN_REG] = sensor_regs[f].tiacn;
            chan_set[i].reg_set[REFCN_REG] = sensor_regs[f].refcn;
            chan_set[i].reg_set[MODECN_REG] = sensor_regs[f].modecn;           
	}
}
