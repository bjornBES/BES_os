
#pragma once
#include <stdint.h>
#include <stdbool.h>

bool islower(char chr);
bool isupper(char chr);
char toupper(char chr);
char tolower(char chr);
bool isdigit(char chr);

bool isalnum(char chr);
bool isalpha(char chr);
bool iscntrl(char chr);
bool isgraph(char chr);
bool isprint(char chr);
bool ispunct(char chr);
bool isspace(char chr);
bool isxdigit(char chr);