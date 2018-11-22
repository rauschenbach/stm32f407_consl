#ifndef _STATUS_H
#define _STATUS_H

#include "globdefs.h"



void status_pump_on(void);
void status_pump_off(void);
void status_set_short(u32);
u32  status_get_short(void);


void status_init_first(void);
void status_get_full(DEV_STATUS_PACK*);

int32_t status_get_time(void);
void status_set_time(int32_t);
void status_sync_time(int);
int16_t status_get_temp(void);
void status_set_temp(int16_t);
int16_t status_get_humidity(void);
void status_set_humidity(int16_t);
s32 status_get_pressure(void);
void status_set_pressure(s32);
int16_t status_get_current(void);
void status_set_current(int16_t);
int16_t status_get_power(void);
void status_set_power(int16_t);

void status_get_sensor(void *);
void status_set_sensor(void *);
void status_dev_reset(void);

void status_vcp_conn(void);
void status_vcp_disconn(void);
bool status_is_conn(void);

#endif /* status.h */
