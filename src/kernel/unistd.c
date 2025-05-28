#include "unistd.h"
#include "arch/i686/pit.h"

void sleepms(uint32_t ms)
{
    sleep_ms(ms);
}
void sleep(uint32_t sec)
{
    sleep_sec(sec);
}