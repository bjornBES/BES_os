#include "ctype.h"

bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

char toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}

// internal test if char is a digit (0-9)
// @return true if char is a digit
bool isdigit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}