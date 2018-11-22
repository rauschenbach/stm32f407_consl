/*****************************************************************************
 * File     : stm32_ub_i2c1.c
 * Datum    : 16.12.2013
 * Version  : 1.1
 * Autor    : UB
 * EMail    : mc-4u(@)t-online.de
 * Web      : www.mikrocontroller-4u.de
 * CPU      : STM32F4
 * IDE      : CooCox CoIDE 1.7.4
 * GCC      : 4.7 2012q4
 * Module   : GPIO, I2C 
 * Funktion : I2C-LoLevel-Funktionen (I2C-1)
 * Hinweis  : m�gliche Pinbelegungen
 *            I2C1 : SCL :[PB6, PB8] 
 *                   SDA :[PB7, PB9]
 *            externe PullUp-Widerst�nde an SCL+SDA notwendig
 *****************************************************************************/


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------

#include "i2c1.h"


u8 I2C1_DATA[I2C1_MULTIBYTE_ANZ];	// ������

//--------------------------------------------------------------
// ���������� �������
//--------------------------------------------------------------
static s16 P_I2C1_timeout(s16 wert);
static void P_I2C1_InitI2C(void);


//--------------------------------------------------------------
// ����������� I2C1
//--------------------------------------------------------------
static I2C1_DEV_t I2C1DEV = {
// PORT , PIN      , Clock              , Source 
    {GPIOB, GPIO_Pin_6, RCC_AHB1Periph_GPIOB, GPIO_PinSource6},	// SCL � PB6
    {GPIOB, GPIO_Pin_9, RCC_AHB1Periph_GPIOB, GPIO_PinSource9},	// SDA � PB9
};



//--------------------------------------------------------------
// ������������� I2C1
//-------------------------------------------------------------- 
void ub_i2c1_init(void)
{
    static u8 init_ok = 0;
    GPIO_InitTypeDef GPIO_InitStructure;

    // �������������, ����������� ���� ���
    if (init_ok != 0) {
	return;
    }
    // I2C-������������, ���������
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // ��������� ������������ �����
    RCC_AHB1PeriphClockCmd(I2C1DEV.SCL.CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(I2C1DEV.SDA.CLK, ENABLE);

    // I2C �������
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    // ��������� �������������� ������� ������ �����/������ ��� I2C
    GPIO_PinAFConfig(I2C1DEV.SCL.PORT, I2C1DEV.SCL.SOURCE, GPIO_AF_I2C1);
    GPIO_PinAFConfig(I2C1DEV.SDA.PORT, I2C1DEV.SDA.SOURCE, GPIO_AF_I2C1);

    // I2C ��� �������������� ������� � OpenDrain  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // SCL-�����
    GPIO_InitStructure.GPIO_Pin = I2C1DEV.SCL.PIN;
    GPIO_Init(I2C1DEV.SCL.PORT, &GPIO_InitStructure);
    // SDA-�����
    GPIO_InitStructure.GPIO_Pin = I2C1DEV.SDA.PIN;
    GPIO_Init(I2C1DEV.SDA.PORT, &GPIO_InitStructure);

    // I2C �������������
    P_I2C1_InitI2C();

    // ��������� ����� �������������
    init_ok = 1;
}

/**
 * ������ � ������ ��������
 */
s16 ub_i2c1_read_byte(u8 slave_adr, u8 adr)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-6));
    }

    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-7));
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
    ret_wert = (s16) (I2C_ReceiveData(I2C1));
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return (ret_wert);
}


/**
 * ������ ��� ������ ��������
 */
s16 ub_i2c1_simple_read_byte(u8 slave_adr)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }


    I2C1->SR2;
    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }


    I2C_SendData(I2C1, 0);///vvvv:
    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

#if 0
    I2C_GenerateSTART(I2C1, ENABLE);
    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }


    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);
    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-6));
    }


    I2C1->SR2;
    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-7));
    }
#endif

//    I2C_GenerateSTOP(I2C1, ENABLE);
    ret_wert = (s16) (I2C_ReceiveData(I2C1));
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return (ret_wert);
}


//--------------------------------------------------------------
// �������� ������ I2C � ������ Slave
// slave_adr => I2C ����� Slave �� ���������
// ADR => ������ ������
// �������� => ������ �����
//
// ������������ �������� :
//    0   , �����
//  < 0   , ������
//--------------------------------------------------------------
s16 ub_i2c1_write_byte(u8 slave_adr, u8 adr, u8 wert)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    // ���������������� ������
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // �������� Slave ������
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // ������ ������ �����
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // �������� ������
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // �������� ������
    I2C_SendData(I2C1, wert);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // ��������� ������������������
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// ��� � �������

    return (ret_wert);
}



