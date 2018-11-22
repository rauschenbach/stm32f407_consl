#include "convert.h"
#include <math.h>

/* Пока нету термометра и датчика давления */
static const f32 T = 293.15;		/* Температура в кельвинах == 20 С */
static const f32 P = 101325;		/* Давление в Паскалях == 1 атм */


/** конвертируем содержание ppm в мг/м3
 * f32 ppm_to_vol(int ppm, int Celsius, int Pressure, gase_type_en) 
 */
f32 ppm_to_vol(f32 ppm, gase_type_en type)
{
    f32 M = 1;			/* Молекулярная (молярная) масса газа */
    f32 data;

    switch (type) {

	/* Аммиак  */
    case NH3:
	M = 17.031;
	break;

	/* Водород */
    case H2:
	M = 2.0156;
	break;

	/* Гексафторид серы */
    case SF6:
	M = 146.01;
	break;

	/* Диокс. азота */
    case NO2:
	M = 46.0055;
	break;

	/*  Диокс. серы */
    case SO2:
	M = 64.06;
	break;

	/*  Кислор.  */
    case O2:
	M = 32.0;
	break;

	/* Меркапт. */
    case RSH:
	M = 1;
	break;

	/* Озон */
    case O3:
	M = 48.0;
	break;

	/* ОкиСЬ азота */
    case NO:
	M = 30.008;
	break;

	/* Сероводород */
    case H2S:
	M = 34.08;
	break;

	/* Син. кислота */
    case HCN:
	M = 27.0253;
	break;

	/* Окись углерода */
    case CO:
	M = 28.01;
	break;

	/*  Диок. углер. */
    case CO2:
	M = 44.01;
	break;

	/* Формальд. */
    case H2CO2:
	M = 30.03;
	break;

	/* Фтор. водород */
    case HF:
	M = 20.01;
	break;

	/* Хладон */
    case CHC1F2:
	M = 86.2;
	break;

	/* Хлор */
    case Cl2:
	M = 70.914;
	break;

	/* Хлороводород */
    case HCl:
	M = 35.452;
	break;

	/* Этанол */
    case C2H5OH:
	M = 46.069;
	break;

	/* Бензол */
    case C6H6:
	M = 78.11;
	break;

	/* Бутан */
    case C4H10:
	M = 58.12;
	break;

	/* Гексан */
    case C6H14:
	M = 86.17;
	break;

	/* Метан  */
    case CH4:
	M = 16.04;
	break;

	/* Пропан */
    case C3H8:
	M = 44.09;
	break;

	/* Этилен */
    case C2H4:
	M = 28.05;
	break;

    default:
	M = 10;
	break;
    }
    data = 0.12 * 1e-6 * ppm * P * M / T;
    return data;
}

/* ppm в проценты */
f32 ppm_to_per(f32 ppm)
{
    return ppm * 1e-4;
}


/* проценты в ppm */
f32 per_to_ppm(f32 per)
{
    return per * 10000.0;
}


/**
 * mv на выходе в ma или ppm
 * По формуле прямой
 */
f32 mv_to_ppm(f32 mv, f32 gain, f32 shift)
{
    f32 data = gain * mv + shift;
    return data;		/* в ppm */
}

/**
 * Возватить число, с проверкой на нормальность
 */
f32 getnormal(f32 x)
{
    if (isfinite(x)) {
	if (fabs(x) > 0.000000001 && fabs(x) < 1000000)
	    return x;
	else
	    return 0;
    }
    return 0;
}

/**
 * Расчет коэффициентов прямой по 2-м точкам
 * Вход Y проценты-переводим в PPM
 */
void get_line_params(f32 x0, f32 m0, f32 x1, f32 m1, f32 * gain, f32 * shift)
{
    f32 y0, y1;
    f32 g, s;

    /* Переводим проценты в PPM */
    y0 = per_to_ppm(m0);
    y1 = per_to_ppm(m1);

    if (x1 != x0) {
	g = (y1 - y0) / (x1 - x0);
	s = y1 - g * x1;
    }
    log_printf("Запись калибровочных данных\r\n");
    log_printf("Точка 0: %f %f\r\n", x0, y0);
    log_printf("Точка 1: %f %f\r\n", x1, y1);
    log_printf("Gain: %f, shift: %f", g, s);
    *gain = g;
    *shift = s;
}
