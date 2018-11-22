/***********************************************************************
 *    Меню выбора на ЖК дисплее
 *    За образец взято меню на миландре
 *    IdleFunc() поменять на реальную функцию в конце
 ***********************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "win_str.h"
#include "eeprom.h"
#include "indic.h"
#include "lmp91k.h"
#include "menu.h"
#include "main.h"

/* статические функции */
static void vMenuTask(void *);
static void menu_init(void);
static void make_rus_menu(void);
static void SelFunc(void);
static void UpFunc(void);
static void RightFunc(void);
static void LeftFunc(void);
static void DownFunc(void);
static void ReturnFunc(void);
static void ReadKey(void);
static bool enterMenu(menu_state_en);
static bool leaveMenu(void);

/* Функции обработки меню */
static void menu_rtc_set(void);
static void main_chan_set(void);
static void menu_pump_set(void);
static void menu_volume_set(void);
static void menu_about_set(void);
static void chan_coef_set(void);
static void addit_chan_set(void);
static void menu_ranges_set(void);
static void menu_acquis_set(void);
static void calibr_menu_set(void);
static void menu_vcp_conn(void);
static void zero_all_chan(void);

/* Menu description */
typedef void (*tMenuFunc) (void);
typedef struct sMenuItem *tMenuItem;
typedef struct sMenu *tMenu;

/**
  * @brief  Menu item structure definition
  */
static struct sMenuItem {
    c8 psTitle[MAX_MENU_STRING];
    tMenuFunc pfMenuFunc;
    tMenu psSubMenu;
};

/**
  * @brief  Menu structure definition
  */
static struct sMenu {
    c8 psTitle[MAX_MENU_STRING];
    tMenuItem psItems;
    uint32_t nItems;
};

/**
 * Структура для хождения по уровням меню в основном режиме 
 */
static struct {
    u32 MenuItemIndex;
    u32 MenuLevel;
    tMenuItem ptrMenuItem;
    tMenuItem ptrCurrentMenuItem;
    tMenu ptrPrevMenu[MAX_MENU_LEVELS];
    tMenu ptrCurrentMenu;
} MainMenuShifter;

/**
 * Для настройки тех меню, где нужно вводить данные
 */
static struct {
    char str0[32];		/* Буфер для ввода */
    int x_index;		/* Индикатор нажатия вправо - влево */
    int y_index;		/* ндикатор нажатия вверх - вниз */
    int sel_index;		/* Выбор. т.е. "Применить"  */
    int rtc_time;		/* Время RTC для подстройки */
    struct tm t0;
} TimeMenuData;

/* Не люблю так делать, но сделаю их внешними из другого модуля !!! */
extern channel_params_settings chan_set[NUM_ALL_CHAN];
extern extra_params_settings extra_set;

/* КА которые определяет какая настройка параметров щас идет */
static menu_state_en menu_state = MAIN_MENU_STATE;


/************************* Third level menu ********************************************/
/* Уровни для меню "настройки" */

/* Общие настройки  */
struct sMenuItem CommonSettingsMenuItems[] = {
    {"Дата/Время", menu_rtc_set, NULL},
    {"Произ-ть насоса", menu_pump_set, NULL},
    {"Звуковое оповещение", menu_volume_set, NULL},
    {"Память", IdleFunc, NULL},
};
struct sMenu CommonSettingsMenu = { "Общие настройки", CommonSettingsMenuItems, countof(CommonSettingsMenuItems) };

/* Настройки каналов  */
struct sMenuItem ChannelSettingsMenuItems[] = {
    {"Основные настройки", main_chan_set, NULL},
    {"Коэффициенты каналов", chan_coef_set, NULL},
    {"Диапазоны измерений", menu_ranges_set, NULL},
    {"Доп. настройки", addit_chan_set, NULL},
};
struct sMenu ChannelSettingsMenu = { "Настройка каналов", ChannelSettingsMenuItems, countof(ChannelSettingsMenuItems) };

/* Калибровка каналов */
struct sMenuItem CalibrMenuItems[] = {
    {"Точки калибровки", IdleFunc, NULL},
    {"Ввод", IdleFunc, NULL}
};
struct sMenu CalibrMenu = { "Калибровка", CalibrMenuItems, countof(CalibrMenuItems) };

/* Обнуление каналов */
struct sMenuItem ZeroinMenuItems[] = {
    {"Все каналы", zero_all_chan, NULL},
    {"Выбранные каналы", IdleFunc, NULL},
};
struct sMenu ZeroinMenu = { "Обнуление каналов", ZeroinMenuItems, countof(ZeroinMenuItems) };

/**************************************** Second level menu *********************************/
/* Настройки */
struct sMenuItem SettingsMenuItems[] = {
    {"Общие настройки", IdleFunc, &CommonSettingsMenu},
    {"Настройка каналов", IdleFunc, &ChannelSettingsMenu},
    {"Калибровка", calibr_menu_set, NULL},
    {"Обнуление каналов", /*IdleFunc */ zero_all_chan, /*&ZeroinMenu */ NULL}
};
struct sMenu SettingsMenu = { "Настройки", SettingsMenuItems, countof(SettingsMenuItems) };

/* Измерения  */
struct sMenuItem MeasMenuItems[] = {
    {"Все каналы", IdleFunc, NULL},
    {"Выбранные каналы", IdleFunc, NULL},
};
struct sMenu MeasMenu = { "Измерения", MeasMenuItems, countof(MeasMenuItems) };

/**************************** Основное меню *******************************************/
/* Main menu */
struct sMenuItem MainMenuItems[] = {
    {"Настройки", IdleFunc, &SettingsMenu},
    {"Измерения", menu_acquis_set, NULL},
    {"О приборе", menu_about_set, NULL}
};
struct sMenu MainMenu = { "Главное меню", MainMenuItems, countof(MainMenuItems) };

/***************************************************************************************/
/**
 * Обнуление всех каналов. Полное стирание EEPROM 
 */
static void zero_all_chan(void)
{
    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */

    enterMenu(MENU_ZERO_ALL_CHAN_STATE);

    /* Заголовок "Обнуление все каналы" */
    show_caption(20, y_pos[0], ZeroinMenuItems[0].psTitle);

    dog_DrawStrP(20, y_pos[1], MENU_FONT, DOG_PSTR(StrZeroinChan0));
    dog_DrawStrP(20, y_pos[2], MENU_FONT, DOG_PSTR(StrZeroinChan1));
    dog_DrawStrP(20, y_pos[3], MENU_FONT, DOG_PSTR(StrZeroinChan2));

    /* Нажатие кнопки "Select" - Стираем всю EEPROM */
    if (leaveMenu()) {
	for (int i = 0; i < NUM_ALL_CHAN; i++) {
	    memset(&chan_set[i], 0, sizeof(channel_params_settings));
	    eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[i], SENS_ADDR(i));
	    log_printf("write eeprom: %d\n", i);
	}
    }
}

