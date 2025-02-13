#include "memory.h"

void* memcpy(void* dst, const void* src, uint16_t num)
{
    uint8_t* u8Dst = (uint8_t *)dst;
    const uint8_t* u8Src = (const uint8_t *)src;

    for (uint16_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void * memset(void * ptr, int value, uint32_t num)
{
    uint8_t* u8Ptr = (uint8_t *)ptr;

    for (uint32_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}

void *memmove(void *destination, const void *source, uint32_t num)
{
    uint8_t *dest = (uint8_t *)destination;
    const uint8_t *src = (const uint8_t *)source;

    if ((uint32_t)dest > (uint32_t)src && (uint32_t)dest < ((uint32_t)src + num))
    {
        for (uint32_t i = num; i > 0; i--)
        {
            dest[i - 1] = src[i - 1];
        }
    }
    else
    {
        for (uint32_t i = 0; i < num; i++)
        {
            dest[i] = src[i];
        }
    }

    return destination;
}
