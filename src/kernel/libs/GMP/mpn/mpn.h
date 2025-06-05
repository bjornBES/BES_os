#pragma once
#include "libs/GMP/gmp-impl.h"

#if defined(_CRAY)
#define MPN_COPY_INCR(dst, src, n)                         \
    do                                                     \
    {                                                      \
        int __i; /* Faster on some Crays with plain int */ \
        _Pragma("_CRI ivdep");                             \
        for (__i = 0; __i < (n); __i++)                    \
            (dst)[__i] = (src)[__i];                       \
    } while (0)
#endif

#if !defined(MPN_COPY_INCR)
#define MPN_COPY_INCR(dst, src, size)               \
    do                                              \
    {                                               \
        ASSERT((size) >= 0);                        \
        ASSERT(MPN_SAME_OR_INCR_P(dst, src, size)); \
        mpn_copyi(dst, src, size);                  \
    } while (0)
#endif

#if !defined(MPN_COPY_DECR)
#define MPN_COPY_DECR(dst, src, size)               \
    do                                              \
    {                                               \
        ASSERT((size) >= 0);                        \
        ASSERT(MPN_SAME_OR_DECR_P(dst, src, size)); \
        mpn_copyd(dst, src, size);                  \
    } while (0)
#endif

#ifndef MPN_COPY
#define MPN_COPY(d, s, n)                        \
    do                                           \
    {                                            \
        ASSERT(MPN_SAME_OR_SEPARATE_P(d, s, n)); \
        MPN_COPY_INCR(d, s, n);                  \
    } while (0)
#endif

/* Set {dst,size} to the limbs of {src,size} in reverse order. */
#define MPN_REVERSE(dst, src, size)                   \
    do                                                \
    {                                                 \
        mp_ptr __dst = (dst);                         \
        mp_size_t __size = (size);                    \
        mp_srcptr __src = (src) + __size - 1;         \
        mp_size_t __i;                                \
        ASSERT((size) >= 0);                          \
        ASSERT(!MPN_OVERLAP_P(dst, size, src, size)); \
        CRAY_Pragma("_CRI ivdep");                    \
        for (__i = 0; __i < __size; __i++)            \
        {                                             \
            *__dst = *__src;                          \
            __dst++;                                  \
            __src--;                                  \
        }                                             \
    } while (0)

#if HAVE_HOST_CPU_FAMILY_power || HAVE_HOST_CPU_FAMILY_powerpc
#define MPN_FILL(dst, n, f)       \
    do                            \
    {                             \
        mp_ptr __dst = (dst) - 1; \
        mp_size_t __n = (n);      \
        ASSERT(__n > 0);          \
        do                        \
            *++__dst = (f);       \
        while (--__n);            \
    } while (0)
#endif

#ifndef MPN_FILL
#define MPN_FILL(dst, n, f)   \
    do                        \
    {                         \
        mp_ptr __dst = (dst); \
        mp_size_t __n = (n);  \
        ASSERT(__n > 0);      \
        do                    \
            *__dst++ = (f);   \
        while (--__n);        \
    } while (0)
#endif

#define MPN_ZERO(dst, n)                    \
    do                                      \
    {                                       \
        ASSERT((n) >= 0);                   \
        if ((n) != 0)                       \
            MPN_FILL(dst, n, CNST_LIMB(0)); \
    } while (0)

#ifndef MPN_NORMALIZE
#define MPN_NORMALIZE(DST, NLIMBS)        \
    do                                    \
    {                                     \
        while ((NLIMBS) > 0)              \
        {                                 \
            if ((DST)[(NLIMBS) - 1] != 0) \
                break;                    \
            (NLIMBS)--;                   \
        }                                 \
    } while (0)
#endif
#ifndef MPN_NORMALIZE_NOT_ZERO
#define MPN_NORMALIZE_NOT_ZERO(DST, NLIMBS) \
    do                                      \
    {                                       \
        while (1)                           \
        {                                   \
            ASSERT((NLIMBS) >= 1);          \
            if ((DST)[(NLIMBS) - 1] != 0)   \
                break;                      \
            (NLIMBS)--;                     \
        }                                   \
    } while (0)
#endif

/* Strip least significant zero limbs from {ptr,size} by incrementing ptr
   and decrementing size.  low should be ptr[0], and will be the new ptr[0]
   on returning.  The number in {ptr,size} must be non-zero, ie. size!=0 and
   somewhere a non-zero limb.  */
