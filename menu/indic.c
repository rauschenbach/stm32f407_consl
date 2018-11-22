#include <string.h>
#include <ctype.h>
#include "userfunc.h"
#include "sensor.h"
#include "dogm128.h"
#include "indic.h"
#include "lmp91k.h"

/**
 * ��� ���������� - �������� ����� � ����� �������� � �������� �������� ���������
 */
void set_calibr_point(int x, int y, int num, char* str, int u, f32 data, bool ok)
{
    char buf[64];
    if(u == PPM_MODE) {
      sprintf(buf, "%s %d: %06.00f", str, num, data);
    } else { // ��������
         sprintf(buf, "%s %d: %06.02f   %s", str, num, data, ok? "Ok": " ");
    }
    dog_DrawStrP(x, y, MENU_FONT, buf);  
}

/**
 * ������� ��������� � ������ ������
 */
void show_one_chan_acquis(int x, int y, int num, f32 mv)
{
    char buf[32];

    sprintf(buf, "%d", (int) mv);

    /* ������� �������� */
    dog_DrawStrP(x, y, MENU_FONT, buf);
 
    x += dog_GetStrWidth(MENU_FONT, buf);    

    /* ������� ������� ��������� mV */
    show_units(x + 2, y, "mV");
}


/**
 * ������� ��������� ������������� � ������� � n
 * � ����������� �� ������ n ������� �������
 * H�lf - ����� �������� ��������� �������
 * ���� vert - 8 ��������� ������ � ����� ����
 */
void show_acquis(int c, void *p, int vert, int half)
{
    static const u8 x_pos[] = { 5, 20, 75, 125 };	/* ������� ������ �� X */
    static const u8 y_pos[] = { 70, 55, 40, 25 };	/* ������� ������ �� Y */
    char buf[32];
    channel_params_settings *set = (channel_params_settings *) p;
    int i, n, k, x;
    int units;
    float f_out, ppm, mv, per;
    float gain, shift;

    if (vert == 0) {
	for (i = 0; i < 4; i++) {

	    /* ������ �������� */
	    if (half) {
		n = i + 4;
	    } else {
		n = i;
	    }

	    /* ������� ����� ������ */
	    sprintf(buf, "%d", n + 1);
	    dog_DrawStrP(x_pos[0], y_pos[i], MENU_FONT, buf);

	    /* ���� ����� �������� - �� ���������� */
	    if (set[n].sens_type == NO_SENSOR) {
		dog_DrawStrP(x_pos[1], y_pos[i], MENU_FONT, "--- Off ---");
		continue;
	    }

	    /* ������� ������� */
	    show_formula2(x_pos[1], y_pos[i], set[n].formula);
	    gain = getnormal(set[n].gain);
	    shift = getnormal(set[n].shift);

/*
	    if (n == 0) {
		gain = -146.976;
		shift = 323641.375;
	    }
*/          
            
	    k = set[n].num_digits;	/* ����� ���� ��� ����� ������� */

	    /* � ����������� �� ��������, ���� ������� � ������ ��������� */
	    units = set[n].type_units;
	    if (set[n].sens_type == I2C_SENSOR) {
		mv = set[n].data.f_val;	/* mv */
		ppm = mv_to_ppm(mv, gain, shift);	/* ��������� � ppm */

		if (units == MMGM_MODE) {
		    f_out = ppm_to_vol(ppm, (gase_type_en) set[n].num_of_gas);	/* ��������� ppm � ��/�3 */
		} else if (units == PPM_MODE) {	/* ppm */
		    f_out = ppm;
		} else if (units == PERCENT_MODE) {	/* % */
		    f_out = ppm_to_per(ppm);
		} else {
		    f_out = mv;	/* mV */
		    k = INT_MODE;	/* �� ����� ����� */
		}
	    } else {
		per = set[n].data.f_val;	/* Modbus ���������� � % */
		ppm = per_to_ppm(per);	/* ������� �������� � PPM */

		if (units == PPM_MODE) {
		    f_out = ppm;
		} else if (units == PERCENT_MODE) {
		    f_out = per;
		} else {	/* ��/�3 */
		    f_out = ppm_to_vol(ppm, (gase_type_en) set[n].num_of_gas);
		}
	    }

	    /* ������� ���� ���������� */
	    if (k == TEN_MODE) {
		sprintf(buf, "%2.1f", f_out);
	    } else if (k == HUN_MODE) {
		sprintf(buf, "%2.2f", f_out);
	    } else {
		sprintf(buf, "%d", (int) f_out);
	    }
	    /* ������� �������� */
	    dog_DrawStrP(x_pos[2], y_pos[i], MENU_FONT, buf);

	    /* ������� ������� ��������� mV - ��/�3 - ppm - % */
	    show_units(x_pos[3], y_pos[i], (units == 0) ? "mV" : (units == 1) ? "\x96\x90\x2f\x96\x33" : (units == 2) ? "ppm " : " % ");
	}
    } else {
	n = 0;
	/* �� ��������� - �������� ����� ��� ��������� */
	for (i = 0; i < 8; i++) {
	    n = i % 4;
	    x = (i < 4) ? 2 : 90;

	    sprintf(buf, "%d.", i + 1);
	    dog_DrawStrP(x, y_pos[n], MENU_FONT, buf);

	    /* ���� ����� �������� - �� ���������� */
	    if (set[i].sens_type == NO_SENSOR) {
		dog_DrawStrP(x + 20, y_pos[n], MENU_FONT, " Off ");
		continue;
	    }

	    gain = getnormal(set[i].gain);
	    shift = getnormal(set[i].shift);

	    k = set[i].num_digits;	/* ����� ���� ��� ����� ������� */

	    /* � ����������� �� ��������, ���� ������� � ������ ��������� */
	    units = set[i].type_units;
	    if (set[i].sens_type == I2C_SENSOR) {
		mv = set[i].data.f_val;	/* mv */
		ppm = mv_to_ppm(mv, gain, shift);	/* ��������� � ppm */

		if (units == MMGM_MODE) {
		    f_out = ppm_to_vol(ppm, (gase_type_en) set[i].num_of_gas);	/* ��������� ppm � ��/�3 */
		} else if (units == PPM_MODE) {	/* ppm */
		    f_out = ppm;
		} else if (units == PERCENT_MODE) {	/* % */
		    f_out = ppm_to_per(ppm);
		} else {
		    f_out = mv;	/* mV */
		    k = INT_MODE;	/* �� ����� ����� */
		}
	    } else {
		per = set[i].data.f_val;	/* Modbus ���������� � % */
		ppm = per_to_ppm(per);	/* ������� �������� � PPM */

		if (units == PPM_MODE) {
		    f_out = ppm;
		} else if (units == PERCENT_MODE) {
		    f_out = per;
		} else {	/* ��/�3 */
		    f_out = ppm_to_vol(ppm, (gase_type_en) set[i].num_of_gas);
		}
	    }

	    /* ������� ���� ���������� */
	    if (k == TEN_MODE) {
		sprintf(buf, "%2.1f", f_out);
	    } else if (k == HUN_MODE) {
		sprintf(buf, "%2.2f", f_out);
	    } else {
		sprintf(buf, "%d", (int) f_out);
	    }
	    /* ������� �������� */
	    dog_DrawStrP(x + 20, y_pos[n], MENU_FONT, buf);

	}
    }
}

