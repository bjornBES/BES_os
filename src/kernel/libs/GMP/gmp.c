#include "libs/GMP/defaultincs.h"

#include "memory.h"
#include "hal/vfs.h"
#include "stdio.h"
#include "stdlib.h"
#include "printfDriver/printf.h"

/* Default allocation functions.  In case of failure to allocate/reallocate
an error message is written to stderr and the program aborts.  */

void *(*gmp_allocate_func)(size_t) = gmp_default_allocate;
void *(*gmp_reallocate_func)(void *, size_t, size_t) = gmp_default_reallocate;
void (*gmp_free_func)(void *, size_t) = gmp_default_free;

void *
gmp_default_allocate(size_t size)
{
    void *ret;
#ifdef DEBUG
    size_t req_size = size;
    size += 2 * GMP_LIMB_BYTES;
#endif
    ret = malloc(size);
    if (ret == 0)
    {
        fprintf(VFS_FD_STDERR, "GNU MP: Cannot allocate memory (size=%lu)\n", (long)size);
        abort();
    }

#ifdef DEBUG
    {
        mp_ptr p = ret;
        p++;
        p[-1] = (0xdeadbeef << 31) + 0xdeafdeed;
        if (req_size % GMP_LIMB_BYTES == 0)
            p[req_size / GMP_LIMB_BYTES] = ~((0xdeadbeef << 31) + 0xdeafdeed);
        ret = p;
    }
#endif
    return ret;
}

void *
gmp_default_reallocate(void *oldptr, size_t old_size, size_t new_size)
{
    void *ret;

#ifdef DEBUG
    size_t req_size = new_size;

    if (old_size != 0)
    {
        mp_ptr p = oldptr;
        if (p[-1] != (0xdeadbeef << 31) + 0xdeafdeed)
        {
            fprintf(VFS_FD_STDERR, "gmp: (realloc) data clobbered before allocation block\n");
            abort();
        }
        if (old_size % GMP_LIMB_BYTES == 0)
            if (p[old_size / GMP_LIMB_BYTES] != ~((0xdeadbeef << 31) + 0xdeafdeed))
            {
                fprintf(VFS_FD_STDERR, "gmp: (realloc) data clobbered after allocation block\n");
                abort();
            }
        oldptr = p - 1;
    }

    new_size += 2 * GMP_LIMB_BYTES;
#endif

    ret = realloc(oldptr, new_size);
    if (ret == 0)
    {
        fprintf(VFS_FD_STDERR, "GNU MP: Cannot reallocate memory (old_size=%lu new_size=%lu)\n", (long)old_size, (long)new_size);
        abort();
    }

#ifdef DEBUG
    {
        mp_ptr p = ret;
        p++;
        p[-1] = (0xdeadbeef << 31) + 0xdeafdeed;
        if (req_size % GMP_LIMB_BYTES == 0)
            p[req_size / GMP_LIMB_BYTES] = ~((0xdeadbeef << 31) + 0xdeafdeed);
        ret = p;
    }
#endif
    return ret;
}

void gmp_default_free(void *blk_ptr, size_t blk_size)
{
#ifdef DEBUG
    {
        mp_ptr p = blk_ptr;
        if (blk_size != 0)
        {
            if (p[-1] != (0xdeadbeef << 31) + 0xdeafdeed)
            {
                fprintf(VFS_FD_STDERR, "gmp: (free) data clobbered before allocation block\n");
                abort();
            }
            if (blk_size % GMP_LIMB_BYTES == 0)
                if (p[blk_size / GMP_LIMB_BYTES] != ~((0xdeadbeef << 31) + 0xdeafdeed))
                {
                    fprintf(VFS_FD_STDERR, "gmp: (free) data clobbered after allocation block\n");
                    abort();
                }
        }
        blk_ptr = p - 1;
    }
#endif
    free(blk_ptr);
}
