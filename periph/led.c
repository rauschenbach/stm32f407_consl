#include "led.h"


/* Вместо времени будет */
int led_quasi_time = 0;

static void vLedTask(void *);

/* Инициализация LED  */
void led_init(void)
{
    STM_EVAL_LEDInit(LED5);
    STM_EVAL_LEDInit(LED6);
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);
}


int led_on(Led_TypeDef led)
{
    STM_EVAL_LEDOn(led);
    return 0;
}


int led_off(Led_TypeDef led)
{
    STM_EVAL_LEDOff(led);
    return 0;
}

int led_toggle(Led_TypeDef led)
{
    STM_EVAL_LEDToggle(led);
    return 0;
}


void led_create_task(void)
{
    xTaskHandle task = NULL;    

    led_init();

    xTaskCreate(vLedTask, (s8 *) "LedTask", LED_TASK_STACK_SIZE, NULL, LED_TASK_PRIORITY, &task);
    if (task == NULL) {
 	log_printf("ERROR: Create LedTask\r\n");
	configASSERT(task);
     }
    log_printf("SUCCESS: Create LedTask\r\n");
}

/**
 * Задача моргает лампами
 */
static void vLedTask(void *p)
{
    led_quasi_time = time(NULL);

    while (1) {
	led_toggle(LED4);
        led_quasi_time++;
        vTaskDelay(1000);
    }
}

/**
 * Установка квази-времени
 */
#if 0
void set_time(int t0)
{
	led_quasi_time = t0;  
}
#endif