/**
 * ������� ������� ��������, ����� ����� ���� ������ ���� 
 */
void show_formula2(int x, int k, const char *buf)
{
    DOG_PGM_P font = MENU_FONT;	/* ������� ����� ��� ���� � �������� */
    char str[64], *p;
    char s;
    int y, i, pos = 0;

    /* ��������� ������� */
    for (i = 0; i < strlen(buf); i++) {
	if (buf[i] == '(') {
	    pos = i + 1;
	} else if (buf[i] == ')') {
	    strcpy(str, &buf[pos]);
	    str[i - pos] = 0;
	}
    }
    p = str;

    /* ����� � ������� ����� */
    while (s = *p++) {
	if (isdigit(s)) {
	    y = k - 2;
	} else {
	    y = k;
	}
	dog_DrawChar(x, y, font, s);
	x += dog_GetCharWidth(font, s);
    }
}

/**
 * ������� ��������� ����
 */
void show_caption(int x, int y, const char *str)
{
    dog_DrawStrP(x, y, CAPT_FONT, DOG_PSTR(str));
    dog_DrawStrP(0, y - 2, MENU_FONT, DOG_PSTR("__________________________________________________________"));
}

/**
 * ������� ��������� ������ � ������� �� 0 �� 100%
 */
void show_pump(int level)
{
    char str[32];
    int i;

    /* ������ ������������� */
    dog_DrawLine(20, 70, 140, 70, 5);
    dog_DrawLine(22, 68, 138, 68, 5);

    dog_DrawLine(20, 70, 20, 30, 5);
    dog_DrawLine(22, 68, 22, 32, 5);

    dog_DrawLine(140, 70, 140, 30, 5);
    dog_DrawLine(138, 68, 138, 32, 5);

    dog_DrawLine(22, 32, 138, 32, 5);
    dog_DrawLine(20, 30, 140, 30, 5);

    /* ������� �������� */
    sprintf(str, "%#3d %%", level);
    dog_DrawStrP(65, 45, CAPT_FONT, DOG_PSTR(str));

    /* 
     * 0 - 22, 100 - 138 
     * �������� �������
     */
    i = (116 * level + 2200) / 100;

    dog_XorBox(22, 32, i, 68);
}

