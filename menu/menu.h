#ifndef _MENU_H
#define _MENU_H

#include "dogm128.h"
#include "keyb.h"
#include "main.h"

#define 	MAX_MENU_STRING         32
#define		MAX_MENU_LEVELS 	4
#define		countof(a) 		(sizeof(a) / sizeof(*(a)))

/* Фонты для отображения меню. Используем только их */
#define 	MENU_FONT	font_7x14	/* Выберем шрифт для меню */
#define 	CAPT_FONT  	font_7x14b	/* Выберем шрифт для заголовка */
#define 	SYMB_FONT 	font_6x10	/* Для подстрочных символов */
#define 	INFO_FONT  	font_4x6	/* для часов */
#define 	TIME_FONT 	font_9x18b	/* Для ввода времение и даты */


/* Состояние меню и набора кнопок */
typedef enum {
    MAIN_MENU_STATE,
    RTC_SET_STATE,		/* Установка даты-времени */
    MAIN_SET_CHANNEL_STATE,	/* Главные установки каналов */
    ADDIT_SET_CHANNEL_STATE,	/* Дополнительные установки каналов */
    RANGES_SET_STATE,		/* Диапазоны */
    MENU_PUMP_SET_STATE,	/* Насос */
    MENU_VOLUME_SET_STATE,	/* Звук */
    MENU_ABOUT_STATE,		/* О приборе */
    MENU_CHAN_COEF_STATE,	/* Коэфициенты */
    MENU_ACQUIS_STATE,		/* Измерения */
    MENU_ZERO_ALL_CHAN_STATE,	/* Сброс всех каналов */
    MENU_VCP_CONN_STATE,	/* Соединение по VCP */
    MENU_CALIBR_STATE,		/* Калибровка */
} menu_state_en;


/* Тип вещества */
typedef enum {
    NH3,			//Аммиак 
    H2,				// Водород
    SF6,			//Гекс. серы 
    NO2,			//Диокс. азота 
    SO2,			// Диокс. серы 
    O2,				// Кислор.
    RSH,			//Меркапт.
    O3,				//Озон 
    NO,				//Окс. азота 
    H2S,			//Сероводород 
    HCN,			//Син. кислота 
    CO,				//Окс. углерода 
    CO2,			//Диок. углер. 
    H2CO2,			//Формальд. 
    HF,				//Фтор. водород 
    CHC1F2,			//Хладон 
    Cl2,			//Хлор 
    HCl,			//Хлороводород 
    C2H5OH,			//Этанол 
    C6H6,			//Бензол 
    C4H10,			//Бутан 
    C6H14,			//Гексан 
    CH4,			//Метан 
    C3H8,			//Пропан 
    C2H4,			//Этилен 
} gase_type_en;

void IdleFunc(void);
void menu_create_task (void);
void set_vcp_menu(void);

#endif				/* menu.h */
