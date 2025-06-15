#include "stdio.h"
#include "SYScalls.h"

void putc(char c)
{
    SYS_Write(((char*)&c), 1);
}

void puts(char* c)
{
    while (*c)
    {
        putc(*c);
        c++;
    }
}