s16 ub_i2c1_simple_write_byte(u8 slave_adr, u8 wert)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    // ���������������� ������
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // �������� Slave ������
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // ������ ������ �����
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // �������� ������
    I2C_SendData(I2C1, wert);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // ��������� ������������������
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// ��� � �������

    return (ret_wert);
}


//--------------------------------------------------------------
// ����� I2C � ������ Slave
// slave_adr => I2C ����� Slave �� ���������
// adr=> ��������� ����� ��������
// cnt => ���������� ���� ��� ������
// ������, ������� ���� ���������, ������������ � "I2C1_DATA"
//
// ������������ �������� :
//    0   , �����
//  < 0   , ������
//--------------------------------------------------------------
s16 ub_i2c1_read_many_bytes(u8 slave_adr, u8 adr, u8 cnt)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;
    u8 wert, n;

    if (cnt == 0)
	return (-8);
    if (cnt > I2C1_MULTIBYTE_ANZ)
	return (-9);

    // ������ ������������������
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    if (cnt == 1) {
	// ACK ���������
	I2C_AcknowledgeConfig(I2C1, DISABLE);
    } else {
	// ACK ��������
	I2C_AcknowledgeConfig(I2C1, ENABLE);
    }

    // �������� ������ Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // ��������� ����� ADDR
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // �������� ����
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // ������ ������������������
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // �������� ������ Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-6));
    }

    // ��������� ����� ADDR
    I2C1->SR2;

    // ������ ��� ������
    for (n = 0; n < cnt; n++) {

	if ((n + 1) >= cnt) {
	    // ACK ���������
	    I2C_AcknowledgeConfig(I2C1, DISABLE);
	    // ��������� ������������������
	    I2C_GenerateSTOP(I2C1, ENABLE);
	}

	timeout = I2C1_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
	    if (timeout != 0)
		timeout--;
	    else
		return (P_I2C1_timeout(-7));
	}

	// ������ ������
	wert = I2C_ReceiveData(I2C1);

	// �������� ������ � �������
	I2C1_DATA[n] = wert;
    }

    // ACK ��������
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    ret_wert = 0;		// ��� � �������

    return (ret_wert);
}


//--------------------------------------------------------------
// ����� I2C � ������ Slave
// slave_adr => I2C ����� Slave �� ���������
// adr=> ��������� ����� ��������
// cnt => ���������� ���� ��� ������
// ������, ������� ���� ���������, ������������ � "I2C1_DATA"
//
// ������������ �������� :
//    0   , �����
//  < 0   , ������
//--------------------------------------------------------------
s16 ub_i2c1_write_many_bytes(u8 slave_adr, u8 adr, u8 cnt)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;
    u8 wert, n;

    if (cnt == 0)
	return (-6);
    if (cnt > I2C1_MULTIBYTE_ANZ)
	return (-7);

    // ������ ������������������
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // �������� ������ Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // ��������� ����� ADDR
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // ����� ���������
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // ��������� ��� ������
    for (n = 0; n < cnt; n++) {
	// ������ ������ �� �������
	wert = I2C1_DATA[n];

	// ������ ����������
	I2C_SendData(I2C1, wert);

	timeout = I2C1_TIMEOUT;
	while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	       || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	    if (timeout != 0)
		timeout--;
	    else
		return (P_I2C1_timeout(-5));
	}
    }

    //  ��������� ������������������
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// ��� � �������

    return (ret_wert);
}

//--------------------------------------------------------------
// �������� �����
//--------------------------------------------------------------
void ub_i2c1_delay(volatile u32 nCount)
{
    while (nCount--) {
    }
}


//--------------------------------------------------------------
// ���������� �������
// ������������� ���������� I2C
//--------------------------------------------------------------
static void P_I2C1_InitI2C(void)
{
    I2C_InitTypeDef I2C_InitStructure;

    // I2C-����������������
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

    // I2C ��������
    I2C_Cmd(I2C1, ENABLE);

    // ��������� �������������
    I2C_Init(I2C1, &I2C_InitStructure);
}

//--------------------------------------------------------------
// ���������� �������
// ����������, �� ����� ����-����
// ����, ����� � ����������������� ���������� I2C
//--------------------------------------------------------------
s16 P_I2C1_timeout(s16 wert)
{
    s16 ret_wert = wert;

    // ��������� � �������
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);

    // I2C ���������������
    I2C_DeInit(I2C1);
    // I2C �������������
    P_I2C1_InitI2C();

    return (ret_wert);
}
