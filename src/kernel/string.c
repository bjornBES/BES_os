#include "string.h"
#include "debug.h"
#include "memory.h"
#include <ctype.h>

const char *strchr(const char *str, char chr)
{
    if (str == NULL)
    {
        return NULL;
    }

    while (*str)
    {
        if (*str == chr)
        {
            return str;
        }

        ++str;
    }

    return NULL;
}
/*

char *strcpy(char *dst, const char *src)
{
char *origDst = dst;

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

uint32_t strlen(const char *str)
{
size_t len = 0;
while (*str)
{
len++;
str++;
}
return len;
}

// internal secure strlen
// @return The length of the string (excluding the terminating 0) limited by 'maxsize'
// @note strlen uses size_t, but wes only use this function with size_t
// variables - hence the signature.
size_t strnlen_s(const char *str, size_t maxsize)
{
const char *s;
for (s = str; *s && maxsize--; ++s)
;
return (size_t)(s - str);
}
*/
int strcmp(const char *a, const char *b)
{
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    while (*a && *b && *a == *b)
    {
        a++;
        b++;
    }
    return (*a) - (*b);
}
int strcoll(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

int strcasecmp(const char *a, const char *b)
{
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    while (tolower(*a) && tolower(*b) && tolower(*a) == tolower(*b))
    {
        a++;
        b++;
    }
    return (*a) - (*b);
}
int strncasecmp(const char *a, const char *b, size_t count)
{
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    while (tolower(*a) && tolower(*b) && tolower(*a) == tolower(*b) && count != 0)
    {
        a++;
        b++;
        count--;
    }
    return (*a) - (*b);
}

int strcmp_debug(const char *a, const char *b)
{

    log_debug("strcmp", "a = %p b = %p", a, b);
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL || b == NULL)
        return -1;

    log_debug("strcmp", "a = %s a = %p", a, a);
    log_debug("strcmp", "b = %s b = %p", b, b);
    while (*a && *b && *a == *b)
    {
        a++;
        b++;
    }
    return (*a) - (*b);
}

// strncmp("Hello", "HellW ", 4)

/*
char *strcat(char *dest, const char *src)
{
char *end = dest + strlen(dest);
memcpy(end, src, strlen(src));
end = end + strlen(src);
*end = '\0';
return dest;
}
*/

void itoa(char *buf, uint32_t n, int base)
{
    uint32_t tmp;
    uint32_t i, j;

    tmp = n;
    i = 0;

    do
    {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; j++, i--)
    {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}
void atoi(char *str, int *a)
{
    int k = 0;
    while (*str)
    {
        k = (k << 3) + (k << 1) + (*str) - '0';
        str++;
    }
    *a = k;
}

// internal ASCII string to size_t conversion
size_t atou(const char *str)
{
    size_t i = 0U;
    while (isdigit(*str))
    {
        i = i * 10U + (size_t)(*((str)) - '0');
        str++;
    }
    return i;
}
// internal ASCII string to size_t conversion
const char *atou_return(const char *str, size_t *result)
{
    size_t i = 0U;
    while (isdigit(*str))
    {
        i = i * 10U + (size_t)(*((str)) - '0');
        str++;
    }
    *result = i;
    return str;
}

uint32_t strcrl(string str, const char what, const char with)
{
    uint32_t i = 0;
    while (str[i] != 0)
    {
        if (str[i] == what)
            str[i] = with;
        i++;
    }
    return i;
}

char *strrchr(const char *s, int c)
{ /* find last occurrence of c in char s[] */
    const char ch = c;
    const char *sc;

    for (sc = NULL;; ++s)
    { /* check another char */
        if (*s == ch)
            sc = s;
        if (*s == '\0')
            return ((char *)sc);
    }
}

uint32_t strcount(string str, char c)
{
    uint32_t count = 0;
    uint16_t len = strlen(str);
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == c)
        {
            count++;
        }
    }
    return count;
}

size_t strspn(const char *s1, const char *s2)
{ /* find index of first s1[i] that matches no s2[] */
    const char *sc1, *sc2;

    for (sc1 = s1; *sc1 != '\0'; ++sc1)
        for (sc2 = s2;; ++sc2)
            if (*sc2 == '\0')
                return (sc1 - s1);
            else if (*sc1 == *sc2)
                break;
    return (sc1 - s1); /* null doesn't match */
}

size_t strcspn(const char *s1, const char *s2)
{ /* find index of first s1[i] that matches any s2[] */
    const char *sc1, *sc2;

    for (sc1 = s1; *sc1 != '\0'; ++sc1)
        for (sc2 = s2; *sc2 != '\0'; ++sc2)
            if (*sc1 == *sc2)
                return (sc1 - s1);
    return (sc1 - s1); /* terminating nulls match */
}

uint32_t str_backspace(string str, char c)
{
    uint32_t i = strlen(str);
    i--;
    while (i)
    {
        i--;
        if (str[i] == c)
        {
            str[i + 1] = 0;
            return 1;
        }
    }
    return 0;
}

static char *strtok_ptr = NULL; // Pointer to the current position in the string

char *strtok(char *str, const char *delim)
{ /* find next token in s1[] delimited by s2[] */
    char *sbegin, *send;
    static char *ssave = ""; /* for safety */

    sbegin = str ? str : ssave;
    sbegin += strspn(sbegin, delim);
    if (*sbegin == '\0')
    {               /* end of scan */
        ssave = ""; /* for safety */
        return (NULL);
    }
    send = sbegin + strcspn(sbegin, delim);
    if (*send != '\0')
        *send++ = '\0';
    ssave = send;
    return (sbegin);
}

uint32_t str_begins_with(string str, string with)
{
    uint32_t j = strlen(with);
    uint32_t i = 0;
    uint32_t ret = 1;
    while (with[j] != 0)
    {
        if (str[i] != with[i])
        {
            ret = 0;
            break;
        }
        j--;
        i++;
    }
    return ret;
}

uint16_t *utf16_to_codepoint(uint16_t *string, int *codepoint)
{
    int c1 = *string;
    ++string;

    if (c1 >= 0xd800 && c1 < 0xdc00)
    {
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

char *codepoint_to_utf8(int codepoint, char *stringOutput)
{
    if (codepoint <= 0x7F)
    {
        *stringOutput = (char)codepoint;
        stringOutput++;
    }
    else if (codepoint <= 0x7FF)
    {
        *stringOutput = 0xC0 | ((codepoint >> 6) & 0x1F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    else if (codepoint <= 0xFFFF)
    {
        *stringOutput = 0xE0 | ((codepoint >> 12) & 0xF);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 6) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    else if (codepoint <= 0x1FFFFF)
    {
        *stringOutput = 0xF0 | ((codepoint >> 18) & 0x7);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 12) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | ((codepoint >> 6) & 0x3F);
        stringOutput++;
        *stringOutput = 0x80 | (codepoint & 0x3F);
        stringOutput++;
    }
    return stringOutput;
}
