#ifndef _CRC16_H
#define _CRC16_H

#include "globdefs.h"


#define 	CRC16_TABLE_SIZE		256

uint16_t    check_crc16(u8 *, u16);
uint16_t    get_crc16_table(u8);
void   add_crc16(u8*);
void add_check_sum(u8 *, int);

#endif /* crc16.h  */
