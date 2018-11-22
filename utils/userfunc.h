#ifndef _USERFUNC_H
#define _USERFUNC_H


#include <stdio.h>
#include "globdefs.h"


int  log_printf(const char *, ...);
long get_sec_ticks(void);
void str_to_cap(char *, int);
int  PRINTF(char*,...);
int  sec_to_tm(long, struct tm *);
void set_time(long);
long tm_to_sec(struct tm *);
int  sec_to_str(long, char *);
int  parse_date_time(void);
c8 *win_to_koi8(c8 *);
c8 *koi8_to_win(const c8 *);

#endif				/* userfunc.h */

