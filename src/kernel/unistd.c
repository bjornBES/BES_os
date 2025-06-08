#include "unistd.h"
#include "arch/i686/pit.h"
#include "stdio.h"
#include "debug.h"

#define MODULE "unistd"

void sleepms(uint32_t ms)
{
    sleep_ms(ms);
}
void sleep(uint32_t sec)
{
    sleep_sec(sec);
}

int fdatasync(int fildes)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "fdatasync");
    return -1; // Placeholder for fdatasync function, should be implemented
}

char* getlogin(void)
{
    FUNC_NOT_IMPLEMENTED(MODULE, "realpath");
    return NULL; // Placeholder for realpath function, should be implemented
}

int system(const char* command)
{
    log_debug(MODULE, "system: command = %s", command);
    FUNC_NOT_IMPLEMENTED(MODULE, "system");
    return -1; // Placeholder for system function, should be implemented
}