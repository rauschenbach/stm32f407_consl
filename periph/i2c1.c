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
 * Hinweis  : mцgliche Pinbelegungen
 *            I2C1 : SCL :[PB6, PB8] 
 *                   SDA :[PB7, PB9]
 *            externe PullUp-Widerstдnde an SCL+SDA notwendig
 *****************************************************************************/


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------

#include "i2c1.h"


u8 I2C1_DATA[I2C1_MULTIBYTE_ANZ];	// массив

//--------------------------------------------------------------
// внутренняя функция
//--------------------------------------------------------------
static s16 P_I2C1_timeout(s16 wert);
static void P_I2C1_InitI2C(void);


//--------------------------------------------------------------
// Определение I2C1
//--------------------------------------------------------------
static I2C1_DEV_t I2C1DEV = {
// PORT , PIN      , Clock              , Source 
    {GPIOB, GPIO_Pin_6, RCC_AHB1Periph_GPIOB, GPIO_PinSource6},	// SCL к PB6
    {GPIOB, GPIO_Pin_9, RCC_AHB1Periph_GPIOB, GPIO_PinSource9},	// SDA к PB9
};



//--------------------------------------------------------------
// Инициализация I2C1
//-------------------------------------------------------------- 
void ub_i2c1_init(void)
{
    static u8 init_ok = 0;
    GPIO_InitTypeDef GPIO_InitStructure;

    // инициализация, выполняется один раз
    if (init_ok != 0) {
	return;
    }
    // I2C-тактирование, включение
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // включение тактирования пинов
    RCC_AHB1PeriphClockCmd(I2C1DEV.SCL.CLK, ENABLE);
    RCC_AHB1PeriphClockCmd(I2C1DEV.SDA.CLK, ENABLE);

    // I2C рестарт
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);

    // включение альтернативной функции портов ввода/вывода как I2C
    GPIO_PinAFConfig(I2C1DEV.SCL.PORT, I2C1DEV.SCL.SOURCE, GPIO_AF_I2C1);
    GPIO_PinAFConfig(I2C1DEV.SDA.PORT, I2C1DEV.SDA.SOURCE, GPIO_AF_I2C1);

    // I2C как альтернативная функция с OpenDrain  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // SCL-вывод
    GPIO_InitStructure.GPIO_Pin = I2C1DEV.SCL.PIN;
    GPIO_Init(I2C1DEV.SCL.PORT, &GPIO_InitStructure);
    // SDA-вывод
    GPIO_InitStructure.GPIO_Pin = I2C1DEV.SDA.PIN;
    GPIO_Init(I2C1DEV.SDA.PORT, &GPIO_InitStructure);

    // I2C инициализация
    P_I2C1_InitI2C();

    // Сохранить режим инициализации
    init_ok = 1;
}

