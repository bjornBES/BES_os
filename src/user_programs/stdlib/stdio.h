#pragma once

#ifndef NULL
#define NULL ((void*)0)
#endif

#define stdout (int)0
#define stdin (int)1
#define stderr (int)2

void putc(char c);
void puts(char* c);

#define getchar() getc()
int getc();
int gets(char* buf);