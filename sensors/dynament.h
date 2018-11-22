#ifndef _DYNAMENT_H
#define _DYNAMENT_H

#include "main.h"

/* данные переданные на команды "выдай dyna_live_data_pack" */
#pragma pack(1)
typedef struct {
    u16 Version;	/* 1 */
    u16 StatusFlags;
    FLT_2_INT Reading;
    FLT_2_INT Temperature;
    u16 Det;
    u16 Ref;
    FLT_2_INT Fa;
    s32 Uptime;
    u16 DetMin;
    u16 DetMax;
    u16 RefMin;
    u16 RefMax;
} dyna_live_data_pack;

void sens_dyna_init(void);
void dyna_send_data(u8 *, u8);
int  dyna_get_live_data(void*);
void dyna_stop(void);
void dyna_start(void);


#endif /* dynament.h */