/* Меню измерения (со всех включенных каналов) */
static void menu_acquis_set(void)
{
    static int half = 0;
    static int vert = 0;

    if (enterMenu(MENU_ACQUIS_STATE)) {
	sensor_start_aqusition();
    }

    /* Заголовок "Измерения" */
    show_caption(20, 90, MainMenuItems[1].psTitle);

    /* Вывести измерение - развернуть по горизонтали или вертикали при нажатии кнопок */
    show_acquis(8, chan_set, vert, half);

    /* Если нажаты кнопки вправо-влево-переключимся на гор. представление */
    if (TimeMenuData.x_index) {
	half += TimeMenuData.x_index;
	half %= 2;
	vert = 0;
	TimeMenuData.x_index = 0;
    }

    /* Выставим все измерения сразу */
    if (TimeMenuData.y_index) {
	vert = 1;
	TimeMenuData.y_index = 0;
    }

    /*  Нажатие кнопки "Select" - выходим из измерений или нет? */
    leaveMenu();
}

/* Меню О приборе */
static void menu_about_set(void)
{
    enterMenu(MENU_ABOUT_STATE);

    /* Заголовок "О приборе" */
    show_caption(30, 90, MainMenuItems[2].psTitle);

    dog_DrawStrP(20, 60, CAPT_FONT, DOG_PSTR(StrAbout0));
    dog_DrawStrP(20, 40, CAPT_FONT, DOG_PSTR(StrAbout1));

    /*  Нажатие кнопки "Select" */
    leaveMenu();
}

/**
 * Громкость
 */
static void menu_volume_set(void)
{
    enterMenu(MENU_VOLUME_SET_STATE);

    /* Заголовок "Насос" */
    show_caption(20, 90, CommonSettingsMenuItems[2].psTitle);

    /* уровень громкости : Меняем положение по X */
    if (TimeMenuData.x_index) {
	extra_set.sound_level += (TimeMenuData.x_index * 5);
	if (extra_set.sound_level < 0) {
	    extra_set.sound_level = 0;
	} else if (extra_set.sound_level > 100) {
	    extra_set.sound_level = 100;
	}
	TimeMenuData.x_index = 0;
    }
    show_volume(extra_set.sound_level);

    /*  Нажатие кнопки "Select" */
    leaveMenu();
}

/**
 * Производительность насоса. Установка
 */
static void menu_pump_set(void)
{
    enterMenu(MENU_PUMP_SET_STATE);

    /* Заголовок "Насос" */
    show_caption(20, 90, CommonSettingsMenuItems[1].psTitle);

    /* Меняем положение по X */
    if (TimeMenuData.x_index) {
	extra_set.pump_level += (TimeMenuData.x_index * 2);
	if (extra_set.pump_level < 0) {
	    extra_set.pump_level = 0;
	} else if (extra_set.pump_level > 100) {
	    extra_set.pump_level = 100;
	}
	TimeMenuData.x_index = 0;
    }
    show_pump(extra_set.pump_level);

    /*  Нажатие кнопки "Select" */
    leaveMenu();
}

/**
 * Установка каналов (часть)
 */
static void main_chan_set(void)
{
    /* 8 позиции в 4 строках */
    static const u8 x_pos[][8] = {
	{58, 82},
	{8},
	{112},
	{112},
    };
    /* Размер (ширина) курсора (так как может менятся) */
    static const u8 cursor[][8] = {
	{10, 40},
	{140},
	{40},
	{40}
    };

    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */
    char str[64];
    static int i = 0;		/* Столбцы */
    static int k = 0;		/* строки */
    static int chan = 0;	/* Номер канала */
    static volatile int f = 0;
    int t;

    /* Поменяли состояние */
    enterMenu(MAIN_SET_CHANNEL_STATE);

    /* Заголовок "Основне настройки" */
    show_caption(20, y_pos[0], ChannelSettingsMenuItems[0].psTitle);

    /* Номер канала если выбираем кнопку вверх/вниз */
    show_onoff_chan(10, y_pos[1], StrNumChan, chan, chan_set[chan].sens_type);

    /* Показываем только если канал включен */
    if (chan_set[chan].sens_type) {
	show_formula(10, y_pos[2], DOG_PSTR(chan_set[chan].formula));

	/* Единицы измерений */
	t = chan_set[chan].type_units;
	if (t == MMGM_MODE) {
	    sprintf(str, "%s%s", StrUnits, StrMg);
	} else if (t == PPM_MODE) {
	    sprintf(str, "%s%s", StrUnits, "ppm");
	} else if (t == PERCENT_MODE) {
	    sprintf(str, "%s%s", StrUnits, " % ");
	} else {
	    sprintf(str, "%s%s", StrUnits, "mV");
	}

	show_units(10, y_pos[3], str);

	/* Дробная часть */
	show_fract(10, y_pos[4], StrDigits, chan_set[chan].num_digits);
    }

    /* Меняем положение по X */
    if (TimeMenuData.x_index) {
	int save = i;
	i += TimeMenuData.x_index;

	/* Вперед */
	if (i > save) {

	    /* Следующая строка или превышение размера */
	    if (i > 7 || x_pos[k][i] < 1) {
		k++;
		i = 0;
	    }
	    k %= 4;		/* Не более 3-х  */

	    /* Назад */
	} else if (i < save) {
	    if (i < 0 || x_pos[k][i] < 1) {
		i = 0;
		k--;
		if (k < 0) {
		    k = 3;
		}
	    }
	}
	TimeMenuData.x_index = 0;	/* Обязательно! */
    }

    /* Перемещаемся для коррекции. если канал включен */
    if (chan_set[chan].sens_type == NO_SENSOR) {
	k = 0;
    }
    dog_XorBox(x_pos[k][i], y_pos[k + 1] - 2, x_pos[k][i] + cursor[k][i], y_pos[k + 1] + 14);	// Включен

    /* Нажимаем кнопку вверх/вниз - коррекция */
    if (TimeMenuData.y_index) {
	if (k == 0) {
	    if (i == 0) {
		chan += TimeMenuData.y_index;	/* меняем номер канала */
		if (chan > (NUM_ALL_CHAN - 1)) {
		    chan = 0;
		} else if (chan < 0) {
		    chan = (NUM_ALL_CHAN - 1);
		}
	    } else if (i == 1) {
		t = chan_set[chan].sens_type;	/* Ключен или выключен */
		t += TimeMenuData.y_index;

		if (t < 0) {
		    t = 2;
		} else if (t > 2) {
		    t = 0;
		}
		chan_set[chan].sens_type = (sensor_type_en) t;
	    }
	} else if (k == 1) {	/* Формула  - пока выбираем из 8-ми примеров */
	    f += TimeMenuData.y_index;	/* Меняем индекс в формуле  */
	    if (f > (int) (sizeof(StrSubst) / sizeof(StrSubst[0]) - 1)) {	/* Приведение типа обязательно! */
		f = 0;
	    } else if (f < 0) {
		f = sizeof(StrSubst) / sizeof(StrSubst[0]) - 1;
	    }

	    /* Номер вещества */
	    strcpy(chan_set[chan].formula, StrSubst[f]);
	    chan_set[chan].num_of_gas = f;

	} else if (k == 2) {	/* Единицы измерений */
	    t = chan_set[chan].type_units;
	    t += TimeMenuData.y_index;
	    if (t > 3) {
		t = 0;
	    } else if (t < 0) {
		t = 3;
	    }

	    chan_set[chan].type_units = (units_mode_en) t;
	} else if (k == 3) {	/* Число цифр */
	    t = chan_set[chan].num_digits;
	    t += TimeMenuData.y_index;
	    if (t > 2) {
		t = 0;
	    } else if (t < 0) {
		t = 2;
	    }
	    chan_set[chan].num_digits = (digit_mode_en) t;
	}
	TimeMenuData.y_index = 0;	/* Обязательно! */
    }

    /* Нажатие кнопки "Select" - сохраняем в EEPROM и будем возвращаться к последнему измененному каналу */
    if (leaveMenu()) {
	/* sensor_write_test_data(); */

	for (t = 0; t < NUM_ALL_CHAN; t++) {
	    if (chan_set[t].sens_type != NO_SENSOR) {
		f = eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[t], SENS_ADDR(t));
		log_printf("write eeprom: %d\n", f);
	    }
	}
    }
}

