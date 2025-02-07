#include "string.h"
#include <stdint.h>
#include <stddef.h>

const char* strchr(const char* str, char chr)
{
    if (str == NULL)
        return NULL;

    while (*str)
    {
        if (*str == chr)
            return str;

        ++str;
    }

    return NULL;
}

char* strcpy(char* dst, const char* src)
{
    char* origDst = dst;

    if (dst == NULL)
        return NULL;

    if (src == NULL)
    {
        *dst = '\0';
        return dst;
    }

    while (*src)
    {
        *dst = *src;
        ++src;
        ++dst;
    }
    
    *dst = '\0';
    return origDst;
}

unsigned strlen(const char* str)
{
    unsigned len = 0;
    while (*str)
    {
        ++len;
        ++str;
    }

    return len;
}

int strcmp(const char* a, const char* b)
{
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    while (*a && *b && *a == *b)
    {
        ++a;
        ++b;
    }
    return (*a) - (*b);
}

void atoi(char *str, int* a)
{
	int k = 0;
	while(*str)
	{
		k = (k<<3)+(k<<1)+(*str)-'0';
		str++;
	}
	*a = k;
}

size_t strcrl(string str, const char what, const char with)
{
	size_t i = 0;
	while(str[i] != 0)
	{
		if(str[i] == what) str[i] = with;
		i++;
	}
	return i;
}

size_t strcount(string str, char c)
{
	size_t i = 0;
	while(*str--)
		if(*str == c) i++;
	return i;
}

size_t str_backspace(string str, char c)
{
	size_t i = strlen(str);
	i--;
	while(i)
	{
		i--;
		if(str[i] == c)
		{
			str[i+1] = 0;
			return 1;
		}
	}
	return 0;
}

size_t strsplit(string str, char delim)
{
	size_t n = 0;
	uint32_t i = 0;
	while(str[i])
	{
		if(str[i] == delim)
		{
			str[i] = 0;
			n++;
		}
		i++;
	}
	n++;
	return n;
}

size_t str_begins_with(const string str, const string with)
{
	size_t j = strlen(with);
	size_t i = 0;
	size_t ret = 1;
	while(with[j] != 0)
	{
		if(str[i] != with[i]) { ret = 0; break; }
		j--;
		i++;
	}
	return ret;
}

wchar_t* utf16_to_codepoint(wchar_t* string, int* codepoint)
{
    int c1 = *string;
    ++string;

    if (c1 >= 0xd800 && c1 < 0xdc00) {
        int c2 = *string;
        ++string;
        *codepoint = ((c1 & 0x3ff) << 10) + (c2 & 0x3ff) + 0x10000;
    }
    *codepoint = c1;

    return string;
}

/* Encoding
   The following byte sequences are used to represent a
   character.  The sequence to be used depends on the UCS code
   number of the character:

   0x00000000 - 0x0000007F:
       0xxxxxxx

   0x00000080 - 0x000007FF:
       110xxxxx 10xxxxxx

   0x00000800 - 0x0000FFFF:
       1110xxxx 10xxxxxx 10xxxxxx

   0x00010000 - 0x001FFFFF:
       11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   [... removed obsolete five and six byte forms ...]

   The xxx bit positions are filled with the bits of the
   character code number in binary representation.  Only the
   shortest possible multibyte sequence which can represent the
   code number of the character can be used.

   The UCS code values 0xd800–0xdfff (UTF-16 surrogates) as well
   as 0xfffe and 0xffff (UCS noncharacters) should not appear in
   conforming UTF-8 streams.
*/

char* codepoint_to_utf8(int codepoint, char* stringOutput)
{
    if (codepoint <= 0x7F) {
        *stringOutput = (char)codepoint;
    }
    else if (codepoint <= 0x7FF) {
        *stringOutput++ = 0xC0 | ((codepoint >> 6) & 0x1F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint <= 0xFFFF) {
        *stringOutput++ = 0xE0 | ((codepoint >> 12) & 0xF);
        *stringOutput++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    else if (codepoint <= 0x1FFFFF) {
        *stringOutput++ = 0xF0 | ((codepoint >> 18) & 0x7);
        *stringOutput++ = 0x80 | ((codepoint >> 12) & 0x3F);
        *stringOutput++ = 0x80 | ((codepoint >> 6) & 0x3F);
        *stringOutput++ = 0x80 | (codepoint & 0x3F);
    }
    return stringOutput;
}