#define MPN_STRIP_LOW_ZEROS_NOT_ZERO(ptr, size, low) \
    do                                               \
    {                                                \
        ASSERT((size) >= 1);                         \
        ASSERT((low) == (ptr)[0]);                   \
                                                     \
        while ((low) == 0)                           \
        {                                            \
            (size)--;                                \
            ASSERT((size) >= 1);                     \
            (ptr)++;                                 \
            (low) = *(ptr);                          \
        }                                            \
    } while (0)

#define MPN_FIB2_SIZE(n) \
    ((mp_size_t)((n) / 32 * 23 / GMP_NUMB_BITS) + 4)

/* Return non-zero if xp,xsize and yp,ysize overlap.
   If xp+xsize<=yp there's no overlap, or if yp+ysize<=xp there's no
   overlap.  If both these are false, there's an overlap. */
#define MPN_OVERLAP_P(xp, xsize, yp, ysize) \
    ((xp) + (xsize) > (yp) && (yp) + (ysize) > (xp))
#define MEM_OVERLAP_P(xp, xsize, yp, ysize) \
    ((char *)(xp) + (xsize) > (char *)(yp) && (char *)(yp) + (ysize) > (char *)(xp))

/* Return non-zero if xp,xsize and yp,ysize are either identical or not
   overlapping.  Return zero if they're partially overlapping. */
#define MPN_SAME_OR_SEPARATE_P(xp, yp, size) \
    MPN_SAME_OR_SEPARATE2_P(xp, size, yp, size)
#define MPN_SAME_OR_SEPARATE2_P(xp, xsize, yp, ysize) \
    ((xp) == (yp) || !MPN_OVERLAP_P(xp, xsize, yp, ysize))

/* Return non-zero if dst,dsize and src,ssize are either identical or
   overlapping in a way suitable for an incrementing/decrementing algorithm.
   Return zero if they're partially overlapping in an unsuitable fashion. */
#define MPN_SAME_OR_INCR2_P(dst, dsize, src, ssize) \
    ((dst) <= (src) || !MPN_OVERLAP_P(dst, dsize, src, ssize))
#define MPN_SAME_OR_INCR_P(dst, src, size) \
    MPN_SAME_OR_INCR2_P(dst, size, src, size)
#define MPN_SAME_OR_DECR2_P(dst, dsize, src, ssize) \
    ((dst) >= (src) || !MPN_OVERLAP_P(dst, dsize, src, ssize))
#define MPN_SAME_OR_DECR_P(dst, src, size) \
    MPN_SAME_OR_DECR2_P(dst, size, src, size)

#if ! HAVE_NATIVE_mpn_com
#undef mpn_com
#define mpn_com(d,s,n)							\
  do {									\
    mp_ptr     __d = (d);						\
    mp_srcptr  __s = (s);						\
    mp_size_t  __n = (n);						\
    ASSERT (__n >= 1);							\
    ASSERT (MPN_SAME_OR_SEPARATE_P (__d, __s, __n));			\
    do									\
      *__d++ = (~ *__s++) & GMP_NUMB_MASK;				\
    while (--__n);							\
  } while (0)
#endif


#if ! HAVE_NATIVE_mpn_and_n
#undef mpn_and_n
#define mpn_and_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, __a & __b)
#endif

#if ! HAVE_NATIVE_mpn_andn_n
#undef mpn_andn_n
#define mpn_andn_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, __a & ~__b)
#endif

#if ! HAVE_NATIVE_mpn_nand_n
#undef mpn_nand_n
#define mpn_nand_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, ~(__a & __b) & GMP_NUMB_MASK)
#endif

#if ! HAVE_NATIVE_mpn_ior_n
#undef mpn_ior_n
#define mpn_ior_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, __a | __b)
#endif

#if ! HAVE_NATIVE_mpn_iorn_n
#undef mpn_iorn_n
#define mpn_iorn_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, (__a | ~__b) & GMP_NUMB_MASK)
#endif

#if ! HAVE_NATIVE_mpn_nior_n
#undef mpn_nior_n
#define mpn_nior_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, ~(__a | __b) & GMP_NUMB_MASK)
#endif

#if ! HAVE_NATIVE_mpn_xor_n
#undef mpn_xor_n
#define mpn_xor_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, __a ^ __b)
#endif

#if ! HAVE_NATIVE_mpn_xnor_n
#undef mpn_xnor_n
#define mpn_xnor_n(rp, up, vp, n) \
  MPN_LOGOPS_N_INLINE (rp, up, vp, n, ~(__a ^ __b) & GMP_NUMB_MASK)
#endif