/** 
 * Установка дополнительных настроек 
 * Измемнение регистров, если датчик на lmp91k
 */
static void addit_chan_set(void)
{
    /* 8 позиции в 4 строках */
    static const u8 x_pos[][4] = {
	{58, 82},
	{35, 120},
	{50, 80},
	{70},
    };

    /* Размер (ширина) курсора (так как может менятся) */
    static const u8 cursor[][4] = {
	{10, 40},
	{55, 40},
	{25, 40},
	{40}
    };

    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */
    //   char str[64];
    u8 reg;
    static int i = 0;		/* Столбцы */
    static int k = 0;		/* строки */
    static int chan = 0;	/* Номер канала */
    static volatile int f = 0;
    int t;

    enterMenu(ADDIT_SET_CHANNEL_STATE);

    /* Заголовок "Доп. настройки" */
    show_caption(10, 90, ChannelSettingsMenuItems[3].psTitle);

    /* Номер канала и его тип если выбираем кнопку вверх/вниз */
    show_onoff_chan(10, y_pos[1], StrNumChan, chan, chan_set[chan].sens_type);

    if (chan_set[chan].sens_type == I2C_SENSOR) {
	/* TIA gain и Rload */
	reg = chan_set[chan].reg_set[TIACN_REG];
	show_tiacn_reg(10, y_pos[2], reg);

	/* Смещение */
	reg = chan_set[chan].reg_set[REFCN_REG];
	show_bias(10, y_pos[3], reg);

	/* Внутренний нуль от напряжения опоры */
	show_int_zero(10, y_pos[4], reg);
    }

    /* Перемещаемся для коррекции. если канал включен */
    if (chan_set[chan].sens_type != I2C_SENSOR) {
	k = 0;
    }
    /* Выделяем */
    dog_XorBox(x_pos[k][i], y_pos[k + 1] - 2, x_pos[k][i] + cursor[k][i], y_pos[k + 1] + 14);

    /* Меняем положение по X */
    if (TimeMenuData.x_index) {
	int save = i;
	i += TimeMenuData.x_index;

	/* Вперед */
	if (i > save) {

	    /* Следующая строка или превышение размера */
	    if (i > 1 || x_pos[k][i] < 1) {
		k++;
		i = 0;
	    }
	    k %= 4;		/* Не более 3-х  */

	    /* Назад */
	} else if (i < save) {
	    if (i < 0 || x_pos[k][i] < 1) {
		i = 0;
		k--;
		if (k < 0) {
		    k = 1;
		}
	    }
	}
	TimeMenuData.x_index = 0;	/* Обязательно! */
    }

    /* Нажимаем кнопку вверх/вниз - коррекция */
    if (TimeMenuData.y_index) {
	if (k == 0) {
	    if (i == 0) {
		chan += TimeMenuData.y_index;	/* меняем номер канала */
		if (chan > (NUM_ALL_CHAN - 1)) {
		    chan = 0;
		} else if (chan < 0) {
		    chan = (NUM_ALL_CHAN - 1);
		}
	    } else if (i == 1) {
		t = chan_set[chan].sens_type;	/* Ключен или выключен */
		t += TimeMenuData.y_index;

		if (t < 0) {
		    t = 2;
		} else if (t > 2) {
		    t = 0;
		}
		chan_set[chan].sens_type = (sensor_type_en) t;
	    }
	} else if (k == 1) {	/* Gain или Load */
	    if (i == 0) {	/* Gain от 0 до 7-ми */
		f += TimeMenuData.y_index;
		if (f > 7) {
		    f = 0;
		} else if (f < 0) {
		    f = 7;
		}
		/* Затрем нулями только 3 места */
		chan_set[chan].reg_set[TIACN_REG] &= ~0x1C;
		chan_set[chan].reg_set[TIACN_REG] |= (f << 2);
	    } else if (i == 1) {
		f += TimeMenuData.y_index;
		if (f > 3) {
		    f = 0;
		} else if (f < 0) {
		    f = 3;
		}
		chan_set[chan].reg_set[TIACN_REG] &= ~0x03;
		chan_set[chan].reg_set[TIACN_REG] |= f;
	    }
	} else if (k == 2) {
	    if (i == 0) {	/* Знак смещения */
		f += TimeMenuData.y_index;
		if (f > 1) {
		    f = 0;
		} else if (f < 0) {
		    f = 1;
		}
		/* Освобождаем одно место  */
		chan_set[chan].reg_set[REFCN_REG] &= ~0x10;
		chan_set[chan].reg_set[REFCN_REG] |= (f << 4);
	    } else if (i == 1) {	/* Смещение в процентах */
		f += TimeMenuData.y_index;
		if (f > 13) {
		    f = 0;
		} else if (f < 0) {
		    f = 13;
		}
		/* Освобождаем 4 места  */
		chan_set[chan].reg_set[REFCN_REG] &= ~0x0F;
		chan_set[chan].reg_set[REFCN_REG] |= f;
	    }
	} else if (k == 3) {
	    f += TimeMenuData.y_index;
	    if (f > 3) {
		f = 0;
	    } else if (f < 0) {
		f = 3;
	    }
	    /* Освобождаем 2 места  */
	    chan_set[chan].reg_set[REFCN_REG] &= ~(3 << 5);
	    chan_set[chan].reg_set[REFCN_REG] |= (f << 5);
	}
	TimeMenuData.y_index = 0;
    }

    /*  Нажатие кнопки "Select" - записываем те каналы, которые поменяли */
    if (leaveMenu()) {
	int f;

	for (int t = 0; t < NUM_ALL_CHAN; t++) {

	    /*
	       Для кислорода в ppm
	       if (chan_set[t].num_of_gas == O2) {
	       chan_set[t].gain = -281.671;
	       chan_set[t].shift = 602776.28;
	       }
	     */

	    if (chan_set[t].sens_type == I2C_SENSOR) {
		f = eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[t], SENS_ADDR(t));
		log_printf("write eeprom: %d\n", f);
	    }
	}
    }
}

