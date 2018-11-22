#include <string.h>
#include "main.h"
#include "lmp91k.h"
#include "sensor.h"
#include "status.h"
#include "eeprom.h"
#include "i2c1.h"
#include "adc.h"

#define		LMP91K_I2C_ADDR		0x90
#define		LMP91K_GPIO_PORT      	GPIOD	/* GPIOD */
#define 	LMP91K_GPIO_CLK       	RCC_AHB1Periph_GPIOD


/* ���� �������� ��� ������ ����������� ������� PORTD */
static uint32_t LMP91K_PIN[] = {
    GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9,
    /* GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, // -- ��� �� ��������!!! */
};

static u8 reg[] = { STATUS_REG, LOCK_REG, TIACN_REG, REFCN_REG, MODECN_REG };
static const char *reg_name[] = {
    "STATUS_REG",
    "LOCK_REG",
    "TIACN_REG",
    "REFCN_REG",
    "MODECN_REG"
};

/**
 * ������������� ���� ������� ���
 * �������� EEPROM
 */
void lmp91k_port_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ������������ */
    RCC_AHB1PeriphClockCmd(LMP91K_GPIO_CLK, ENABLE);

    /* Configure PINS MENB - 4 ����� */
    GPIO_InitStructure.GPIO_Pin = LMP91K_PIN[0] | LMP91K_PIN[1] | LMP91K_PIN[2] | LMP91K_PIN[3];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(LMP91K_GPIO_PORT, &GPIO_InitStructure);

    /* ������������� I2C */
    ub_i2c1_init();
}


/**
 * �������� ���������� ������. ����� ������� � ����� �����
 * ���������� �� ��������� ����� �����
 */
void lmp91k_on(uint8_t num)
{
    volatile int i, j;

    /* ������� ��� �������� */
    for (i = 0; i < NUM_LMP91K_SENS; i++) {
	if (i == num) {
	    GPIO_ResetBits(LMP91K_GPIO_PORT, LMP91K_PIN[num]);	/* �������� ���������� ������ */
	} else {
	    GPIO_SetBits(LMP91K_GPIO_PORT, LMP91K_PIN[i]);
	}
	/* �� ������ �� taskDelay �� delay_ms ��� ��� �� ���� � ����� ��������� ��������� */
	for (j = 0; j < 10000; j++);
    }
}


/**
 * ��������� ����������� ����������� ������� �� I2C
 * ���� �������� - return 0, ��� = -1
 */
int lmp91k_check(u8 * data)
{
    int res = -1;
    int i, k;

    /* ������ ������ � registra ������� */
    log_printf("Read data from regs:\n");
    for (i = 0; i < 5; i++) {
	k = reg[i];
	res = ub_i2c1_read_byte(LMP91K_I2C_ADDR, k);
	if (res < 0) {
	    log_printf("error read i2c\n");
	    break;
	}
	data[k] = res;
	log_printf("%s (reg_%02X): %02X\n", reg_name[i], k, res);
	res = 0;
    }
    log_printf("\n");
    return res;
}


/**
 * ���������������� 5-�� (�� 20 ���������) ��������� ������� �� LMP91K
 * ������ ���� �������������� ������!
 */
int lmp91k_config(u8 * data)
{
    int res = -1;
    u8 byte, lock;

    do {
	if (data == NULL) {
	    break;
	}
	log_printf("write data to regs:\n");

	/* ��������� ������� LOCK ������ ��� ������ � ������ �������� 
	 * ��� ��� ������ ����������, ��������� 0 - ������ �����������
	 * ������ � ������ �������� */
	lock = 0;
	res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, LOCK_REG, lock);
	if (res < 0) {
	    break;
	}
	log_printf("LOCK_REG: %02X\n", lock);

	/* ������ ������ */
	res = ub_i2c1_read_byte(LMP91K_I2C_ADDR, STATUS_REG);
	if (res != 1) {
	    log_printf("error: STATUS_REG isn't READY (%02X)\n", res);
	    break;
	}


	/* ����������� ������� TIACN */
	byte = data[TIACN_REG];
	if (byte & 0xC0) {
	    byte = 0;		// ��� �� �������� � EEPROM!
	    byte |= 0x03;
	}
	res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, TIACN_REG, byte);
	if (res < 0) {
	    break;
	}
	log_printf("TIACN_REG: %02X\n", byte);

	/* ������� REFCN */
	byte = data[REFCN_REG];
	res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, REFCN_REG, byte);
	if (res < 0) {
	    break;
	}
	log_printf("REFCN_REG: %02X\n", byte);

	/* ������� MODECN */
	byte = data[MODECN_REG];
	if (byte == 1) {
	    byte |= 0x03;	/* ���� ��� ground reference - ������ ���� ��� */
	}
	res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, MODECN_REG, byte);
	if (res < 0) {
	    break;
	}
	log_printf("MODECN_REG: %02X\n", byte);

	lock = 1;
	res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, LOCK_REG, lock);
	if (res < 0) {
	    break;
	}
	log_printf("LOCK_REG: %02X\n", lock);
	log_printf("--------------------------------\n");

	/* ������ ��� �������� ��� �������� */
