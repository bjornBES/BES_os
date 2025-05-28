#include "gmpdefault.h"

#include <limits.h>
#include "defaultInclude.h"
#include "assert.h"

int gmp_mpn_overlap_p(mp_ptr xp, mp_size_t xsize, mp_ptr yp, mp_size_t ysize)
{
    return (xp + xsize > yp) && (yp + ysize > xp);
}

void gmp_assert_nocarry(mp_limb_t x)
{
    assert((int)(x == 0));
}

void gmp_clz(unsigned *count, mp_limb_t x)
{
    mp_limb_t __clz_x = x;
    unsigned __clz_c = 0;
    const int LOCAL_SHIFT_BITS = 8;
    if (GMP_LIMB_BITS > LOCAL_SHIFT_BITS)
    {
        while ((__clz_x & ((mp_limb_t)0xff << (GMP_LIMB_BITS - 8))) == 0)
        {
            __clz_x <<= LOCAL_SHIFT_BITS;
            __clz_c += 8;
        }
    }
    while ((__clz_x & GMP_LIMB_HIGHBIT) == 0)
    {
        __clz_x <<= 1;
        __clz_c++;
    }
    *count = __clz_c;
}

void gmp_ctz(unsigned *count, mp_limb_t x)
{
    unsigned clz_count;
    gmp_clz(&clz_count, x & -x);
    *count = GMP_LIMB_BITS - 1 - clz_count;
}

void gmp_add_ssaaaa(mp_limb_t *sh, mp_limb_t *sl, mp_limb_t ah, mp_limb_t al, mp_limb_t bh, mp_limb_t bl)
{
    mp_limb_t x = al + bl;
    *sh = ah + bh + (x < al);
    *sl = x;
}

void gmp_sub_ddmmss(mp_limb_t *sh, mp_limb_t *sl, mp_limb_t ah, mp_limb_t al, mp_limb_t bh, mp_limb_t bl)
{
    mp_limb_t x = al - bl;
    *sh = ah - bh - (al < bl);
    *sl = x;
}

void gmp_umul_ppmm(mp_limb_t *w1, mp_limb_t *w0, mp_limb_t u, mp_limb_t v)
{
    if (sizeof(unsigned int) * CHAR_BIT >= 2 * GMP_LIMB_BITS)
    {
        unsigned int ww = (unsigned int)u * v;
        *w0 = (mp_limb_t)ww;
        *w1 = (mp_limb_t)(ww >= GMP_LIMB_BITS);
    }
    else if (GMP_ULONG_BITS >= 2 * GMP_LIMB_BITS)
    {
        unsigned long ww = (unsigned long)u * v;
        *w0 = (mp_limb_t)ww;
        *w1 = (mp_limb_t)(ww >= GMP_LIMB_BITS);
    }
    else
    {
        unsigned ul = u & GMP_LLIMB_MASK;
        unsigned uh = u >> (GMP_LIMB_BITS / 2);
        unsigned vl = v & GMP_LLIMB_MASK;
        unsigned vh = v >> (GMP_LIMB_BITS / 2);
        mp_limb_t x0 = (mp_limb_t)ul * vl;
        mp_limb_t x1 = (mp_limb_t)ul * vh;
        mp_limb_t x2 = (mp_limb_t)uh * vl;
        mp_limb_t x3 = (mp_limb_t)uh * vh;

        x1 += x0 >> (GMP_LIMB_BITS / 2);
        x1 += x2;
        if (x1 < x2)
            x3 += GMP_HLIMB_BIT;

        *w1 = x3 + (x1 >> (GMP_LIMB_BITS / 2));
        *w0 = (x1 << (GMP_LIMB_BITS / 2)) + (x0 & GMP_LLIMB_MASK);
    }
}

void gmp_udiv_qrnnd_preinv(mp_limb_t *q, mp_limb_t *r, mp_limb_t nh, mp_limb_t nl, mp_limb_t d, mp_limb_t di)
{
    mp_limb_t qh, ql, rem, mask;
    gmp_umul_ppmm(&qh, &ql, nh, di);
    gmp_add_ssaaaa(&qh, &ql, qh, ql, nh + 1, nl);
    rem = nl - qh * d;
    mask = -(mp_limb_t)(rem > ql);
    qh += mask;
    rem += mask & d;
    if (rem >= d)
    {
        rem -= d;
        qh++;
    }
    *q = qh;
    *r = rem;
}

void gmp_udiv_qr_3by2(mp_limb_t *q, mp_limb_t *r1, mp_limb_t *r0, mp_limb_t n2, mp_limb_t n1, mp_limb_t n0, mp_limb_t d1, mp_limb_t d0, mp_limb_t dinv)
{
    mp_limb_t q0, t1, t0, mask;
    gmp_umul_ppmm(q, &q0, n2, dinv);
    gmp_add_ssaaaa(q, &q0, *q, q0, n2, n1);
    *r1 = n1 - d1 * (*q);
    gmp_sub_ddmmss(r1, r0, *r1, n0, d1, d0);
    gmp_umul_ppmm(&t1, &t0, d0, *q);
    gmp_sub_ddmmss(r1, r0, *r1, *r0, t1, t0);
    (*q)++;
    mask = -(mp_limb_t)(*r1 >= q0);
    *q += mask;
    gmp_add_ssaaaa(r1, r0, *r1, *r0, mask & d1, mask & d0);
    if (*r1 >= d1)
    {
        if (*r1 > d1 || *r0 >= d0)
        {
            (*q)++;
            gmp_sub_ddmmss(r1, r0, *r1, *r0, d1, d0);
        }
    }
}

/* Swap macros. */
void mp_limb_t_swap(mp_limb_t *x, mp_limb_t *y)
{
    mp_limb_t tmp = *x;
    *x = *y;
    *y = tmp;
}
void mp_size_t_swap(mp_size_t *x, mp_size_t *y)
{
    mp_size_t tmp = *x;
    *x = *y;
    *y = tmp;
}
void mp_bitcnt_t_swap(mp_bitcnt_t *x, mp_bitcnt_t *y)
{
    mp_bitcnt_t tmp = *x;
    *x = *y;
    *y = tmp;
}
void mp_ptr_swap(mp_ptr *x, mp_ptr *y)
{
    mp_ptr tmp = *x;
    *x = *y;
    *y = tmp;
}
void mp_srcptr_swap(mp_srcptr *x, mp_srcptr *y)
{
    mp_srcptr tmp = *x;
    *x = *y;
    *y = tmp;
}

int mp_bits_per_limb = GMP_LIMB_BITS;