/**
 * Установка коеффициентов каналов 
 */
static void chan_coef_set(void)
{
    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */

    /* 8 позиции в 4 строках */
    static const u8 x_pos[][8] = {
	{58},
	{105, 140},
	{105, 140},
    };
    /* Размер (ширина) курсора (так как может менятся) */
    static const u8 cursor[][8] = {
	{10},
	{30, 20},
	{30, 20},
    };

    static int i = 0;		/* Столбцы */
    static int k = 0;		/* строки */
    static int chan = 0;	/* Номер канала */

    enterMenu(MENU_CHAN_COEF_STATE);

    /* Заголовок "Коэфициенты" */
    show_caption(10, y_pos[0], ChannelSettingsMenuItems[1].psTitle);

    /* Номер канала если выбираем кнопку вверх/вниз */
    show_num_chan(10, y_pos[1], StrNumChan, chan);

    /* Коэф. умножения  */
    show_mult_coef(10, y_pos[2], StrMultCoef, chan_set[chan].mult_coef);

    /* Смещение нуля  */
    show_mult_coef(10, y_pos[3], StrZeroShift, chan_set[chan].zero_shift);


    dog_XorBox(x_pos[k][i], y_pos[k + 1] - 2, x_pos[k][i] + cursor[k][i], y_pos[k + 1] + 14);	// Включен


    /* Меняем положение по X */
    if (TimeMenuData.x_index) {
	int save = i;
	i += TimeMenuData.x_index;

	/* Вперед */
	if (i > save) {

	    /* Следующая строка или превышение размера */
	    if (i > 1 || x_pos[k][i] < 1) {
		k++;
		i = 0;
	    }
	    k %= 3;		/* Не более 3-х  */

	    /* Назад */
	} else if (i < save) {
	    if (i < 0 || x_pos[k][i] < 1) {
		i = 1;
		k--;
		if (k < 0) {
		    k = 2;
		} else if (k == 0) {
		    i = 0;
		}
	    }
	}
	TimeMenuData.x_index = 0;	/* Обязательно! */
    }

    /* Нажимаем кнопку вверх/вниз - коррекция chan */
    if (TimeMenuData.y_index) {

	/* Меняем номер канала */
	if (k == 0) {
	    chan += TimeMenuData.y_index;

	    if (chan > (NUM_ALL_CHAN - 1)) {
		chan = 0;
	    } else if (chan < 0) {
		chan = (NUM_ALL_CHAN - 1);
	    }
	} else if (k == 1) {

	    if (i == 0) {	/* Целые  */
		chan_set[chan].mult_coef += TimeMenuData.y_index;	/* Меняем число "усиление канала"  */
	    } else if (i == 1) {	/* Дробные */
		chan_set[chan].mult_coef += TimeMenuData.y_index / 100.0;
	    }

	    if (chan_set[chan].mult_coef > 100.0) {
		chan_set[chan].mult_coef = -100.0;
	    } else if (chan_set[chan].mult_coef < -100.0) {
		chan_set[chan].mult_coef = 100.0;
	    }
	} else if (k == 2) {
	    if (i == 0) {	/* Целые  */
		chan_set[chan].zero_shift += TimeMenuData.y_index;	/* Меняем число "Смещение"  */
	    } else if (i == 1) {	/* Дробные */
		chan_set[chan].zero_shift += TimeMenuData.y_index / 100.0;
	    }

	    if (chan_set[chan].zero_shift > 100.0) {
		chan_set[chan].zero_shift = -100.0;
	    } else if (chan_set[chan].zero_shift < -100.0) {
		chan_set[chan].zero_shift = 100.0;
	    }
	}
	/* Если отпускаем клавишу */
	TimeMenuData.y_index = 0;	/* Обязательно! */
    }


    /* Нажатие кнопки "Select" */
    if (leaveMenu()) {
	int f;
	for (int t = 0; t < NUM_ALL_CHAN; t++) {
	    f = eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[t], SENS_ADDR(t));
	    log_printf("write eeprom: %d\n", f);
	}
    }
}

/////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/**
 * Калибровка каналов, пока по двум точкам прямой
 */
