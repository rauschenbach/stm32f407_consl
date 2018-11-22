#ifndef _LMP91K_H
#define _LMP91K_H

#include "sensor.h"
#include "main.h"


/**
 *  Регистры модуля LMP91000
 */
typedef enum {
    STATUS_REG,
    LOCK_REG,
    TIACN_REG = 0x10,
    REFCN_REG,
    MODECN_REG,
} LMP91k_REGS_ENUM;


void lmp91k_port_init(void);
void lmp91k_on(u8);
int  lmp91k_check(u8*);
int  lmp91k_config(u8*);
int  lmp91k_stop(void);

int  lmp91k_get_tia_gain(u8); 
int  lmp91k_get_bias(u8);
int  lmp91k_get_int_zero(u8);
int  lmp91k_get_rload(u8);
int lmp91k_get_sign(u8);

#endif				/* lmp91k.h */
