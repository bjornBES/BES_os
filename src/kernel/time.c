#include "time.h"
#include "drivers/CMOS.h"
#include "malloc.h"

int daylight;
long int timezone;
char *tzname[2];

static const int days_per_month[] = {
    31, 28, 31, 30, 31, 30,
    31, 31, 30, 31, 30, 31};

int days_in_month(int year, int month)
{
    if (month == 2 && is_leap_year(year))
        return 29;
    return days_per_month[month - 1];
}

uint64_t datetime_to_unix(const struct tm *dt)
{
    static const int days_per_month[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

    int year = dt->tm_year;
    int month = dt->tm_mon;
    int day = dt->tm_mday;

    // Days since 1970/01/01
    uint64_t days = 0;

    // Add days from years
    for (int y = 1970; y < year; y++)
    {
        days += is_leap_year(y) ? 366 : 365;
    }

    // Add days from months
    for (int m = 1; m < month; m++)
    {
        days += days_per_month[m - 1];
        if (m == 2 && is_leap_year(year))
        {
            days += 1; // February 29
        }
    }

    // Add days from day of the month (1-based)
    days += day - 1;

    // Convert to seconds
    uint64_t seconds = days * 86400ULL;
    seconds += dt->tm_hour * 3600ULL;
    seconds += dt->tm_min * 60ULL;
    seconds += dt->tm_sec;

    return seconds;
}

void unix_to_datetime(uint64_t unix_time, struct tm *dt) {
    uint64_t seconds = unix_time;

    // Extract time of day
    dt->tm_sec = seconds % 60;
    seconds /= 60;
    dt->tm_min = seconds % 60;
    seconds /= 60;
    dt->tm_hour = seconds % 24;
    uint64_t days = seconds / 24;

    // Calculate year
    int year = 1970;
    while (true) {
        int days_in_year = is_leap_year(year) ? 366 : 365;
        if (days < days_in_year) break;
        days -= days_in_year;
        year++;
    }

    // Calculate month
    int month = 1;
    while (true) {
        int dim = days_in_month(year, month);
        if (days < dim) break;
        days -= dim;
        month++;
    }

    dt->tm_mday = (uint8_t)(days + 1);  // 1-based day
    dt->tm_mon = (uint8_t)month;
    dt->tm_year = year;
}

void localtime(time_t __timer, struct tm *time)
{
    unix_to_datetime(__timer, time);
}

void datetime2tm(datetime* dt, struct tm* tm)
{
    tm->tm_sec = dt->second;
    tm->tm_min = dt->minute;
    tm->tm_hour = dt->hour;
    tm->tm_mday = dt->day;
    tm->tm_mon = dt->month;
    tm->tm_year = dt->year;
}

void time(time_t* timer)
{
    datetime dt;
    rtc_read_datetime(&dt);
    struct tm tm;
    datetime2tm(&dt, &tm);
    *timer = (time_t)datetime_to_unix(&tm);
}

time_t mktime(struct tm* tm)
{
    time_t timer = 0;
    time(&timer);
    unix_to_datetime(timer, tm);
    return timer;
}