static void calibr_menu_set(void)
{
    static const u8 x_pos[][5] = {
	{53},
	{105},
//      {67, 74, 81, 88, 95, 102},
//      {67, 74, 81, 88, 95, 102},

	{67, 74, 81, 95, 102},
	{67, 74, 81, 95, 102},
    };

    /* Размер (ширина) курсора (так как может менятся) */
    static const u8 cursor[][5] = {
	{10},
	{30},
	{8, 8, 8, 8, 8},
	{8, 8, 8, 8, 8},
    };

    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */
    static int i = 0;		/* Столбцы */
    static int k = 0;		/* строки */
//    static int t = 0;
    static int chan = 0;	/* Номер канала */
    static int unit = 0;
    static f32 fp0[NUM_ALL_CHAN], fp1[NUM_ALL_CHAN];                    /*vvv:  Точки калибровки для 8-ми каналов - расчитать их из gain и shift !!!!! */
    static bool bok0[NUM_ALL_CHAN] = {false}, bok1[NUM_ALL_CHAN] = {false};     /* Откалибровано/ если меняем числа - убираем */
    static f32 mv0[NUM_ALL_CHAN], mv1[NUM_ALL_CHAN];                    /* Милливольты для расчета точек прямой */
    char str[32];

    /* Запустим измерения */
    if (enterMenu(MENU_CALIBR_STATE)) {
	sensor_start_aqusition();
    }

    /* Заголовок "Точки калибровки" */
    show_caption(20, y_pos[0], CalibrMenuItems[0].psTitle);

    /* Номер канала и если он включен - выбираем кнопку вверх/вниз */
    show_onoff_chan(5, y_pos[1], StrNumChan, chan, chan_set[chan].sens_type);

    /* Меняем положение по X */
    if (TimeMenuData.x_index) {
	int save = i;
	i += TimeMenuData.x_index;
	/* Вперед */
	if (i > save) {

	    /* Следующая строка или превышение размера */
	    if (i > 4 || x_pos[k][i] < 1) {
		k++;
		i = 0;
	    }
	    k %= 4;		/* Не более 3-х  */

	    /* Назад */
	} else if (i < save) {
	    if (i < 0 || x_pos[k][i] < 1) {
		i = 0;
		k--;
		if (k < 0) {
		    k = 1;
		}
	    }
	}
	TimeMenuData.x_index = 0;
    }

    /* Нажимаем кнопку вверх/вниз - коррекция chan */
    if (TimeMenuData.y_index) {
	/* Меняем номер канала в 1-ой строке */
	if (k == 0) {
	    chan += TimeMenuData.y_index;

	    if (chan > (NUM_ALL_CHAN - 1)) {
		chan = 0;
	    } else if (chan < 0) {
		chan = (NUM_ALL_CHAN - 1);
	    }
	} else if (k == 1) {	// единицы измерений
	    /*   
	       unit += TimeMenuData.y_index;
	       if(unit < PPM_MODE) {
	       unit = PERCENT_MODE;
	       } else if(unit > PERCENT_MODE){
	       unit = PPM_MODE;
	       } 
	     */
	    unit = PERCENT_MODE;
	} else if (k == 2) {	// точка 0
	    f32 fidx = fp0[chan];

	    // старшая цифра
	    if (i == 0) {
		fidx += TimeMenuData.y_index * 100;
	    } else if (i == 1) {
		fidx += (f32) TimeMenuData.y_index * 10;
	    } else if (i == 2) {
		fidx += (f32) TimeMenuData.y_index;
	    } else if (i == 3) {
		fidx += (f32) TimeMenuData.y_index / 10.0;
	    } else if (i == 4) {
		fidx += (f32) TimeMenuData.y_index / 100.0;
	    }

	    if (fidx > 100.0)
		fidx = 100.0;
	    if (fidx < 0.0)
		fidx = 0;

            
            bok0[chan] = false;
            bok1[chan] = false;            
	    fp0[chan] = fidx;

	} else if(k == 3) { // точка 1        
	    f32 fidx = fp1[chan];

	    // старшая цифра
	    if (i == 0) {
		fidx += TimeMenuData.y_index * 100;
	    } else if (i == 1) {
		fidx += (f32) TimeMenuData.y_index * 10;
	    } else if (i == 2) {
		fidx += (f32) TimeMenuData.y_index;
	    } else if (i == 3) {
		fidx += (f32) TimeMenuData.y_index / 10.0;
	    } else if (i == 4) {
		fidx += (f32) TimeMenuData.y_index / 100.0;
	    }

	    if (fidx > 100.0)
		fidx = 100.0;
	    if (fidx < 0.0)
		fidx = 0;

            bok0[chan] = false;
            bok1[chan] = false;            
	    fp1[chan] = fidx;         
        }
        
	TimeMenuData.y_index = 0;
    }

    /*  Нажали кнопку "SET" */ 
    if(TimeMenuData.sel_index) {
      if(k == 2) {
        bok0[chan] = true;
        mv0[chan] = chan_set[chan].data.f_val; 
        k = 3; // следующая точка
        i = 0;
      } else if(k == 3) {
         bok1[chan] = true;
         mv1[chan] = chan_set[chan].data.f_val; 

         // другой канал
         if(bok0[chan] == true) {
           k = 0;
           i = 0;
         } else {
           k = 2;
           i = 0;
         }
      } else { // В любом другом положении выходим и записываем
            if (leaveMenu()) {        
                 bok0[chan] = bok1[chan] = false;
		 get_line_params(mv0[chan], fp0[chan], mv1[chan], fp1[chan], &chan_set[chan].gain, &chan_set[chan].shift);
		 eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[chan], SENS_ADDR(chan));
            }
      }
      
          TimeMenuData.sel_index = 0;
     }
    
    
    /* Показываем только если канал I2C */
    if (chan_set[chan].sens_type == I2C_SENSOR) {

	/* Милливольты канала */
	show_one_chan_acquis(115, y_pos[1], chan, chan_set[chan].data.f_val);

	/* Единицы измерений */
	if (unit == PPM_MODE) {
	    sprintf(str, "%s%s", StrUnits, "ppm");
	} else {		// проценты
	    sprintf(str, "%s%s", StrUnits, " % ");
	}
	show_units(5, y_pos[2], str);

	set_calibr_point(5, y_pos[3], 0, StrCalibrPoint, unit, fp0[chan], bok0[chan]);	// Первая точка
	set_calibr_point(5, y_pos[4], 1, StrCalibrPoint, unit, fp1[chan], bok1[chan]);	// Вторая точка
    }

    // Выделяем для изменений
    dog_XorBox(x_pos[k][i], y_pos[k + 1] - 2, x_pos[k][i] + cursor[k][i], y_pos[k + 1] + 14);

    /*  Нажимаем кнопку "Set" */
//    if (leaveMenu()) {
//	log_printf("Запись калибровочных данных\r\n");
//    }
}

//////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
/**
 * Дополнительные настройки каналов 
 */
static void menu_ranges_set(void)
{
    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };	/* Позиция по Y */
    static const u8 x_pos[] = { 58, 100 };	/* Позиция по X */
    static int k = 0;
    static int chan = 0;

    enterMenu(RANGES_SET_STATE);

    /* Заголовок "Доп. настройки" */
    show_caption(20, 90, ChannelSettingsMenuItems[2].psTitle);

    /* Номер канала если выбираем кнопку вверх/вниз */
    show_num_chan(10, y_pos[1], StrNumChan, chan);

    /* Порог минимум */
    show_thresh_diapaz(10, y_pos[2], StrThreshMin, chan_set[chan].thresh_min);

    /* Порог максимум */
    show_thresh_diapaz(10, y_pos[3], StrThreshMax, chan_set[chan].thresh_max);

    /* Диапазоны макс */
    show_thresh_diapaz(10, y_pos[4], StrDiapazMax, chan_set[chan].max_range);

    /* Меняем положение */
    if (TimeMenuData.x_index) {
	k += TimeMenuData.x_index;

	if (k > 3) {
	    k = 0;
	} else if (k < 0) {
	    k = 3;
	}
	TimeMenuData.x_index = 0;	/* Обязательно! */
    }

    if (k == 0) {
	dog_XorBox(x_pos[0], y_pos[k + 1], x_pos[0] + 10, y_pos[k + 1] + 14);	// Включен
    } else {
	dog_XorBox(x_pos[1], y_pos[k + 1], x_pos[1] + 40, y_pos[k + 1] + 14);	// Включен
    }

    /* Нажимаем кнопку вверх/вниз - коррекция chan */
    if (TimeMenuData.y_index) {

	/* Меняем номер канала */
	if (k == 0) {
	    chan += TimeMenuData.y_index;

	    if (chan > 7) {
		chan = 0;
	    } else if (chan < 0) {
		chan = 7;
	    }
	} else if (k == 1) {	/* Чувствит. минимум */
	    chan_set[chan].thresh_min += (TimeMenuData.y_index) / 10.0;
	    if (chan_set[chan].thresh_min > 10.0) {
		chan_set[chan].thresh_min = 0;
	    } else if (chan_set[chan].thresh_min < 0) {
		chan_set[chan].thresh_min = 10.0;
	    }
	} else if (k == 2) {	/* Чувствит. максимум */
	    chan_set[chan].thresh_max += TimeMenuData.y_index * 10;
	    if (chan_set[chan].thresh_max > 100.0) {
		chan_set[chan].thresh_max = 0;
	    } else if (chan_set[chan].thresh_max < 0) {
		chan_set[chan].thresh_max = 100.0;
	    }
	} else if (k == 3) {	/* Максим диапазон измерений */
	    chan_set[chan].max_range += TimeMenuData.y_index * 10;
	    if (chan_set[chan].max_range > 100.0) {
		chan_set[chan].max_range = 0;
	    } else if (chan_set[chan].max_range < 0) {
		chan_set[chan].max_range = 100.0;
	    }
	}

	TimeMenuData.y_index = 0;	/* Обязательно! */
    }

    /* Нажатие кнопки "Select" */
    if (leaveMenu()) {
	int f;
	for (int t = 0; t < NUM_ALL_CHAN; t++) {
	    f = eeprom_write_pack((EEPROM_VALUE_PACK *) & chan_set[t], SENS_ADDR(t));
	    log_printf("write eeprom: %d\n", f);
	}
    }
}


