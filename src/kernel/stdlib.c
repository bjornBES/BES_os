#include "stdlib.h"
#include <stdint.h>
#include "string.h"
#include "memory.h"
#include "stdio.h"
#include "array_size.h"

char *__env[31];
static const int __nenv = ARRAY_SIZE(__env);

void qsort_internal(void *base,
                    size_t num,
                    size_t size,
                    size_t left,
                    size_t right,
                    int (*compar)(const void *, const void *))
{
    if (left >= right)
        return;

    int i = left, j = right;
    void *pivot = base + (i * size);
    uint8_t temp;

    for (;;)
    {
        while ((*compar)(base + (i * size), pivot) < 0)
            i++;
        while ((*compar)(pivot, base + (j * size)) < 0)
            j--;
        if (i >= j)
            break;

        // swap
        for (int k = 0; k < size; k++)
        {
            temp = *((uint8_t *)(base + (i * size)) + k);
            *((uint8_t *)(base + (i * size)) + k) = *((uint8_t *)(base + (j * size)) + k);
            *((uint8_t *)(base + (j * size)) + k) = temp;
        }
        i++;
        j--;
    }

    qsort_internal(base, num, size, left, i - 1, compar);
    qsort_internal(base, num, size, j + 1, right, compar);
}

void qsort(void *base,
           size_t num,
           size_t size,
           int (*compar)(const void *, const void *))
{
    qsort_internal(base, num, size, 0, num - 1, compar);
}

void exit(int __status)
{
}

void abort()
{
}

char *getenv(const char *match)
{
    char **ep = __env;
    int matchlen = strlen(match);
    int i;

    for (i = 0; i < __nenv; i++)
    {
        char *e = *ep++;

        if (!e)
            continue;

        if ((strncmp(match, e, matchlen) == 0) && ((e[matchlen] == '\0') || (e[matchlen] == '=')))
        {
            char *cp = (char *)strchr(e, '=');
            return cp ? ++cp : "";
        }
    }
    return NULL;
}

int setenv(const char *var, const char *val)
{
    int i;
    char *ep;
    size_t varlen, vallen;

    varlen = strlen(var);
    vallen = strlen(val);
    ep = malloc(varlen + vallen + 2);

    if (!ep)
    {
        return false;
    }

    sprintf(ep, "%s=%s", var, val);

    for (i = 0; i < __nenv; i++)
    {
        if (__env[i] && ((strncmp(__env[i], var, varlen) == 0) && ((__env[i][varlen] == '\0') || (__env[i][varlen] == '='))))
        {
            free(__env[i]);
            __env[i] = ep;
            return 0;
        }
    }
    /*
     * Wasn't existing variable.  Fit into slot.
     */
    for (i = 0; i < __nenv - 1; i++)
    {
        if (__env[i] == (char *)0)
        {
            __env[i] = ep;
            return 0;
        }
    }

    return true;
}

int setenvOW(const char* var, const char* val, int overwrite)
{
    int i;
    char *ep;
    size_t varlen, vallen;

    varlen = strlen(var);
    vallen = strlen(val);
    ep = malloc(varlen + vallen + 2);

    if (!ep)
    {
        return false;
    }

    sprintf(ep, "%s=%s", var, val);

    for (i = 0; i < __nenv; i++)
    {
        if (__env[i] && ((strncmp(__env[i], var, varlen) == 0) && ((__env[i][varlen] == '\0') || (__env[i][varlen] == '='))))
        {
            if (overwrite)
            {
                free(__env[i]);
                __env[i] = ep;
            }
            else
            {
                free(ep);
            }
            return 0;
        }
    }
    /*
     * Wasn't existing variable.  Fit into slot.
     */
    for (i = 0; i < __nenv - 1; i++)
    {
        if (__env[i] == (char *)0)
        {
            __env[i] = ep;
            return 0;
        }
    }

    return true;
}

int unsetenv(const char *var)
{
    char **ep = __env;
    int matchlen = strlen(var);
    int i;

    for (i = 0; i < __nenv; i++)
    {
        char *e = *ep++;

        if (!e)
            continue;

        if ((strncmp(var, e, matchlen) == 0) && ((e[matchlen] == '\0') || (e[matchlen] == '=')))
        {
            free(__env[i]);
        }
    }
    return 0;
}

int putenv(const char *var)
{
    char **ep = __env;
    int matchlen = strlen(var);
    int i;

    for (i = 0; i < __nenv; i++)
    {
        char *e = *ep++;

        if (!e)
            continue;
        if ((strncmp(var, e, matchlen) == 0) && ((e[matchlen] == '\0') || (e[matchlen] == '=')))
        {
            char *cp = (char *)strchr(e, '=');
            __env[i] = cp;
            return 1;
        }
    }
    return 0;
}

/*
 * printenv() - Display the current environment variables.
 */
void printenv(void)
{
    int i;

    for (i = 0; i < __nenv; i++)
    {
        if (__env[i])
            fprintf(3, "%s\n", __env[i]);
    }
}