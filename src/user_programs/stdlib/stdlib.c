#include "stdlib.h"
#include "SYScalls.h"

void exit(int status)
{
    SYS_Exit(status);
}