/**
 * Отображение меню настройки времени и даты
 * шрифт:  9x18b-позиции расчитаны из него
 */
static void menu_rtc_set(void)
{
    static const u8 x_pos[] = { 66, 9 * 3 + 66 - 1, 9 * 6 + 66 };
    static const u8 y_pos[] = { 90, 60, 40 };
    static int i = 0;		/* Столбцы */
    static int k = 0;		/* строки */

    /* Поменяли состояние */
    if (enterMenu(RTC_SET_STATE)) {
	TimeMenuData.rtc_time = get_sec_ticks();

	/* Выводим время */
	sec_to_tm(TimeMenuData.rtc_time, &TimeMenuData.t0);
	sec_to_str(TimeMenuData.rtc_time, TimeMenuData.str0);
    }

    show_caption(30, y_pos[0], CommonSettingsMenuItems[0].psTitle);	/* Заголовок */

    dog_DrawStrP(10, y_pos[1], TIME_FONT, DOG_PSTR(StrTime));
    dog_DrawStrP(x_pos[0], y_pos[1], TIME_FONT, DOG_PSTR(TimeMenuData.str0 + 9));

    TimeMenuData.str0[8] = 0;
    dog_DrawStrP(10, y_pos[2], TIME_FONT, DOG_PSTR(StrDate));
    dog_DrawStrP(x_pos[0], y_pos[2], TIME_FONT, DOG_PSTR(TimeMenuData.str0));

    /* Выделяем время или дату для коррекции */
    if (TimeMenuData.x_index) {
	/* Меняем положение по X */
	i += TimeMenuData.x_index;
	if (i < 0) {
	    i = 2;
	    k--;
	} else if (i > 2) {
	    i = 0;
	    k++;
	}

	/*  Меняем положение по Y */
	if (k < 0) {
	    k = 1;
	} else if (k > 1) {
	    k = 0;
	}

	TimeMenuData.x_index = 0;	/* Обязательно! */
    }

    /* Перемещаемся для коррекции */
    if (k == 0) {
	dog_XorBox(x_pos[i], y_pos[1] - 5, x_pos[i] + 18, y_pos[1] + 15);	// Время
    } else {
	dog_XorBox(x_pos[i], y_pos[2] - 5, x_pos[i] + 18, y_pos[2] + 15);	// Дата
    }

    /* Нажаты клавиши верх/вниз ? */
    if (TimeMenuData.y_index) {
	if (k == 0) {
	    /* Меняем Часы */
	    if (i == 0) {
		TimeMenuData.t0.tm_hour += TimeMenuData.y_index;
		if (TimeMenuData.t0.tm_hour > 23) {
		    TimeMenuData.t0.tm_hour = 0;
		} else if (TimeMenuData.t0.tm_hour < 0) {
		    TimeMenuData.t0.tm_hour = 23;
		}
		/* Меняем минуты */
	    } else if (i == 1) {
		TimeMenuData.t0.tm_min += TimeMenuData.y_index;
		if (TimeMenuData.t0.tm_min > 59) {
		    TimeMenuData.t0.tm_min = 0;
		} else if (TimeMenuData.t0.tm_min < 0) {
		    TimeMenuData.t0.tm_min = 59;
		}
		/* Меняем секунды */
	    } else if (i == 2) {
		TimeMenuData.t0.tm_sec += TimeMenuData.y_index;
		if (TimeMenuData.t0.tm_sec > 59) {
		    TimeMenuData.t0.tm_sec = 0;
		} else if (TimeMenuData.t0.tm_sec < 0) {
		    TimeMenuData.t0.tm_sec = 59;
		}
	    }
	} else {
	    /* здесь будет ошибка, если в месяцах с 30 или меньше днями будет 31
	     * чтобы этого избежать нужна проверка
	     */

	    /* Меняем День месяца */
	    if (i == 0) {
		TimeMenuData.t0.tm_mday += TimeMenuData.y_index;
		if (TimeMenuData.t0.tm_mday > 31) {
		    TimeMenuData.t0.tm_mday = 1;
		} else if (TimeMenuData.t0.tm_mday < 1) {
		    TimeMenuData.t0.tm_mday = 31;
		}

		/* Меняем месяц */
	    } else if (i == 1) {
		TimeMenuData.t0.tm_mon += TimeMenuData.y_index;
		if (TimeMenuData.t0.tm_mon > 11) {
		    TimeMenuData.t0.tm_mon = 0;
		} else if (TimeMenuData.t0.tm_mon < 0) {
		    TimeMenuData.t0.tm_mon = 11;
		}
		/* Меняем год 0 .. 99 */
	    } else if (i == 2) {
		int year = TimeMenuData.t0.tm_year - 100;
		year += TimeMenuData.y_index;
		if (year > 99) {
		    year = 0;
		} else if (year < 0) {
		    year = 99;
		}
		TimeMenuData.t0.tm_year = year + 100;
	    }
	}
	TimeMenuData.y_index = 0;	/* Обязательно! */
	TimeMenuData.rtc_time = tm_to_sec(&TimeMenuData.t0);
	sec_to_str(TimeMenuData.rtc_time, TimeMenuData.str0);	/* Выводим время */
    }

    /*  Нажатие кнопки "Select" */
    if (leaveMenu()) {
	set_time(TimeMenuData.rtc_time);
	i = 0;
    }
}


/**
 * Отображение главного меню. Y позиция для основного меню 
 * нужно написать, так как у нас считается Y-0  внизу
 */
static void main_menu_set(void)
{
    tMenuItem psMenuItem2;
    static const u8 y_pos[] = { 90, 70, 55, 40, 25 };
    u8 i;

    /* Выведем Заголовок */
    show_caption(30, y_pos[0], MainMenuShifter.ptrCurrentMenu->psTitle);

    /* Выведем пункты меню */
    for (u8 i = 0; i < MainMenuShifter.ptrCurrentMenu->nItems; i++) {
	psMenuItem2 = &(MainMenuShifter.ptrCurrentMenu->psItems[i]);
	dog_DrawStrP(15, y_pos[i + 1], MENU_FONT, DOG_PSTR(psMenuItem2->psTitle));
    }

    /* Выбран пункт */
    MainMenuShifter.ptrMenuItem = &(MainMenuShifter.ptrCurrentMenu->psItems[MainMenuShifter.MenuItemIndex]);

    /* Выделяем меню */
    i = y_pos[(MainMenuShifter.MenuItemIndex + 1) % 5];
    dog_XorBox(0, i - 2, 160, i + 12);
}

