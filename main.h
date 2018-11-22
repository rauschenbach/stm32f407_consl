#ifndef _MAIN_H
#define _MAIN_H

#include <stm32f4_discovery.h>
#include <stm32f4xx_conf.h>
#include <stm32f4xx.h>
#include <core_cm4.h>
#include <userfunc.h>
#include <com_cmd.h>
#include <convert.h>
#include <status.h>
#include <sensor.h>
#include <timer.h>
#include <menu.h>
#include <led.h>

/*
#include <usbd_cdc_core.h>
#include <usbd_cdc_vcp.h>
#include <usbd_usr.h>
#include <usb_conf.h>
#include <usbd_desc.h>
*/
#include "globdefs.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stackmacros.h"

#define LED_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)
#define SENSOR_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define MENU_TASK_PRIORITY				(tskIDLE_PRIORITY + 4)
#define RTC_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)


#define LED_TASK_STACK_SIZE				(configMINIMAL_STACK_SIZE)
#define SENSOR_TASK_STACK_SIZE				(configMINIMAL_STACK_SIZE)
#define MENU_TASK_STACK_SIZE				(configMINIMAL_STACK_SIZE * 2)
#define RTC_TASK_STACK_SIZE				(configMINIMAL_STACK_SIZE)



/* Exported functions ------------------------------------------------------- */
int get_sys_ticks(void);

#endif /* _MAIN_H */
