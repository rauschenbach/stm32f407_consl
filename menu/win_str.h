#ifndef _WIN_STR_H
#define _WIN_STR_H


/************************************ строки. НЕ КОНСТАНТЫ!!!****************************/
static char StrTime[] = "Время: ";
static char StrDate[] = "Дата:  ";
static char StrNumChan[] = "Канал: ";
static char StrOnOff[] = "Вкл/Выкл: ";
static char StrOn[] = "- вкл ";
static char StrOff[] = "- выкл";
static char StrFormula[] = "Формула: ";
static char StrUnits[] = "Ед. измерений: ";
static char StrDigits[] = "Дробная часть: ";

static char StrMultCoef[]  = "Коэф. умнож.: ";
static char StrZeroShift[] = "Смещен. нуля: ";


static char StrThreshMin[] = "Чувст.  мин: ";
static char StrThreshMax[] = "Чувст. макс: ";
static char StrDiapazMax[] = "Диапаз. изм: ";
static char StrVcpConn0[] = " Соединение ";
static char StrVcpConn1[] = "   по VCP ";


static char StrZeroinChan0[] = "Обнулить EEPROM?";
static char StrZeroinChan1[] = "<SET> - Обнулить";
static char StrZeroinChan2[] = "<ESC> - Отмена";

static char StrCalibrPoint[] = "Точка";



static char StrAbout0[] =   "Газоанализатор на";
static char StrAbout1[] = "   8 веществ";
static char StrMg[] = "мг/м3";
static char StrSubst[][20] = {
    "Аммиак (NH3)",
    "Водород (H2)",
    "Гекс. серы (SF6)",
    "Диокс. азота (NO2)",
    "Диокс. серы (SO2)",
    "Кислор.(O2)",
    "Меркапт.(RSH)",
    "Озон (O3)",
    "Окс. азота (NO)",
    "Сероводород (H2S)",
    "Син. кислота (HCN)",
    "Окс. углерода (CO)",
    "Диок. углер. (CO2)",
    "Формальд. (H2CO2)",
    "Фтор. водород (HF)",
    "Хладон (CHC1F2)",
    "Хлор (Cl2)",
    "Хлороводород (HCl)",
    "Этанол (C2H5OH)",
    "Бензол (C6H6)",
    "Бутан (C4H10)",
    "Гексан (C6H14)",
    "Метан (CH4)",
    "Пропан (C3H8)",
    "Этилен (C2H4)",
};


#endif /* _WIN_STR_H  */