/**
 * Надпись - соединение по VCP - можно выйти только выдернув провод 
 */
static void menu_vcp_conn(void)
{
    dog_DrawStrP(30, 60, CAPT_FONT, DOG_PSTR(StrVcpConn0));
    dog_DrawStrP(30, 40, CAPT_FONT, DOG_PSTR(StrVcpConn1));

    /* Возврат в главное меню */
    if (!status_is_conn()) {
	menu_state = MAIN_MENU_STATE;	/* vvvv: вернуть на место!!! */
	TimeMenuData.sel_index = 0;	/* Обязательно! */
    }
}


/**
 * Перекодировать меню, так как нам нужен шрифт в формате koi8
 */
static void make_rus_menu(void)
{
#if defined _WIN_STR_H
    int i, num;

    /* Главное-Main menu */
    for (i = 0; i < sizeof(MainMenuItems) / sizeof(MainMenuItems[0]); i++) {
	win_to_koi8(MainMenuItems[i].psTitle);
    }
    win_to_koi8(MainMenu.psTitle);

    /* Установки */
    for (i = 0; i < sizeof(SettingsMenuItems) / sizeof(SettingsMenuItems[0]); i++) {
	win_to_koi8(SettingsMenuItems[i].psTitle);
    }
    win_to_koi8(SettingsMenu.psTitle);

    /* Измерения */
    for (i = 0; i < sizeof(MeasMenuItems) / sizeof(MeasMenuItems[0]); i++) {
	win_to_koi8(MeasMenuItems[i].psTitle);
    }
    win_to_koi8(MeasMenu.psTitle);

    /* Прочие */
    for (i = 0; i < sizeof(ZeroinMenuItems) / sizeof(ZeroinMenuItems[0]); i++) {
	win_to_koi8(ZeroinMenuItems[i].psTitle);
    }
    win_to_koi8(ZeroinMenu.psTitle);

    /* Calibr */
    for (i = 0; i < sizeof(CalibrMenuItems) / sizeof(CalibrMenuItems[0]); i++) {
	win_to_koi8(CalibrMenuItems[i].psTitle);
    }
    win_to_koi8(CalibrMenu.psTitle);

    /* Graphic */
    for (i = 0; i < sizeof(ChannelSettingsMenuItems) / sizeof(ChannelSettingsMenuItems[0]); i++) {
	win_to_koi8(ChannelSettingsMenuItems[i].psTitle);
    }
    win_to_koi8(ChannelSettingsMenu.psTitle);

    /* Settings */
    for (i = 0; i < sizeof(CommonSettingsMenuItems) / sizeof(CommonSettingsMenuItems[0]); i++) {
	win_to_koi8(CommonSettingsMenuItems[i].psTitle);
    }
    win_to_koi8(CommonSettingsMenu.psTitle);

    /* Время и дата */
    win_to_koi8(StrTime);
    win_to_koi8(StrDate);

    /* Вещества */
    num = sizeof(StrSubst) / sizeof(StrSubst[0]);
    for (i = 0; i < num; i++) {
	win_to_koi8(StrSubst[i]);
    }

    /* Основные настройки канала */
    win_to_koi8(StrNumChan);
    win_to_koi8(StrOnOff);
    win_to_koi8(StrOff);
    win_to_koi8(StrOn);
    win_to_koi8(StrFormula);
    win_to_koi8(StrUnits);
    win_to_koi8(StrDigits);
    win_to_koi8(StrAbout1);
    win_to_koi8(StrAbout0);
    win_to_koi8(StrMg);
    win_to_koi8(StrZeroShift);
    win_to_koi8(StrMultCoef);

    win_to_koi8(StrThreshMin);
    win_to_koi8(StrThreshMax);
    win_to_koi8(StrDiapazMax);

    win_to_koi8(StrVcpConn0);
    win_to_koi8(StrVcpConn1);

    win_to_koi8(StrZeroinChan0);
    win_to_koi8(StrZeroinChan1);
    win_to_koi8(StrZeroinChan2);

    win_to_koi8(StrCalibrPoint);
#endif
}

/**
 * Description: Dispatcher function
 */
static void ReadKey(void)
{
    char key;
    key = KeypadRead();
    switch (key) {

	/* Set/Select */
    case 'S':
	SelFunc();
	break;
	/* Up */
    case 'U':
	UpFunc();
	break;
	/* Down */
    case 'D':
	DownFunc();
	break;
	/* Right */
    case 'R':
	RightFunc();
	break;
	/* Left */
    case 'L':
	LeftFunc();
	break;
	/* Escape */
    case 'E':
	ReturnFunc();
	break;

    default:
	break;
    }
}


/**
 *  Ничего не делает
 */
void IdleFunc(void)
{
}


/**
 * Первоначальная инициализация
 */
static void menu_init(void)
{
    MainMenuShifter.ptrCurrentMenu = &MainMenu;
    MainMenuShifter.ptrPrevMenu[MainMenuShifter.MenuLevel] = MainMenuShifter.ptrCurrentMenu;
    MainMenuShifter.ptrMenuItem = MainMenuItems;
}

/**
 *  Ходим по веткам ВВЕРХ только в состоянии MAIN
 */
static void UpFunc(void)
{
    if (menu_state == MAIN_MENU_STATE) {

	/* Display current menu item as non-selected */
	MainMenuShifter.ptrMenuItem = &MainMenuShifter.ptrCurrentMenu->psItems[MainMenuShifter.MenuItemIndex];

	/* Determine new menu item (iteratively) */
	if (MainMenuShifter.MenuItemIndex > 0) {
	    MainMenuShifter.MenuItemIndex--;
	} else {
	    MainMenuShifter.MenuItemIndex = MainMenuShifter.ptrCurrentMenu->nItems - 1;
	}

	/* Display new menu item as selected */
	MainMenuShifter.ptrMenuItem = &MainMenuShifter.ptrCurrentMenu->psItems[MainMenuShifter.MenuItemIndex];
	log_printf("Up: MenuLevel %d, %s\r\n", MainMenuShifter.MenuLevel, koi8_to_win(MainMenuShifter.ptrMenuItem->psTitle));
    }

    TimeMenuData.y_index = 1;
}


/**
 *  Ходим по веткам Вниз только в состоянии MAIN
 */
static void DownFunc(void)
{
    if (menu_state == MAIN_MENU_STATE) {
	/* Display current menu item as non-selected */
	MainMenuShifter.ptrMenuItem = &MainMenuShifter.ptrCurrentMenu->psItems[MainMenuShifter.MenuItemIndex];
	/* Determine new menu item (iteratively) */
	if (MainMenuShifter.MenuItemIndex >= ((MainMenuShifter.ptrCurrentMenu->nItems) - 1)) {
	    MainMenuShifter.MenuItemIndex = 0;
	} else {
	    MainMenuShifter.MenuItemIndex++;
	}
	MainMenuShifter.ptrMenuItem = &(MainMenuShifter.ptrCurrentMenu->psItems[MainMenuShifter.MenuItemIndex]);
	/* Display new menu item as selected */
	log_printf("Down: MenuLevel %d, %s\r\n", MainMenuShifter.MenuLevel, koi8_to_win(MainMenuShifter.ptrMenuItem->psTitle));
    }
    TimeMenuData.y_index = -1;
}

