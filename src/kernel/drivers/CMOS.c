
#include "CMOS.h"
#include "arch/i686/i8259.h"
#include "arch/i686/io.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"
#include "malloc.h"

#define CURRENT_YEAR 2023 // Change this each year!
char *weekday_map[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
datetime current_datetime; // Set by ACPI table parsing code if possible

/* CMOS values are stored like so:
 * Say it's 8:42 AM, then the values are stored as:
 * 0x08, 0x42... why this was a good idea, I have no
 * clue, but that's how it usually is.
 *
 * This function will convert between this "BCD" format
 * and regular decimal integers. */
#define from_bcd(val) ((val / 16) * 10 + (val & 0xf))

uint8_t readRTCRegister(int reg)
{
    i686_outb(cmos_address, reg);
    return i686_inb(cmos_data);
}
void writeRTCRegister(int reg, uint8_t data)
{
    i686_outb(cmos_address, reg);
    i686_outb(cmos_data, data);
}

#define isUpdatingRTC() readRTCRegister(cmos_reg_StatusA) & 0x80

/*
 * Read current date and time from rtc, store in global var current_datetime
 * */
void rtc_read_datetime(datetime* datetime)
{
    // Wait until rtc is not updating
    while (isUpdatingRTC())
    {
    }

    datetime->second = readRTCRegister(0x00);
    datetime->minute = readRTCRegister(0x02);
    datetime->hour = readRTCRegister(0x04);
    datetime->day = readRTCRegister(0x07);
    datetime->month = readRTCRegister(0x08);
    datetime->year = readRTCRegister(0x09);

    uint8_t registerB = readRTCRegister(0x0B);

    // Convert BCD to binary values if necessary
    if (!(registerB & 0x04))
    {
        datetime->second = from_bcd(datetime->second);
        datetime->minute = from_bcd(datetime->minute) * 10;
        datetime->hour = from_bcd(datetime->hour) | (datetime->hour & 0x80);
        datetime->day = from_bcd(datetime->day);
        datetime->month = from_bcd(datetime->month);
        datetime->year = from_bcd(datetime->year);
    }
}

/*
 * Write a datetime struct to rtc
 * */
void rtc_write_datetime(datetime *dt)
{
    // Wait until rtc is not updating
    while (isUpdatingRTC())
        ;

    writeRTCRegister(0x00, dt->second);
    writeRTCRegister(0x02, dt->minute);
    writeRTCRegister(0x04, dt->hour);
    writeRTCRegister(0x07, dt->day);
    writeRTCRegister(0x08, dt->month);
    writeRTCRegister(0x09, dt->year);
}

/*
 * A convenient function that converts a datetime struct to string
 * Only support the format: "day hour:minute"
 * For example, "Sat 6:32"
 * */
char *datetime_to_str(datetime *dt, Page* page)
{
    char *ret = calloc(15, 1, page);
    char day[4];
    char hour[3];
    char min[3];

    memset(&day, 0x0, 4);
    memset(&hour, 0x0, 3);
    memset(&min, 0x0, 3);

    int weekdayInt = get_weekday_from_date(dt);
    const char *weekday = weekday_map[weekdayInt];
    strcpy(day, weekday);
    itoa(hour, dt->hour, 10);
    itoa(min, dt->minute, 10);

    strcpy(ret, day);
    strcat(ret, " ");
    strcat(ret, hour);
    strcat(ret, ":");
    strcat(ret, min);
    strcat(ret, " PM");
    return ret;
}

char *get_current_datetime_str(Page* page)
{
    return datetime_to_str(&current_datetime, page);
}

/*
 * Given a date, calculate it's weekday, using the algorithm described here: http://blog.artofmemory.com/how-to-calculate-the-day-of-the-week-4203.html
 * */
int get_weekday_from_date(datetime *dt)
{
    char month_code_array[] = {0x0, 0x3, 0x3, 0x6, 0x1, 0x4, 0x6, 0x2, 0x5, 0x0, 0x3, 0x5};
    char century_code_array[] = {0x4, 0x2, 0x0, 0x6, 0x4, 0x2, 0x0}; // Starting from 18 century

    // Simple fix...
    dt->century = 21;

    // Calculate year code
    int year_code = (dt->year + (dt->year / 4)) % 7;
    int month_code = month_code_array[dt->month - 1];
    int century_code = century_code_array[dt->century - 1 - 17];
    int leap_year_code = is_leap_year((int)dt->year);

    int ret = (year_code + month_code + century_code + dt->day - leap_year_code) % 7;
    return ret;
}

int is_leap_year(int year)
{
    if (year % 4 == 0)
        return 1;
    return 0;
}

/*
 * Initialize RTC
 * */
void rtc_init()
{
    /*
    current_datetime.century = 21;
    current_datetime.year = 16;
    current_datetime.month = 1;
    current_datetime.day = 1;
    current_datetime.hour = 0;
    current_datetime.minute = 0;
    current_datetime.second = 0;
    rtc_write_datetime(&current_datetime);
    rtc_read_datetime();
    */
}
