#ifndef _RTC_H
#define _RTC_H

#include "main.h"

int  rtc_init(void);
void rtc_set_time(void *p);
void rtc_get_time(void *p);
void rtc_get_alarm(void *p);
void rtc_set_alarm(void *p);
void set_alarm_irq(void);
void rtc_alarm_config(void);
void rtc_create_task(void);



#endif /* rtc.h */
