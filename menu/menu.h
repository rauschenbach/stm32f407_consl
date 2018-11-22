#ifndef _MENU_H
#define _MENU_H

#include "dogm128.h"
#include "keyb.h"
#include "main.h"

#define 	MAX_MENU_STRING         32
#define		MAX_MENU_LEVELS 	4
#define		countof(a) 		(sizeof(a) / sizeof(*(a)))

/* ����� ��� ����������� ����. ���������� ������ �� */
#define 	MENU_FONT	font_7x14	/* ������� ����� ��� ���� */
#define 	CAPT_FONT  	font_7x14b	/* ������� ����� ��� ��������� */
#define 	SYMB_FONT 	font_6x10	/* ��� ����������� �������� */
#define 	INFO_FONT  	font_4x6	/* ��� ����� */
#define 	TIME_FONT 	font_9x18b	/* ��� ����� �������� � ���� */


/* ��������� ���� � ������ ������ */
typedef enum {
    MAIN_MENU_STATE,
    RTC_SET_STATE,		/* ��������� ����-������� */
    MAIN_SET_CHANNEL_STATE,	/* ������� ��������� ������� */
    ADDIT_SET_CHANNEL_STATE,	/* �������������� ��������� ������� */
    RANGES_SET_STATE,		/* ��������� */
    MENU_PUMP_SET_STATE,	/* ����� */
    MENU_VOLUME_SET_STATE,	/* ���� */
    MENU_ABOUT_STATE,		/* � ������� */
    MENU_CHAN_COEF_STATE,	/* ����������� */
    MENU_ACQUIS_STATE,		/* ��������� */
    MENU_ZERO_ALL_CHAN_STATE,	/* ����� ���� ������� */
    MENU_VCP_CONN_STATE,	/* ���������� �� VCP */
    MENU_CALIBR_STATE,		/* ���������� */
} menu_state_en;


/* ��� �������� */
typedef enum {
    NH3,			//������ 
    H2,				// �������
    SF6,			//����. ���� 
    NO2,			//�����. ����� 
    SO2,			// �����. ���� 
    O2,				// ������.
    RSH,			//�������.
    O3,				//���� 
    NO,				//���. ����� 
    H2S,			//����������� 
    HCN,			//���. ������� 
    CO,				//���. �������� 
    CO2,			//����. �����. 
    H2CO2,			//��������. 
    HF,				//����. ������� 
    CHC1F2,			//������ 
    Cl2,			//���� 
    HCl,			//������������ 
    C2H5OH,			//������ 
    C6H6,			//������ 
    C4H10,			//����� 
    C6H14,			//������ 
    CH4,			//����� 
    C3H8,			//������ 
    C2H4,			//������ 
} gase_type_en;

void IdleFunc(void);
void menu_create_task (void);
void set_vcp_menu(void);

#endif				/* menu.h */
