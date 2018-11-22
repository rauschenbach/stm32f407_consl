/***************************************************************************
 *                  Драйвер матричной клавиатуры 2x4 
 *                                v 1.0
 *                  Copyright (c) Кизим Игорь aka Igoryosha
 *                        Website : lobotryasy.net 
 *
 *		    i.rau cделал это для STM32	
 *			boxeur@mail.ru
 ***************************************************************************/
#include "keyb.h"


#define MAX_VALUE   20		/* Антидребезговая выдержка времени */

/* Подключение строк (выход микроконтроллера) */
#define ROW_PORT            GPIOB	/* Порт, подключённый к строкам (к выходу) */
#define ROW_GPIO_PIN1       GPIO_Pin_4  /* линия 1  */
#define ROW_GPIO_PIN2       GPIO_Pin_5  /* линия 2  */
#define ROW_GPIO_CLK        RCC_AHB1Periph_GPIOB

#define NUM_ROW             2	/* Количество строк клавиатуры */


/* Подключение столбцов (вход микроконтроллера) */
#define COLUMN_PORT         GPIOD
#define COLUMN_GPIO_PIN1    GPIO_Pin_0     // PD.00 - A
#define COLUMN_GPIO_PIN2    GPIO_Pin_1     // PD.01 - B
#define COLUMN_GPIO_PIN3    GPIO_Pin_2     // PD.02 - C
#define COLUMN_GPIO_PIN4    GPIO_Pin_3     // PD.03 - D
#define COLUMN_GPIO_CLK     RCC_AHB1Periph_GPIOD  
#define FIRST_BIT_COLUMN     0

#define NUM_COLUMN           4	/* Количество столбцов клавиатуры */


const uint16_t KEYPAD_ROW[] = { ROW_GPIO_PIN1, ROW_GPIO_PIN2 };
const uint16_t KEYPAD_COLUMN[] = { COLUMN_GPIO_PIN1, COLUMN_GPIO_PIN2, COLUMN_GPIO_PIN3, COLUMN_GPIO_PIN4 };

/* антидребезг */
static struct {
    volatile char ScanKey;	/* Переменная для сохранения кода нажатой кнопки */
    char lastkey;
    char counter;
} bounce_key;

static char keykodes[2][4] = {
    {'L', 'D', 'S', 'R'},
    {'E', '2', 'U', '1'},
};

/*
static char* keykodes[2][4] = {
    {'Left', 'Down', 'Set', 'Right'},
    {'Esc', 'F2', 'Up', 'F1'},
};
*/

/* Проверка нажатия хотя бы одной кнопки */
/*static*/ bool KeypadCheck(void);



/**
 * Инициализация клавиатуры 2x4
 * Нам нужно 2 порта на выход и 4 на вход
 */
void KeypadInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Тактирование разрешить  */
    RCC_AHB1PeriphClockCmd(ROW_GPIO_CLK | COLUMN_GPIO_CLK, ENABLE);

    /* Формирование 2-х портов ROW PB.04..PB.05 - на выход */
    GPIO_InitStructure.GPIO_Pin = ROW_GPIO_PIN1 | ROW_GPIO_PIN2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ROW_PORT, &GPIO_InitStructure);


    /* Формирование 4-х портов Column на вход PB.04...PB.06, +подключение подтягивающих резисторов */
    GPIO_InitStructure.GPIO_Pin = COLUMN_GPIO_PIN1 | COLUMN_GPIO_PIN2 | COLUMN_GPIO_PIN3 | COLUMN_GPIO_PIN4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = /*GPIO_PuPd_NOPULL | */ GPIO_PuPd_UP;
    GPIO_Init(COLUMN_PORT, &GPIO_InitStructure);

    bounce_key.lastkey = 0;
    bounce_key.counter = 0;
}

/**
 * Проверка нажатия хотя бы одной кнопки
 */
/*static*/ bool KeypadCheck(void)
{
    u16 temp;

    /* Установил на всех выходах клавиатуры (на выходах строк) '0' */
    GPIO_ResetBits(ROW_PORT, ROW_GPIO_PIN1);
    GPIO_ResetBits(ROW_PORT, ROW_GPIO_PIN2);

    /* Проверяю нажатие (0), если да - возвращаю true */
    for (int i = 0; i < NUM_COLUMN; i++) {
	temp = GPIO_ReadInputDataBit(COLUMN_PORT, KEYPAD_COLUMN[i]);
	if (!temp) {
	    return true;
	}
    }

    return false;
}

/**
 *   Получение кода нажатой кнопки
 */
char KeypadRead(void)
{
    char key = bounce_key.ScanKey;
    bounce_key.ScanKey = 0;
    return key;
}

/**
 * Функция сканирования клавиатуры (запускается в прерывании таймера)
 */
void KeypadScan(void)
{
    char row, column, temp;
    char key = 0;

    /* Если ни одна кнопка не нажата - досвидос! */
    if (KeypadCheck() == 0) {
	bounce_key.counter = 0;
	return;
    }

    /* Перебираю строки клавиатуры */
    for (row = 0; row < NUM_ROW; row++) {

	/* Установил на обеих выходах строк '1' */
	GPIO_SetBits(ROW_PORT, ROW_GPIO_PIN1);
	GPIO_SetBits(ROW_PORT, ROW_GPIO_PIN2);

	/* Устанавливаю на очередной строке клавиатуры '0' */
	GPIO_ResetBits(ROW_PORT, KEYPAD_ROW[row]);
	asm(" nop ");
	/* Сохраняю значения всех 4-х входов (столбцов) */
	temp = GPIO_ReadInputData(COLUMN_PORT);

	/* Опрашиваю столбцы клавиатуры и получаю код кнопки */
	for (column = 0; column < NUM_COLUMN; column++) {
	    /* Если подключенные порты идут последовательно. Иначе переделать этот код */
	    if ((temp & (1 << (column + FIRST_BIT_COLUMN))) == 0) {
		key = keykodes[row][column];
	    }
	}
    }

    if (key != bounce_key.lastkey) {
	bounce_key.lastkey = key;
	bounce_key.counter = 0;
    } else {
	if (bounce_key.counter == MAX_VALUE) {
	    bounce_key.counter = MAX_VALUE + 10;
	    bounce_key.ScanKey = key;
	    return;
	}
	if (bounce_key.counter < MAX_VALUE)
	    bounce_key.counter++;
    }
}
