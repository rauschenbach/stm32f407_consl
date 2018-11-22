#ifndef _INDIC_H
#define _INDIC_H

#include "main.h"

void show_pump(int);
void show_acquis(int, void*, int, int);

void show_clock(int);
void show_bat(int);
void show_volume(int);
void show_about(void);
void set_calibr_point(int, int, int, char*, int, f32, bool);
void show_formula(int, int, const char*);
void show_formula2(int, int, const char*);
void show_units(int, int, const char *);
void show_caption(int, int, const char *);
void show_fract(int, int, const char *, int);
void show_onoff_chan(int, int, const char *, int, u8);
void show_num_chan(int, int, const char *, int);
void show_mult_coef(int, int, const char *, f32);
void show_zero_shift(int, int, const char *, f32);
void show_thresh_diapaz(int, int, const char *, f32);
void show_logo(int, int, char*);
void show_tiacn_reg(int, int, u8);
void show_bias(int, int, u8);
void show_int_zero(int, int, u8);
void show_one_chan_acquis(int, int, int, f32);

#endif /* indic.h */
