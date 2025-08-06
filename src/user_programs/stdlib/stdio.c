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

int getc()
{
    char c = '\0';
    while (c == '\0')
    {
        SYS_Read(&c, 1);
    }
    return c;
}

int gets(char* buf)
{
    char* result = &(*buf);
    int count = 0;
    while (*buf)
    {
        *buf = (char)getc();
        if (*buf == '\n')
        {
            break;
        }
        buf++;
        count++;
    }
    return result;
}