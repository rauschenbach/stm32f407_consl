#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "eeprom.h"
#include "main.h"
#include "menu.h"
#include "rtc.h"
#include "vcp.h"

/* Для подсчета времени и таймаутов */
static int  sys_tick_ms = 0;
static void hw_init_all(void);
static void all_task_error(void);       

/**
  * Запускаем все задачи, в том числе и ожидающие
  */
int main(void)
{
    hw_init_all();

    led_create_task();
    rtc_create_task();		/* Задача для RTC - можно будет перенести в Idle или другое место */
    menu_create_task();		/* Инициализация задачи для меню и клавиатуры */    
    sensor_create_task();	/* Инициализация задачи для сенсоров */    
#if 1
    vcp_create_task();          /* Виртуальный ком порт - пока обычный COM порт */
#else
    com_create_task();          /* Виртуальный ком порт - пока обычный COM порт */
#endif

    vTaskStartScheduler();      /* Теперь вызываем старт планировщика */
    all_task_error();           /* Если у нас будет проблема со стеком во время запуска-попадем сюда */
    return 0;
}

/**
 * Настраиваем периферию
 */
static void hw_init_all(void)
{
    status_init_first();
    timer2_init();
    eeprom_init();     /* Init eeprom - при правильном чтении сразу убрать статус "нет констант" */  
}

/**
 * Моргаем всеми лампами
 */
static void all_task_error(void)
{
    while (1) {
	led_toggle(LED3);
	led_toggle(LED4);
	led_toggle(LED5);
	led_toggle(LED6);
	for (volatile int i = 0; i < 0x80000; i++);
    }
}

/* Decrement the TimingDelay variable */
void vApplicationTickHook(void)
{
    sys_tick_ms++;
    KeypadScan();
}

/* vApplicationMallocFailedHook() will only be called if
 * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
 * function that will get called if a call to pvPortMalloc() fails.
 * pvPortMalloc() is called internally by the kernel whenever a task, queue,
 * timer or semaphore is created.  It is also called by various parts of the
 * demo application.  If heap_1.c or heap_2.c are used, then the size of the
 * heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
 * FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
 * to query the size of free heap space that remains (although it does not
 * provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
 * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
 * task.  It is essential that code added to this hook function never attempts
 * to block in any way (for example, call xQueueReceive() with a block time
 * specified, or call vTaskDelay()).  If the application makes use of the
 * vTaskDelete() API function (as this demo application does) then it is also
 * important that vApplicationIdleHook() is permitted to return to its calling
 * function, because it is the responsibility of the idle task to clean up
 * memory allocated by the kernel to any task that has since been deleted. */
void vApplicationIdleHook(void)
{
}


/* Run time stack overflow checking is performed if
 * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
 * function is called if a stack overflow is detected. 
 */
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/**
 * Число миллисекунд
 */
int get_sys_ticks(void)
{
    return sys_tick_ms;
}