/**
 * ������� ��������� ���� � ������� ������� ����� ������
 * ����������� ������ 10 ������
 */
void show_clock(int t0)
{
    char date[32];
    char time[16];
    int clock;

    clock = get_sec_ticks();

    sec_to_str(t0, date);	/* � ���� � ����� */
    strcpy(time, &date[9]);
    date[8] = 0;

    if (clock % 20 < 10) {
	dog_DrawStrP(1, 2, INFO_FONT, DOG_PSTR(date));	/* ���� */
    } else {
	dog_DrawStrP(1, 2, INFO_FONT, DOG_PSTR(time));	/* ����� */
    }
}

/**
 * �������. ������ ������� ������� 0...100 �� 20%
 */
void show_bat(int bat)
{
    int i;

#define X0      (140)
#define Y0      (9)
#define X1      (153)
#define Y1      (2)

    dog_DrawLine(X0, Y0, X1, Y0, 5);
    dog_DrawLine(X0, Y0, X0, Y1, 5);
    dog_DrawLine(X1, Y0, X1, Y1, 5);
    dog_DrawLine(X1 + 1, Y0 - 2, X1 + 1, Y1 + 2, 1);
    dog_DrawLine(X0, Y1, X1, Y1, 5);

    /* �� 0 �� 5 */
    if (bat > 5) {
	bat = 5;
    }

    /* ������ ������� ������� */
    for (i = 1; i < bat; i++) {
	dog_DrawLine(X0 + i * 2, Y0 - 2, X0 + i * 2, Y1 + 2, 5);
    }
}


/**
 * �������� ������ ������ ���������
 * 0...100
 */
void show_volume(int level)
{
    char str[32];
    int i;

    /* ������ ������������� ������� */
    dog_DrawLine(40, 70, 60, 70, 1);
    dog_DrawLine(40, 40, 60, 40, 1);
    dog_DrawLine(40, 70, 40, 40, 1);
    dog_DrawLine(60, 70, 60, 40, 1);

    /* ������ �����. ������� */
    dog_DrawLine(60, 40, 80, 30, 1);
    dog_DrawLine(80, 30, 80, 80, 1);
    dog_DrawLine(60, 70, 80, 80, 1);

    /* ��������� */
    if (level > 100) {
	level = 100;
    }
    sprintf(str, "%d %%", level);
    dog_DrawStrP(85, 25, MENU_FONT, DOG_PSTR(str));

    /* ������ ������������� ����� */
    if (level == 0) {
	dog_DrawLine(20, 40, 100, 70, 1);
    }


    level = level / 20;

    for (i = 0; i < level; i++) {
	dog_DrawArc(80, 55, 15 + i * 10, -5 - i * 4, 5 + i * 4, 1);
    }
}

/**
 * ������� ������� ���������  
 */
void show_units(int x, int k, const char *str)
{
    DOG_PGM_P font;
    char s;
    int y;

    /* ����� � ������� ����� ����� �������, ����� ������ */
    while (s = *str++) {
	if (isdigit(s)) {
	    font = SYMB_FONT;	/* ��� ����������� ���� */
	    y = k + 4;
	} else {
	    font = MENU_FONT;	/* ������� ����� ��� ���� � �������� */
	    y = k;
	}
	dog_DrawChar(x, y, font, s);
	x += dog_GetCharWidth(font, s);
    }
}

/**
 * ������� ������� �����
 */
void show_fract(int x, int y, const char *str, int num)
{
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(str));
    dog_DrawStrP(x + strlen(str) * 7, y, MENU_FONT, DOG_PSTR(num == 0 ? "1" : num == 1 ? "1.2" : "1.24"));
}

/**
 * ������� ����� ������ � ���/����
 */
void show_onoff_chan(int x, int y, const char *str, int num, u8 on)
{
    char buf[64];
    static const char on_off[][9] = {
	"\x2d\x20\xa0\xa2\x94\x95\x20\x00",
	"- i2c  ",
	"- rs232 ",
    };

    /* ����� ������ */
    sprintf(buf, "%s%d %s", str, num + 1, on_off[on % 3]);	/* �����������! */
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}


/**
 * ������� ����� ������ ��� ���������
 */
void show_num_chan(int x, int y, const char *str, int num)
{
    char buf[64];

    /* ����� ������ */
    sprintf(buf, "%s%d", str, num + 1);
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}

/**
 * ������� ����. ���������
 */
void show_mult_coef(int x, int y, const char *str, f32 coef)
{
    char buf[64];

    /* ����������� - ������� ������ ����� � ���� � ������� ����� ������� */
    sprintf(buf, "%s%7.2f", str, coef);
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}