#if 0
	log_printf("LOCK_REG: %02X\n", ub_i2c1_read_byte(LMP91K_I2C_ADDR, LOCK_REG));
	log_printf("TIACN_REG: %02X\n", ub_i2c1_read_byte(LMP91K_I2C_ADDR, TIACN_REG));
	log_printf("REFCN_REG: %02X\n", ub_i2c1_read_byte(LMP91K_I2C_ADDR, REFCN_REG));
	log_printf("MODECN_REG: %02X\n", ub_i2c1_read_byte(LMP91K_I2C_ADDR, MODECN_REG));
#endif
	res = 0;
    } while (0);
    return res;
}


/**
 * ��������� ������ � �������� ���
 * �������������� ����� ������� ������
 */
int lmp91k_stop(void)
{
    int res = -1;

    res = ub_i2c1_write_byte(LMP91K_I2C_ADDR, MODECN_REG, 0);
    if (res == 0) {
	//log_printf("Deep sleep...\n");
	res = 0;
    }
    return res;
}

/**
 * ������ ������������� ��������� Rtia
 */
int lmp91k_get_tia_gain(u8 reg)
{
    int res = -1;		/* ������� ������������� ���������� */
    reg >>= 2;
    reg &= 0x07;

    switch (reg) {
    case 1:
	res = 2750;
	break;

    case 2:
	res = 3500;
	break;

    case 3:
	res = 7000;
	break;

    case 4:
	res = 14000;
	break;

    case 5:
	res = 35000;
	break;

    case 6:
	res = 120000;
	break;

    case 7:
	res = 350000;
	break;

    default:
	break;
    }
    return res;
}

/* ������ ���� ��������*/
int lmp91k_get_sign(u8 reg)
{
    if (reg & 0x10) {
	return 1;
    } else {
	return -1;
    }
}


/**
 * ������ ���������� �������� � ������������!
 * Vref ������� ��� 2500 mv
 */
int lmp91k_get_bias(u8 reg)
{
    int res = 0;
    u8 bias = reg & 0x0F;

    /* ������� �������� */
    switch (bias) {
    case 1:
	res = 1;
	break;

    case 2:
	res = 2;
	break;

    case 3:
	res = 4;
	break;

    case 4:
	res = 6;
	break;

    case 5:
	res = 8;
	break;

    case 6:
	res = 10;
	break;

    case 7:
	res = 12;
	break;

    case 8:
	res = 14;
	break;

    case 9:
	res = 16;
	break;

    case 10:
	res = 18;
	break;

    case 11:
	res = 20;
	break;

    case 12:
	res = 22;
	break;

    case 13:
	res = 24;
	break;

    default:
	break;
    }
    return res;
}

/**
 * ���������� ���� - ������ � ������������!
 */
int lmp91k_get_int_zero(u8 reg)
{
    int res = 0;
    reg = (reg >> 5) & 0x03;

    switch (reg) {
    case 0:
	res = 20;
	break;

    case 1:
	res = 50;
	break;

    case 2:
	res = 67;
	break;

    default:
	break;
    }

    return res;
}

/**
 * ����������� �������
 */
int lmp91k_get_rload(u8 reg)
{
    reg = (reg & 0x03);
    int res = 100;

    switch (reg) {
    case 0:
	res = 10;
	break;

    case 1:
	res = 33;
	break;

    case 2:
	res = 50;
	break;

    default:
	break;
    }
    return res;
}
