#include <stdint.h>
#include "timer.h"

static int ms = 0;


/**
 * Инициализация прерываний таймера - получаем частоту 1000 Гц. 
 * freq = SysTick * 2/ ((prescal + 1) * (period + 1) * (autoreload + 1));
 */
void timer2_init(void)
{
    extern uint32_t SystemCoreClock;
    TIM_TimeBaseInitTypeDef timer;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_Prescaler = (u32) ((SystemCoreClock / 1000000) / 2) - 1;
    timer.TIM_Period = 999;
    TIM_TimeBaseInit(TIM2, &timer);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    NVIC_EnableIRQ(TIM2_IRQn);
}


/* Функцыя таймера - ЧАСТОТА 1000 гц */
int timer2_get_ms_ticks(void)
{
    return ms;
}

void timer2_call_back_func(void)
{
    ms++;
}
