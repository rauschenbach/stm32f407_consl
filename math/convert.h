#ifndef _CONVERT_H
#define _CONVERT_H

#include "main.h"
#include "menu.h"

f32 ppm_to_vol(f32, gase_type_en);
f32 ppm_to_per(f32);
f32 per_to_ppm(f32);
f32 mv_to_ppm(f32, f32, f32);
f32 getnormal(f32);
void get_line_params(f32, f32, f32, f32, f32*, f32*);

#endif /* convert.h */