/**
 * Чтение с адреса регистра
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
 * Чтение без адреса регистра
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
// Описание адреса I2C в режиме Slave
// slave_adr => I2C адрес Slave по умолчанию
// ADR => запись адреса
// Значение => запись байта
//
// возвращаемое значение :
//    0   , норма
//  < 0   , ошибка
//--------------------------------------------------------------
s16 ub_i2c1_write_byte(u8 slave_adr, u8 adr, u8 wert)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    // последовательный запуск
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // Отправка Slave адреса
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // запись адреса флага
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // отправка адреса
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // отправка данных
    I2C_SendData(I2C1, wert);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // остановка последовательности
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// все в порядке

    return (ret_wert);
}



s16 ub_i2c1_simple_write_byte(u8 slave_adr, u8 wert)
{
    s16 ret_wert = 0;
    u32 timeout = I2C1_TIMEOUT;

    // последовательный запуск
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // Отправка Slave адреса
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // запись адреса флага
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // отправка данных
    I2C_SendData(I2C1, wert);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // остановка последовательности
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// все в порядке

    return (ret_wert);
}


//--------------------------------------------------------------
// Адрес I2C в режиме Slave
// slave_adr => I2C адрес Slave по умолчанию
// adr=> начальный адрес регистра
// cnt => количество байт для чтения
// Данные, которые были прочитаны, записываются в "I2C1_DATA"
//
// Возвращаемое значение :
//    0   , норма
//  < 0   , ошибка
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

    // запуск последовательности
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    if (cnt == 1) {
	// ACK выключено
	I2C_AcknowledgeConfig(I2C1, DISABLE);
    } else {
	// ACK включено
	I2C_AcknowledgeConfig(I2C1, ENABLE);
    }

    // Отправка адреса Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // установка флага ADDR
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // отправка даты
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while ((!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
	   || (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // запуск последовательности
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-5));
    }

    // отправка адреса Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Receiver);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-6));
    }

    // установка флага ADDR
    I2C1->SR2;

    // читать все данные
    for (n = 0; n < cnt; n++) {

	if ((n + 1) >= cnt) {
	    // ACK выключено
	    I2C_AcknowledgeConfig(I2C1, DISABLE);
	    // остановка последовательности
	    I2C_GenerateSTOP(I2C1, ENABLE);
	}

	timeout = I2C1_TIMEOUT;
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE)) {
	    if (timeout != 0)
		timeout--;
	    else
		return (P_I2C1_timeout(-7));
	}

	// чтение данных
	wert = I2C_ReceiveData(I2C1);

	// хранение данных в массиве
	I2C1_DATA[n] = wert;
    }

    // ACK включено
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    ret_wert = 0;		// все в порядке

    return (ret_wert);
}


//--------------------------------------------------------------
// Адрес I2C в режиме Slave
// slave_adr => I2C адрес Slave по умолчанию
// adr=> начальный адрес регистра
// cnt => количество байт для чтения
// Данные, которые были прочитаны, записываются в "I2C1_DATA"
//
// Возвращаемое значение :
//    0   , норма
//  < 0   , ошибка
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

    // запуск последовательности
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-1));
    }

    // отправка адреса Slave
    I2C_Send7bitAddress(I2C1, slave_adr, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-2));
    }

    // установка флага ADDR
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-3));
    }

    // адрес отправлен
    I2C_SendData(I2C1, adr);

    timeout = I2C1_TIMEOUT;
    while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE)) {
	if (timeout != 0)
	    timeout--;
	else
	    return (P_I2C1_timeout(-4));
    }

    // отправить все данные
    for (n = 0; n < cnt; n++) {
	// Чтение данных из массива
	wert = I2C1_DATA[n];

	// данные отправлены
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

    //  остановка последовательности
    I2C_GenerateSTOP(I2C1, ENABLE);

    ret_wert = 0;		// все в порядке

    return (ret_wert);
}

//--------------------------------------------------------------
// короткая пауза
//--------------------------------------------------------------
void ub_i2c1_delay(volatile u32 nCount)
{
    while (nCount--) {
    }
}


//--------------------------------------------------------------
// внутренняя функция
// Инициализация интерфейса I2C
//--------------------------------------------------------------
static void P_I2C1_InitI2C(void)
{
    I2C_InitTypeDef I2C_InitStructure;

    // I2C-конфигурирование
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C1_CLOCK_FRQ;

    // I2C включено
    I2C_Cmd(I2C1, ENABLE);

    // структура инициализации
    I2C_Init(I2C1, &I2C_InitStructure);
}

//--------------------------------------------------------------
// внутренняя функция
// Вызывается, во время тайм-аута
// Стоп, сброс и переинициализация интерфейса I2C
//--------------------------------------------------------------
s16 P_I2C1_timeout(s16 wert)
{
    s16 ret_wert = wert;

    // остановка и рестарт
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, ENABLE);
    I2C_SoftwareResetCmd(I2C1, DISABLE);

    // I2C деинициализация
    I2C_DeInit(I2C1);
    // I2C инициализация
    P_I2C1_InitI2C();

    return (ret_wert);
}