/**
 * Кнопка "влево"
 */
static void LeftFunc(void)
{
    TimeMenuData.x_index = -1;
}


/**
 * Кнопка "вправо"
 */
static void RightFunc(void)
{
    TimeMenuData.x_index = 1;
}

/**
 *  Select
 */
static void SelFunc(void)
{
    /*  Ходим по веткам только в состоянии MAIN */
    if (menu_state == MAIN_MENU_STATE) {

	MainMenuShifter.ptrCurrentMenuItem = MainMenuShifter.ptrMenuItem;
	if (MainMenuShifter.ptrMenuItem->psSubMenu != NULL) {
	    MainMenuShifter.MenuItemIndex = 0;
	    MainMenuShifter.ptrCurrentMenu = MainMenuShifter.ptrMenuItem->psSubMenu;
	    MainMenuShifter.ptrMenuItem = &(MainMenuShifter.ptrCurrentMenu->psItems)[MainMenuShifter.MenuItemIndex];
	    MainMenuShifter.MenuLevel++;
	    MainMenuShifter.ptrPrevMenu[MainMenuShifter.MenuLevel] = MainMenuShifter.ptrCurrentMenu;
	}
	MainMenuShifter.ptrCurrentMenuItem->pfMenuFunc();
	log_printf("Sel: MenuLevel %d, %s\r\n", MainMenuShifter.MenuLevel, koi8_to_win(MainMenuShifter.ptrMenuItem->psTitle));
    } else {
	TimeMenuData.sel_index = 1;
	///vvvvv:        menu_state = MAIN_MENU_STATE; //vvvv: вернуть на место!
    }
}

/**
 *  Возврат на предыдущее
 */
static void ReturnFunc(void)
{
    /*  Ходим по веткам только в состоянии MAIN */
    if (menu_state == MAIN_MENU_STATE) {
	if (MainMenuShifter.MenuLevel == 0) {
	    MainMenuShifter.MenuLevel++;
	}

	MainMenuShifter.ptrCurrentMenu = MainMenuShifter.ptrPrevMenu[MainMenuShifter.MenuLevel - 1];
	MainMenuShifter.ptrMenuItem = &MainMenuShifter.ptrCurrentMenu->psItems[0];
	MainMenuShifter.MenuItemIndex = 0;
	MainMenuShifter.MenuLevel--;
    }
    /* Всегда - выход из режима ввода */
    menu_state = MAIN_MENU_STATE;
    log_printf("Ret: MenuLevel %d, %s\r\n", MainMenuShifter.MenuLevel, koi8_to_win(MainMenuShifter.ptrMenuItem->psTitle));
}

/**
 * Задача разбора нажатия кнопки и выбора меню
 */
void menu_create_task(void)
{
    xTaskHandle task = NULL;
    KeypadInit();		/* Клавиатура. Обрабтка нажатия происходит по таймеру SysTick */
    menu_init();		/* Инициализация меню */
    make_rus_menu();		/* Перекодировать меню */

#if 0
    /* Предварительные настройки каналов */
    for (int i = 0; i < 8; i++) {
	chan_set[i].sens_type = I2C_SENSOR;
	chan_set[i].type_units = PPM_MODE;
	chan_set[i].num_digits = INT_MODE;
	strcpy(chan_set[i].formula, StrSubst[i]);	/* не более 31 */
    }
#endif

    /* Создаем задачу хождения по меню */
    xTaskCreate(vMenuTask, (s8 *) "MenuTask", MENU_TASK_STACK_SIZE, NULL, MENU_TASK_PRIORITY, &task);
    if (task == NULL) {
	log_printf("ERROR: Create MenuTask\r\n");
	configASSERT(task);
    }
    log_printf("SUCCESS: Create MenuTask\r\n");
}

/**
 * Задача обработки меню
 */
static void vMenuTask(void *v)
{
    dog_Init(0);

    for (;;) {
	int t0 = get_sec_ticks();

	/* Читаем кнопку если нет соединения (блокирование во время работы с VCP) */
	if (!status_is_conn()) {
	    ReadKey();
	}

	dog_StartPage();
	do {
	    if (menu_state == MAIN_MENU_STATE) {
		main_menu_set();	/* Меню и выборы подменю */
	    } else if (menu_state == RTC_SET_STATE) {
		menu_rtc_set();	/* Ввод данных */
	    } else if (menu_state == MAIN_SET_CHANNEL_STATE) {
		main_chan_set();	/* Главные установки каналов */
	    } else if (menu_state == ADDIT_SET_CHANNEL_STATE) {
		addit_chan_set();
	    } else if (menu_state == MENU_PUMP_SET_STATE) {
		menu_pump_set();
	    } else if (menu_state == MENU_VOLUME_SET_STATE) {
		menu_volume_set();
	    } else if (menu_state == MENU_ABOUT_STATE) {
		menu_about_set();
	    } else if (menu_state == MENU_CHAN_COEF_STATE) {
		chan_coef_set();
	    } else if (menu_state == RANGES_SET_STATE) {
		menu_ranges_set();
	    } else if (menu_state == MENU_ACQUIS_STATE) {
		menu_acquis_set();
	    } else if (menu_state == MENU_ZERO_ALL_CHAN_STATE) {
		zero_all_chan();
	    } else if (menu_state == MENU_VCP_CONN_STATE) {
		menu_vcp_conn();
	    } else if (menu_state == MENU_CALIBR_STATE) {
		calibr_menu_set();
	    }

	    /* Внизу показываем время, номер прибора и еще какую нибудь инфу */
	    dog_DrawStrP(1, 10, MENU_FONT, DOG_PSTR("------------------------------------------------------------"));
	    show_clock(t0);
	    show_bat(5);
	} while (dog_NextPage());

	dog_Delay(50);
    }
}

/** 
 * Для сокращения функций
 * Вход
 */
static bool enterMenu(menu_state_en newState)
{
    bool res = false;
    if (menu_state == MAIN_MENU_STATE) {
	menu_state = newState;
	TimeMenuData.x_index = 0;
	TimeMenuData.y_index = 0;
	TimeMenuData.sel_index = 0;
	res = true;
    }
    return res;
}


/**
 *  Выход
 */
static bool leaveMenu(void)
{
    bool res = false;
    if (TimeMenuData.sel_index) {
	menu_state = MAIN_MENU_STATE;	/* vvvv: вернуть на место!!! */
	TimeMenuData.sel_index = 0;	/* Обязательно! */
	res = true;
    }
    return res;
}

/**
 * Переключить на работу с VCP
 */
void set_vcp_menu(void)
{
    menu_state = MENU_VCP_CONN_STATE;
}