/**
 * ������� �������� ���� 
 */
void show_zero_shift(int x, int y, const char *str, f32 coef)
{
    char buf[64];

    /* ����������� �������� ���� */
    sprintf(buf, "%s%2.1f", str, coef);
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}

/**
 * ������� ������ � ���������
 */
void show_thresh_diapaz(int x, int y, const char *str, f32 coef)
{
    char buf[64];

    /* ����������� */
    sprintf(buf, "%s%2.1f", str, coef);
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}

/* ������� ������� ��������, ����� ����� ���� ������ ���� */
void show_formula(int x, int k, const char *str)
{
    DOG_PGM_P font;
    char s;
    int y;

    /* ����� � ������� ����� ����� �������, ����� ������ */
    while (s = *str++) {
	if (isdigit(s)) {
	    font = SYMB_FONT;	/* ��� ����������� ���� */
	    y = k - 2;
	} else {
	    font = MENU_FONT;	/* ������� ����� ��� ���� � �������� */
	    y = k;
	}
	dog_DrawChar(x, y, font, s);
	x += dog_GetCharWidth(font, s);
    }
}

/**
 * ������� �������� � �������� ��������
 * Tia_gain	    R_load
 */
void show_tiacn_reg(int x, int k, u8 reg)
{
    char buf[64];
    int gain, y, i;
    u8 load;
    DOG_PGM_P font;
    char kohm[] = "\x94\xB8\x96";
    char ohm[] = "\x98\x96";

    /* ����������� */
    gain = lmp91k_get_tia_gain(reg);
    load = lmp91k_get_rload(reg);

    if (gain == 2750) {
	sprintf(buf, "Rg: 2.75%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 3500) {
	sprintf(buf, "Rg:  3.5%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 7000) {
	sprintf(buf, "Rg:    7%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 14000) {
	sprintf(buf, "Rg:   14%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 35000) {
	sprintf(buf, "Rg:   35%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 120000) {
	sprintf(buf, "Rg:  120%s Rl: %3d%s", kohm, load, ohm);
    } else if (gain == 350000) {
	sprintf(buf, "Rg:  350%s Rl: %3d%s", kohm, load, ohm);
    } else {			/* 0 */

	sprintf(buf, "Rg:  Ext    Rl: %3d%s", load, ohm);
    }


    /* ����� � ������� ����� ����� �������, ����� ������ */
    for (i = 0; i < strlen(buf); i++) {
	if (buf[i] == 'g' || buf[i] == 'l') {
	    font = SYMB_FONT;	/* ��� ����������� ���� */
	    y = k - 2;
	} else {
	    font = MENU_FONT;	/* ������� ����� ��� ���� � �������� */
	    y = k;
	}
	dog_DrawChar(x, y, font, buf[i]);
	x += dog_GetCharWidth(font, buf[i]);
    }
}

/**
 * �������� ��������
 */
void show_bias(int x, int y, u8 reg)
{
    int bias, sign;
    char buf[64];

    bias = lmp91k_get_bias(reg);
    sign = lmp91k_get_sign(reg);

    sprintf(buf, "Bias: %s %#2d %%", sign < 0 ? "neg" : "pos", abs(bias));
    dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf));
}

/**
 * ������� ����������
 */
void show_int_zero(int x, int k, u8 reg)
{
    char buf[64];
    DOG_PGM_P font;
    int i, y, zero = lmp91k_get_int_zero(reg);

    if (zero != 0) {
	sprintf(buf, "Intzero: %#2d %%", zero);
    } else {
	sprintf(buf, "Intzero: bpsd ");
    }

    /* ����� � ������� ����� ����� �������, ����� ������ */
    for (i = 0; i < strlen(buf); i++) {
	if (buf[i] == 'z' || buf[i] == 'e' || buf[i] == 'r' || buf[i] == 'o') {
	    font = SYMB_FONT;	/* ��� ����������� ���� */
	    y = k - 2;
	} else {
	    font = MENU_FONT;	/* ������� ����� ��� ���� � �������� */
	    y = k;
	}
	dog_DrawChar(x, y, font, buf[i]);
	x += dog_GetCharWidth(font, buf[i]);
    }

/*  dog_DrawStrP(x, y, MENU_FONT, DOG_PSTR(buf)); */
}

/**
 * ������� �������
 */
void show_logo(int x, int y, char *str)
{
//    dog_DrawStrP(20, 60, CAPT_FONT, DOG_PSTR(StrAbout0));
//    dog_DrawStrP(20, 40, CAPT_FONT, DOG_PSTR(StrAbout1));
//  dog_XorBox(40, 100, img_width + 40, 50);
//    dog_DrawLine(20, 70, 140, 70, 5);
}
