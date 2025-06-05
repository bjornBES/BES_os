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

int access(const char* name, int type)
{
    return true;
}

int rmdir(const char* name)
{
    return 0; //?
}
int mkdir(const char* name, int mode)
{
    return 0; 
}
int chdir(const char* name)
{
    return 0; //?
}
char* getcwd(const char* buf, size_t size)
{
    return NULL; //?
}
int unlink(const char* name)
{
    return 0; //?
}