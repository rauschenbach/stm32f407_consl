#include "adc.h"


// на пине PORTC 2


/**
  * @brief  ADC3 channel12 with DMA configuration
  * @param  None
  * @retval None
  */
void adc_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Тактирование АЦП и GPIO */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);


    /* Configure ADC3 Channel12 pin as analog input ***************************** */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);


    /* ADC Common Init */
    ADC_DeInit();
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);


    ADC_InitStructure.ADC_ScanConvMode = DISABLE;    
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;    
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

    ADC_Init(ADC3, &ADC_InitStructure);
    ADC_Cmd(ADC3, ENABLE);
}


/**
 * Старт измерений
 */
void adc_start(void)
{
      ADC_ClearFlag(ADC3, ADC_FLAG_OVR);  /* Сбросим флаг OVR */      
    //  ADC_Cmd(ADC3, ENABLE);
}


/**
 * Остановиь измерения
 */
void adc_stop(void)
{
    // ADC_Cmd(ADC3, DISABLE);
}


/**
 * convert the ADC value (from 0 to 0xFFF) to a voltage value (from 0V to 3.0V)
 * Выход в мвольтах
 */
u16 adc_read_chan(u8 i)
{
    u32 data;

    ADC_ClearFlag(ADC3, ADC_FLAG_OVR);  /* Сбросим флаг OVR */

    /* настройка - канал 1, время преобразования 480 цикла */
    ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_480Cycles);

    /* начать преобразование */
    ADC_SoftwareStartConv(ADC3);


    /* ждем окончания преобразования */
    while (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET ) {
     if(!(status_get_short() & 0x80))
        break;
    }

    data = ADC_GetConversionValue(ADC3);

    //   log_printf("data--> %04X - %d\n", data, data);

    /* возвращаем полученное значение, приведенное к mv */
    return ((data >> 4) * 3000) / 4095;
}
