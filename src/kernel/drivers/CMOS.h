
#pragma once

#include <stdint.h>

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};

enum {
    cmos_reg_Seconds,
    cmos_reg_SecondsAlarm,
    cmos_reg_Minutes,
    cmos_reg_MinutesAlarm,
    cmos_reg_Hours,
    cmos_reg_HoursAlarm,
    cmos_reg_DayOfWeek,
    cmos_reg_DateOfMonth,
    cmos_reg_Month,
    cmos_reg_Year,
    cmos_reg_StatusA,
    cmos_reg_StatusB,
    cmos_reg_StatusC,
    cmos_reg_StatusD,
};

typedef struct datetime {
    uint16_t century;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}datetime;

int is_leap_year(int year, int month);
int get_weekday_from_date(datetime * dt);