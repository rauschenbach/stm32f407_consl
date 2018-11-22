/***************************************************************************
 *                  ������� ��������� ���������� 2x4 
 *                                v 1.0
 *                  Copyright (c) ����� ����� aka Igoryosha
 *                        Website : lobotryasy.net 
 *
 *		    i.rau c����� ��� ��� STM32	
 *			boxeur@mail.ru
 ***************************************************************************/
#include "keyb.h"


#define MAX_VALUE   20		/* ��������������� �������� ������� */

/* ����������� ����� (����� ����������������) */
#define ROW_PORT            GPIOB	/* ����, ������������ � ������� (� ������) */
#define ROW_GPIO_PIN1       GPIO_Pin_4  /* ����� 1  */
#define ROW_GPIO_PIN2       GPIO_Pin_5  /* ����� 2  */
#define ROW_GPIO_CLK        RCC_AHB1Periph_GPIOB

#define NUM_ROW             2	/* ���������� ����� ���������� */


/* ����������� �������� (���� ����������������) */
#define COLUMN_PORT         GPIOD
#define COLUMN_GPIO_PIN1    GPIO_Pin_0     // PD.00 - A
#define COLUMN_GPIO_PIN2    GPIO_Pin_1     // PD.01 - B
#define COLUMN_GPIO_PIN3    GPIO_Pin_2     // PD.02 - C
#define COLUMN_GPIO_PIN4    GPIO_Pin_3     // PD.03 - D
#define COLUMN_GPIO_CLK     RCC_AHB1Periph_GPIOD  
#define FIRST_BIT_COLUMN     0

#define NUM_COLUMN           4	/* ���������� �������� ���������� */


const uint16_t KEYPAD_ROW[] = { ROW_GPIO_PIN1, ROW_GPIO_PIN2 };
const uint16_t KEYPAD_COLUMN[] = { COLUMN_GPIO_PIN1, COLUMN_GPIO_PIN2, COLUMN_GPIO_PIN3, COLUMN_GPIO_PIN4 };

/* ����������� */
static struct {
    volatile char ScanKey;	/* ���������� ��� ���������� ���� ������� ������ */
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

/* �������� ������� ���� �� ����� ������ */
/*static*/ bool KeypadCheck(void);



/**
 * ������������� ���������� 2x4
 * ��� ����� 2 ����� �� ����� � 4 �� ����
 */
void KeypadInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ������������ ���������  */
    RCC_AHB1PeriphClockCmd(ROW_GPIO_CLK | COLUMN_GPIO_CLK, ENABLE);

    /* ������������ 2-� ������ ROW PB.04..PB.05 - �� ����� */
    GPIO_InitStructure.GPIO_Pin = ROW_GPIO_PIN1 | ROW_GPIO_PIN2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ROW_PORT, &GPIO_InitStructure);


    /* ������������ 4-� ������ Column �� ���� PB.04...PB.06, +����������� ������������� ���������� */
    GPIO_InitStructure.GPIO_Pin = COLUMN_GPIO_PIN1 | COLUMN_GPIO_PIN2 | COLUMN_GPIO_PIN3 | COLUMN_GPIO_PIN4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = /*GPIO_PuPd_NOPULL | */ GPIO_PuPd_UP;
    GPIO_Init(COLUMN_PORT, &GPIO_InitStructure);

    bounce_key.lastkey = 0;
    bounce_key.counter = 0;
}

/**
 * �������� ������� ���� �� ����� ������
 */
/*static*/ bool KeypadCheck(void)
{
    u16 temp;

    /* ��������� �� ���� ������� ���������� (�� ������� �����) '0' */
    GPIO_ResetBits(ROW_PORT, ROW_GPIO_PIN1);
    GPIO_ResetBits(ROW_PORT, ROW_GPIO_PIN2);

    /* �������� ������� (0), ���� �� - ��������� true */
    for (int i = 0; i < NUM_COLUMN; i++) {
	temp = GPIO_ReadInputDataBit(COLUMN_PORT, KEYPAD_COLUMN[i]);
	if (!temp) {
	    return true;
	}
    }

    return false;
}

/**
 *   ��������� ���� ������� ������
 */
char KeypadRead(void)
{
    char key = bounce_key.ScanKey;
    bounce_key.ScanKey = 0;
    return key;
}

/**
 * ������� ������������ ���������� (����������� � ���������� �������)
 */
void KeypadScan(void)
{
    char row, column, temp;
    char key = 0;

    /* ���� �� ���� ������ �� ������ - ��������! */
    if (KeypadCheck() == 0) {
	bounce_key.counter = 0;
	return;
    }

    /* ��������� ������ ���������� */
    for (row = 0; row < NUM_ROW; row++) {

	/* ��������� �� ����� ������� ����� '1' */
	GPIO_SetBits(ROW_PORT, ROW_GPIO_PIN1);
	GPIO_SetBits(ROW_PORT, ROW_GPIO_PIN2);

	/* ������������ �� ��������� ������ ���������� '0' */
	GPIO_ResetBits(ROW_PORT, KEYPAD_ROW[row]);
	asm(" nop ");
	/* �������� �������� ���� 4-� ������ (��������) */
	temp = GPIO_ReadInputData(COLUMN_PORT);

	/* ��������� ������� ���������� � ������� ��� ������ */
	for (column = 0; column < NUM_COLUMN; column++) {
	    /* ���� ������������ ����� ���� ���������������. ����� ���������� ���� ��� */
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
