/* mini-gmp, a minimalistic implementation of a GNU GMP subset.

   Contributed to the GNU project by Niels Möller

Copyright 1991-1997, 1999-2019 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the GNU MP Library.  If not,
see https://www.gnu.org/licenses/.  */

/* NOTE: All functions in this file which are not declared in
   mini-gmp.h are internal, and are not intended to be compatible
   with GMP or with future versions of mini-gmp. */

/* Much of the material copied from GMP files, including: gmp-impl.h,
   longlong.h, mpn/generic/add_n.c, mpn/generic/addmul_1.c,
   mpn/generic/lshift.c, mpn/generic/mul_1.c,
   mpn/generic/mul_basecase.c, mpn/generic/rshift.c,
   mpn/generic/sbpi1_div_qr.c, mpn/generic/sub_n.c,
   mpn/generic/submul_1.c. */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "libs/GMP/longlong.h"

#include "mini-gmp.h"
#include "libs/GMP/gmp-impl.h"
#include "libs/GMP/gmpdefault.h"

#if !defined(MINI_GMP_DONT_USE_FLOAT_H)
#include <float.h>
#endif

/* Memory allocation and other helper functions. */
void gmp_die(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    abort();
}

mp_ptr gmp_xalloc_limbs(mp_size_t size)
{
    return (mp_ptr)gmp_xalloc(size * sizeof(mp_limb_t));
}

mp_ptr gmp_xrealloc_limbs(mp_ptr old, mp_size_t size)
{
    assert(size > 0);
    return (mp_ptr)(*gmp_default_reallocate)(old, 0, size * sizeof(mp_limb_t));
}

/* MPN interface */

/*
mp_size_t mpn_normalized_size(mp_srcptr xp, mp_size_t n)
{
    while (n > 0 && xp[n - 1] == 0)
    --n;
    return n;
}
*/

mp_bitcnt_t mpn_common_scan(mp_limb_t limb, mp_size_t i, mp_srcptr up, mp_size_t un, mp_limb_t ux)
{
    unsigned int cnt = 0;

    assert(ux == 0 || ux == GMP_LIMB_MAX);
    assert(0 <= i && i <= un);

    while (limb == 0)
    {
        i++;
        if (i == un)
            return (ux == 0 ? ~(mp_bitcnt_t)0 : un * GMP_LIMB_BITS);
        limb = ux ^ up[i];
    }
    gmp_ctz(&cnt, limb);
    return (mp_bitcnt_t)i * GMP_LIMB_BITS + cnt;
}
/*

mp_limb_t mpn_invert_3by2(mp_limb_t u1, mp_limb_t u0)
{
mp_limb_t r = 0, m = 0;

{
    mp_limb_t p, ql;
    unsigned ul, uh, qh;

    // For notation, let b denote the half-limb base, so that B = b^2. Split u1 = b uh + ul.
    ul = u1 & GMP_LLIMB_MASK;
    uh = u1 >> (GMP_LIMB_BITS / 2);

    // Approximation of the high half of quotient. Differs from the 2/1
    // inverse of the half limb uh, since we have already subtracted u0.
    qh = (u1 ^ GMP_LIMB_MAX) / uh;

    //Adjust to get a half-limb 3/2 inverse, i.e., we want
    //qh' = floor( (b^3 - 1) / u) - b = floor ((b^3 - b u - 1) / u
    //= floor( (b (~u) + b-1) / u),
    //and the remainder
    //r = b (~u) + b-1 - qh (b uh + ul) = b (~u - qh uh) + b-1 - qh ul
    //Subtraction of qh ul may underflow, which implies adjustments.
    //But by normalization, 2 u >= B > qh ul, so we need to adjust by
    //at most 2.
    //

    r = ((~u1 - (mp_limb_t)qh * uh) << (GMP_LIMB_BITS / 2)) | GMP_LLIMB_MASK;

    p = (mp_limb_t)qh * ul;
    // Adjustment steps taken from udiv_qrnnd_c
    if (r < p)
    {
        qh--;
        r += u1;
        if (r >= u1) // i.e. we didn't get carry when adding to r
        if (r < p)
        {
            qh--;
            r += u1;
        }
    }
    r -= p;

    // Low half of the quotient is

    // ql = floor ( (b r + b-1) / u1).

    // This is a 3/2 division (on half-limbs), for which qh is a
    // suitable inverse.

    p = (r >> (GMP_LIMB_BITS / 2)) * qh + r;
    // Unlike full-limb 3/2, we can add 1 without overflow. For this to
    // work, it is essential that ql is a full mp_limb_t.
    ql = (p >> (GMP_LIMB_BITS / 2)) + 1;

    // By the 3/2 trick, we don't need the high half limb.
    r = (r << (GMP_LIMB_BITS / 2)) + GMP_LLIMB_MASK - ql * u1;

    if (r >= (GMP_LIMB_MAX & (p << (GMP_LIMB_BITS / 2))))
    {
        ql--;
        r += u1;
    }
    m = ((mp_limb_t)qh << (GMP_LIMB_BITS / 2)) + ql;
    if (r >= u1)
    {
        m++;
        r -= u1;
    }
}

// Now m is the 2/1 inverse of u1. If u0 > 0, adjust it to become a 3/2 inverse.
if (u0 > 0)
{
    mp_limb_t th = 0, tl = 0;
    r = ~r;
    r += u0;
    if (r < u0)
    {
        m--;
        if (r >= u1)
        {
            m--;
            r -= u1;
        }
        r -= u1;
    }
    gmp_umul_ppmm(&th, &tl, u0, m);
    r += th;
    if (r < th)
    {
        m--;
        m -= ((r > u1) | ((r == u1) & (tl > u0)));
    }
}

return m;
}
*/

void mpn_div_qr_1_invert(struct gmp_div_inverse *inv, mp_limb_t d)
{
    unsigned shift = 0;

    assert(d > 0);
    gmp_clz(&shift, d);
    inv->shift = shift;
    inv->d1 = d << shift;
    inv->di = mpn_invert_limb(inv->d1);
}

void mpn_div_qr_2_invert(struct gmp_div_inverse *inv, mp_limb_t d1, mp_limb_t d0)
{
    unsigned shift = 0;

    assert(d1 > 0);
    gmp_clz(&shift, d1);
    inv->shift = shift;
    if (shift > 0)
    {
        d1 = (d1 << shift) | (d0 >> (GMP_LIMB_BITS - shift));
        d0 <<= shift;
    }
    inv->d1 = d1;
    inv->d0 = d0;
    inv->di = mpn_invert_3by2(d1, d0);
}

void mpn_div_qr_invert(struct gmp_div_inverse *inv, mp_srcptr dp, mp_size_t dn)
{
    assert(dn > 0);

    if (dn == 1)
        mpn_div_qr_1_invert(inv, dp[0]);
    else if (dn == 2)
        mpn_div_qr_2_invert(inv, dp[1], dp[0]);
    else
    {
        unsigned shift = 0;
        mp_limb_t d1 = 0, d0 = 0;

        d1 = dp[dn - 1];
        d0 = dp[dn - 2];
        assert(d1 > 0);
        gmp_clz(&shift, d1);
        inv->shift = shift;
        if (shift > 0)
        {
            d1 = (d1 << shift) | (d0 >> (GMP_LIMB_BITS - shift));
            d0 = (d0 << shift) | (dp[dn - 3] >> (GMP_LIMB_BITS - shift));
        }
        inv->d1 = d1;
        inv->d0 = d0;
        inv->di = mpn_invert_3by2(d1, d0);
    }
}

mp_limb_t mpn_div_qr_1_preinv(mp_ptr qp, mp_srcptr np, mp_size_t nn, const struct gmp_div_inverse *inv)
{
    mp_limb_t d = 0;
    mp_limb_t di = 0;
    mp_limb_t r = 0;
    mp_ptr tp = NULL;

    if (inv->shift > 0)
    {
        /* Shift, reusing qp area if possible. In-place shift if qp == np. */
        tp = qp ? qp : gmp_xalloc_limbs(nn);
        r = mpn_lshift(tp, np, nn, inv->shift);
        np = tp;
    }
    else
        r = 0;

    d = inv->d1;
    di = inv->di;
    while (--nn >= 0)
    {
        mp_limb_t q = 0;

        gmp_udiv_qrnnd_preinv(&q, &r, r, np[nn], d, di);
        if (qp)
            qp[nn] = q;
    }
    if ((inv->shift > 0) && (tp != qp))
        gmp_free(tp);

    return r >> inv->shift;
}

void mpn_div_qr_2_preinv(mp_ptr qp, mp_ptr np, mp_size_t nn, const struct gmp_div_inverse *inv)
{
    unsigned shift = 0;
    mp_size_t i = 0;
    mp_limb_t d1 = 0;
    mp_limb_t d0 = 0;
    mp_limb_t di = 0;
    mp_limb_t r1 = 0;
    mp_limb_t r0 = 0;

    assert(nn >= 2);
    shift = inv->shift;
    d1 = inv->d1;
    d0 = inv->d0;
    di = inv->di;

    if (shift > 0)
        r1 = mpn_lshift(np, np, nn, shift);
    else
        r1 = 0;

    r0 = np[nn - 1];

    i = nn - 2;
    do
    {
        mp_limb_t n0 = 0;
        mp_limb_t q = 0;
        n0 = np[i];
        gmp_udiv_qr_3by2(&q, &r1, &r0, r1, r0, n0, d1, d0, di);

        if (qp)
            qp[i] = q;
    } while (--i >= 0);

    if (shift > 0)
    {
        assert((r0 & (GMP_LIMB_MAX >> (GMP_LIMB_BITS - shift))) == 0);
        r0 = (r0 >> shift) | (r1 << (GMP_LIMB_BITS - shift));
        r1 >>= shift;
    }

    np[1] = r1;
    np[0] = r0;
}

void mpn_div_qr_pi1(mp_ptr qp, mp_ptr np, mp_size_t nn, mp_limb_t n1, mp_srcptr dp, mp_size_t dn, mp_limb_t dinv)
{
    mp_size_t i;

    mp_limb_t d1, d0;
    mp_limb_t cy, cy1;
    mp_limb_t q;

    assert(dn > 2);
    assert(nn >= dn);

    d1 = dp[dn - 1];
    d0 = dp[dn - 2];

    assert((d1 & GMP_LIMB_HIGHBIT) != 0);
    /* Iteration variable is the index of the q limb.
     *
     * We divide <n1, np[dn-1+i], np[dn-2+i], np[dn-3+i],..., np[i]>
     * by            <d1,          d0,        dp[dn-3],  ..., dp[0] >
     */

    i = nn - dn;
    do
    {
        mp_limb_t n0 = np[dn - 1 + i];

        if (n1 == d1 && n0 == d0)
        {
            q = GMP_LIMB_MAX;
            mpn_submul_1(np + i, dp, dn, q);
            n1 = np[dn - 1 + i]; /* update n1, last loop's value will now be invalid */
        }
        else
        {
            gmp_udiv_qr_3by2(&q, &n1, &n0, n1, n0, np[dn - 2 + i], d1, d0, dinv);

            cy = mpn_submul_1(np + i, dp, dn - 2, q);

            cy1 = n0 < cy;
            n0 = n0 - cy;
            cy = n1 < cy1;
            n1 = n1 - cy1;
            np[dn - 2 + i] = n0;

            if (cy != 0)
            {
                n1 += d1 + mpn_add_n(np + i, np + i, dp, dn - 1);
                q--;
            }
        }

        if (qp)
            qp[i] = q;
    } while (--i >= 0);

    np[dn - 1] = n1;
}

void mpn_div_qr_preinv(mp_ptr qp, mp_ptr np, mp_size_t nn, mp_srcptr dp, mp_size_t dn, const struct gmp_div_inverse *inv)
{
    assert(dn > 0);
    assert(nn >= dn);

    if (dn == 1)
        np[0] = mpn_div_qr_1_preinv(qp, np, nn, inv);
    else if (dn == 2)
        mpn_div_qr_2_preinv(qp, np, nn, inv);
    else
    {
        mp_limb_t nh;
        unsigned shift;

        assert(inv->d1 == dp[dn - 1]);
        assert(inv->d0 == dp[dn - 2]);
        assert((inv->d1 & GMP_LIMB_HIGHBIT) != 0);

        shift = inv->shift;
        if (shift > 0)
            nh = mpn_lshift(np, np, nn, shift);
        else
            nh = 0;

        mpn_div_qr_pi1(qp, np, nn, nh, dp, dn, inv->di);

        if (shift > 0)
            gmp_assert_nocarry(mpn_rshift(np, np, dn, shift));
    }
}

void mpn_div_qr(mp_ptr qp, mp_ptr np, mp_size_t nn, mp_srcptr dp, mp_size_t dn)
{
    struct gmp_div_inverse inv;
    mp_ptr tp = NULL;

    assert(dn > 0);
    assert(nn >= dn);

    mpn_div_qr_invert(&inv, dp, dn);
    if (dn > 2 && inv.shift > 0)
    {
        tp = gmp_xalloc_limbs(dn);
        gmp_assert_nocarry(mpn_lshift(tp, dp, dn, inv.shift));
        dp = tp;
    }
    mpn_div_qr_preinv(qp, np, nn, dp, dn, &inv);
    if (tp)
        gmp_free(tp);
}

/* MPN base conversion. */
unsigned mpn_base_power_of_two_p(unsigned b)
{
    switch (b)
    {
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    case 64:
        return 6;
    case 128:
        return 7;
    case 256:
        return 8;
    default:
        return 0;
    }
}

void mpn_get_base_info(mpn_base_info *info, mp_limb_t b)
{
    mp_limb_t m;
    mp_limb_t p;
    unsigned exp;

    m = GMP_LIMB_MAX / b;
    for (exp = 1, p = b; p <= m; exp++)
        p *= b;

    info->exp = exp;
    info->bb = p;
}

mp_bitcnt_t mpn_limb_size_in_base_2(mp_limb_t u)
{
    unsigned int shift = 0;

    assert(u > 0);
    gmp_clz(&shift, u);
    return GMP_LIMB_BITS - shift;
}

size_t mpn_get_str_bits(unsigned char *sp, unsigned bits, mp_srcptr up, mp_size_t un)
{
    unsigned char mask;
    size_t sn, j;
    mp_size_t i;
    unsigned shift;

    sn = ((un - 1) * GMP_LIMB_BITS + mpn_limb_size_in_base_2(up[un - 1]) + bits - 1) / bits;

    mask = (1U << bits) - 1;

    for (i = 0, j = sn, shift = 0; j-- > 0;)
    {
        unsigned char digit = up[i] >> shift;

        shift += bits;

        if (shift >= GMP_LIMB_BITS && ++i < un)
        {
            shift -= GMP_LIMB_BITS;
            digit |= up[i] << (bits - shift);
        }
        sp[j] = digit & mask;
    }
    return sn;
}

size_t mpn_limb_get_str(unsigned char *sp, mp_limb_t w, const struct gmp_div_inverse *binv)
{
    mp_size_t i;
    for (i = 0; w > 0; i++)
    {
        mp_limb_t h, l, r;

        h = w >> (GMP_LIMB_BITS - binv->shift);
        l = w << binv->shift;

        gmp_udiv_qrnnd_preinv(&w, &r, h, l, binv->d1, binv->di);
        assert((r & (GMP_LIMB_MAX >> (GMP_LIMB_BITS - binv->shift))) == 0);
        r >>= binv->shift;

        sp[i] = r;
    }
    return i;
}

mp_size_t mpn_set_str_bits(mp_ptr rp, const unsigned char *sp, size_t sn, unsigned bits)
{
    mp_size_t rn;
    size_t j;
    unsigned shift;

    for (j = sn, rn = 0, shift = 0; j-- > 0;)
    {
        if (shift == 0)
        {
            rp[rn++] = sp[j];
            shift += bits;
        }
        else
        {
            rp[rn - 1] |= (mp_limb_t)sp[j] << shift;
            shift += bits;
            if (shift >= GMP_LIMB_BITS)
            {
                shift -= GMP_LIMB_BITS;
                if (shift > 0)
                    rp[rn++] = (mp_limb_t)sp[j] >> (bits - shift);
            }
        }
    }
    rn = mpn_normalized_size(rp, rn);
    return rn;
}

mp_size_t mpn_set_str_other(mp_ptr rp, const unsigned char *sp, size_t sn, mp_limb_t b, const mpn_base_info *info)
{
    mp_size_t rn;
    mp_limb_t w;
    unsigned k;
    size_t j;

    assert(sn > 0);

    k = 1 + (sn - 1) % info->exp;

    j = 0;
    w = sp[j++];
    while (--k != 0)
        w = w * b + sp[j++];

    rp[0] = w;

    for (rn = 1; j < sn;)
    {
        mp_limb_t cy;

        w = sp[j++];
        for (k = 1; k < info->exp; k++)
            w = w * b + sp[j++];

        cy = mpn_mul_1(rp, rp, rn, info->bb);
        cy += mpn_add_1(rp, rp, rn, w);
        if (cy > 0)
            rp[rn++] = cy;
    }
    assert(j == sn);

    return rn;
}

int mpn_absfits_ulong_p(mp_srcptr up, mp_size_t un)
{
    int ulongsize = GMP_ULONG_BITS / GMP_LIMB_BITS;
    mp_limb_t ulongrem = 0;

    if (GMP_ULONG_BITS % GMP_LIMB_BITS != 0)
        ulongrem = (mp_limb_t)(ULONG_MAX >> GMP_LIMB_BITS * ulongsize) + 1;

    return un <= ulongsize || (up[ulongsize] < ulongrem && un == ulongsize + 1);
}

mpz_srcptr mpz_roinit_normal_n(mpz_t x, mp_srcptr xp, mp_size_t xs)
{
    x->_mp_alloc = 0;
    x->_mp_d = (mp_ptr)xp;
    x->_mp_size = xs;
    return x;
}

/*
mp_size_t mpz_abs_add(mpz_t r, const mpz_t a, const mpz_t b)
{
    mp_size_t an = GMP_ABS(a->_mp_size);
    mp_size_t bn = GMP_ABS(b->_mp_size);
    mp_ptr rp;
    mp_limb_t cy;

    if (an < bn)
    {
        MPZ_SRCPTR_SWAP(a, b);
        MP_SIZE_T_SWAP(an, bn);
    }

    rp = MPZ_REALLOC(r, an + 1);
    cy = mpn_add(rp, a->_mp_d, an, b->_mp_d, bn);

    rp[an] = cy;

    return an + cy;
}
*/

/* MPZ division */
enum mpz_div_round_mode
{
    GMP_DIV_FLOOR,
    GMP_DIV_CEIL,
    GMP_DIV_TRUNC
};

int mpz_div_qr(mpz_t q, mpz_t r, const mpz_t n, const mpz_t d, enum mpz_div_round_mode mode)
{
    mp_size_t ns, ds, nn, dn, qs;
    ns = n->_mp_size;
    ds = d->_mp_size;

    if (ds == 0)
        gmp_die("mpz_div_qr: Divide by zero.");

    if (ns == 0)
    {
        if (q)
            q->_mp_size = 0;
        if (r)
            r->_mp_size = 0;
        return 0;
    }

    nn = GMP_ABS(ns);
    dn = GMP_ABS(ds);

    qs = ds ^ ns;

    if (nn < dn)
    {
        if (mode == GMP_DIV_CEIL && qs >= 0)
        {
            /* q = 1, r = n - d */
            if (r)
                mpz_sub(r, n, d);
            if (q)
                mpz_set_ui(q, 1);
        }
        else if (mode == GMP_DIV_FLOOR && qs < 0)
        {
            /* q = -1, r = n + d */
            if (r)
                mpz_add(r, n, d);
            if (q)
                mpz_set_si(q, -1);
        }
        else
        {
            /* q = 0, r = d */
            if (r)
                mpz_set(r, n);
            if (q)
                q->_mp_size = 0;
        }
        return 1;
    }
    else
    {
        mp_ptr np, qp;
        mp_size_t qn, rn;
        mpz_t tq, tr;

        mpz_init_set(tr, n);
        np = tr->_mp_d;

        qn = nn - dn + 1;

        if (q)
        {
            mpz_init2(tq, qn * GMP_LIMB_BITS);
            qp = tq->_mp_d;
        }
        else
            qp = NULL;

        mpn_div_qr(qp, np, nn, d->_mp_d, dn);

        if (qp)
        {
            qn -= (qp[qn - 1] == 0);

            tq->_mp_size = qs < 0 ? -qn : qn;
        }
        rn = mpn_normalized_size(np, dn);
        tr->_mp_size = ns < 0 ? -rn : rn;

        if (mode == GMP_DIV_FLOOR && qs < 0 && rn != 0)
        {
            if (q)
                mpz_sub_ui(tq, tq, 1);
            if (r)
                mpz_add(tr, tr, d);
        }
        else if (mode == GMP_DIV_CEIL && qs >= 0 && rn != 0)
        {
            if (q)
                mpz_add_ui(tq, tq, 1);
            if (r)
                mpz_sub(tr, tr, d);
        }

        if (q)
        {
            mpz_swap(tq, q);
            mpz_clear(tq);
        }
        if (r)
            mpz_swap(tr, r);

        mpz_clear(tr);

        return rn != 0;
    }
}

void mpz_div_q_2exp(mpz_t q, const mpz_t u, mp_bitcnt_t bit_index, enum mpz_div_round_mode mode)
{
    mp_size_t un, qn;
    mp_size_t limb_cnt;
    mp_ptr qp;
    int adjust;

    un = u->_mp_size;
    if (un == 0)
    {
        q->_mp_size = 0;
        return;
    }
    limb_cnt = bit_index / GMP_LIMB_BITS;
    qn = GMP_ABS(un) - limb_cnt;
    bit_index %= GMP_LIMB_BITS;

    if (mode == ((un > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* un != 0 here. */
        /* Note: Below, the final indexing at limb_cnt is valid because at
           that point we have qn > 0. */
        adjust = (qn <= 0 || !mpn_zero_p(u->_mp_d, limb_cnt) || (u->_mp_d[limb_cnt] & (((mp_limb_t)1 << bit_index) - 1)));
    else
        adjust = 0;

    if (qn <= 0)
        qn = 0;
    else
    {
        qp = MPZ_REALLOC(q, qn);

        if (bit_index != 0)
        {
            mpn_rshift(qp, u->_mp_d + limb_cnt, qn, bit_index);
            qn -= qp[qn - 1] == 0;
        }
        else
        {
            mpn_copyi(qp, u->_mp_d + limb_cnt, qn);
        }
    }

    q->_mp_size = qn;

    if (adjust)
        mpz_add_ui(q, q, 1);
    if (un < 0)
        mpz_neg(q, q);
}

void mpz_div_r_2exp(mpz_t r, const mpz_t u, mp_bitcnt_t bit_index, enum mpz_div_round_mode mode)
{
    mp_size_t us, un, rn;
    mp_ptr rp;
    mp_limb_t mask;

    us = u->_mp_size;
    if (us == 0 || bit_index == 0)
    {
        r->_mp_size = 0;
        return;
    }
    rn = (bit_index + GMP_LIMB_BITS - 1) / GMP_LIMB_BITS;
    assert(rn > 0);

    rp = MPZ_REALLOC(r, rn);
    un = GMP_ABS(us);

    mask = GMP_LIMB_MAX >> (rn * GMP_LIMB_BITS - bit_index);

    if (rn > un)
    {
        /* Quotient (with truncation) is zero, and remainder is
     non-zero */
        if (mode == ((us > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* us != 0 here. */
        {
            /* Have to negate and sign extend. */
            mp_size_t i;

            gmp_assert_nocarry(!mpn_neg(rp, u->_mp_d, un));
            for (i = un; i < rn - 1; i++)
                rp[i] = GMP_LIMB_MAX;

            rp[rn - 1] = mask;
            us = -us;
        }
        else
        {
            /* Just copy */
            if (r != u)
                mpn_copyi(rp, u->_mp_d, un);

            rn = un;
        }
    }
    else
    {
        if (r != u)
            mpn_copyi(rp, u->_mp_d, rn - 1);

        rp[rn - 1] = u->_mp_d[rn - 1] & mask;

        if (mode == ((us > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* us != 0 here. */
        {
            /* If r != 0, compute 2^{bit_count} - r. */
            mpn_neg(rp, rp, rn);

            rp[rn - 1] &= mask;

            /* us is not used for anything else, so we can modify it
               here to indicate flipped sign. */
            us = -us;
        }
    }
    rn = mpn_normalized_size(rp, rn);
    r->_mp_size = us < 0 ? -rn : rn;
}

unsigned long mpz_div_qr_ui(mpz_t q, mpz_t r,
                            const mpz_t n, unsigned long d, enum mpz_div_round_mode mode)
{
    unsigned long ret;
    mpz_t rr, dd;

    mpz_init(rr);
    mpz_init_set_ui(dd, d);
    mpz_div_qr(q, rr, n, dd, mode);
    mpz_clear(dd);
    ret = mpz_get_ui(rr);

    if (r)
        mpz_swap(r, rr);
    mpz_clear(rr);

    return ret;
}

/* GCD */

mp_bitcnt_t mpz_make_odd(mpz_t r)
{
    mp_bitcnt_t shift;

    assert(r->_mp_size > 0);
    /* Count trailing zeros, equivalent to mpn_scan1, because we know that there is a 1 */
    shift = mpn_common_scan(r->_mp_d[0], 0, r->_mp_d, 0, 0);
    mpz_tdiv_q_2exp(r, r, shift);

    return shift;
}

/* Primality testing */

/* Computes Kronecker (a/b) with odd b, a!=0 and GCD(a,b) = 1 */
/* Adapted from JACOBI_BASE_METHOD==4 in mpn/generic/jacbase.c */
int gmp_jacobi_coprime(mp_limb_t a, mp_limb_t b)
{
    unsigned int c = 0;
    int bit = 0;

    assert(b & 1);
    assert(a != 0);
    /* assert (mpn_gcd_11 (a, b) == 1); */

    /* Below, we represent a and b shifted right so that the least
       significant one bit is implicit. */
    b >>= 1;

    gmp_ctz(&c, a);
    a >>= 1;

    do
    {
        a >>= c;
        /* (2/b) = -1 if b = 3 or 5 mod 8 */
        bit ^= c & (b ^ (b >> 1));
        if (a < b)
        {
            bit ^= a & b;
            a = b - a;
            b -= a;
        }
        else
        {
            a -= b;
            assert(a != 0);
        }

        gmp_ctz(&c, a);
        ++c;
    } while (b > 0);

    return bit & 1 ? -1 : 1;
}

void gmp_lucas_step_k_2k(mpz_t V, mpz_t Qk, const mpz_t n)
{
    mpz_mod(Qk, Qk, n);
    /* V_{2k} <- V_k ^ 2 - 2Q^k */
    mpz_mul(V, V, V);
    mpz_submul_ui(V, Qk, 2);
    mpz_tdiv_r(V, V, n);
    /* Q^{2k} = (Q^k)^2 */
    mpz_mul(Qk, Qk, Qk);
}

/* Computes V_k, Q^k (mod n) for the Lucas' sequence */
/* with P=1, Q=Q; k = (n>>b0)|1. */
/* Requires an odd n > 4; b0 > 0; -2*Q must not overflow a long */
/* Returns (U_k == 0) and sets V=V_k and Qk=Q^k. */
int gmp_lucas_mod(mpz_t V, mpz_t Qk, long Q,
                  mp_bitcnt_t b0, mpz_struct *n)
{
    mp_bitcnt_t bs;
    mpz_t U;
    int res;

    assert(b0 > 0);
    assert(Q <= -(LONG_MIN / 2));
    assert(Q >= -(LONG_MAX / 2));
    assert(mpz_cmp_ui(n, 4) > 0);
    assert(mpz_odd_p(n));

    mpz_init_set_ui(U, 1); /* U1 = 1 */
    mpz_set_ui(V, 1);      /* V1 = 1 */
    mpz_set_si(Qk, Q);

    for (bs = mpz_sizeinbase(n, 2) - 1; --bs >= b0;)
    {
        /* U_{2k} <- U_k * V_k */
        mpz_mul(U, U, V);
        /* V_{2k} <- V_k ^ 2 - 2Q^k */
        /* Q^{2k} = (Q^k)^2 */
        gmp_lucas_step_k_2k(V, Qk, n);

        /* A step k->k+1 is performed if the bit in $n$ is 1	*/
        /* mpz_tstbit(n,bs) or the bit is 0 in $n$ but	*/
        /* should be 1 in $n+1$ (bs == b0)			*/
        if (b0 == bs || mpz_tstbit(n, bs))
        {
            /* Q^{k+1} <- Q^k * Q */
            mpz_mul_si(Qk, Qk, Q);
            /* U_{k+1} <- (U_k + V_k) / 2 */
            mpz_swap(U, V); /* Keep in V the old value of U_k */
            mpz_add(U, U, V);
            /* We have to compute U/2, so we need an even value, */
            /* equivalent (mod n) */
            if (mpz_odd_p(U))
                mpz_add(U, U, n);
            mpz_tdiv_q_2exp(U, U, 1);
            /* V_{k+1} <-(D*U_k + V_k) / 2 =
              U_{k+1} + (D-1)/2*U_k = U_{k+1} - 2Q*U_k */
            mpz_mul_si(V, V, -2 * Q);
            mpz_add(V, U, V);
            mpz_tdiv_r(V, V, n);
        }
        mpz_tdiv_r(U, U, n);
    }

    res = U->_mp_size == 0;
    mpz_clear(U);
    return res;
}

/* Performs strong Lucas' test on x, with parameters suggested */
/* for the BPSW test. Qk is only passed to recycle a variable. */
/* Requires GCD (x,6) = 1.*/
int gmp_stronglucas(const mpz_t x, mpz_t Qk)
{
    mp_bitcnt_t b0;
    mpz_t V, n;
    mp_limb_t maxD, D; /* The absolute value is stored. */
    long Q;
    mp_limb_t tl;

    /* Test on the absolute value. */
    mpz_roinit_normal_n(n, x->_mp_d, GMP_ABS(x->_mp_size));

    assert(mpz_odd_p(n));
    /* assert (mpz_gcd_ui (NULL, n, 6) == 1); */
    if (mpz_root(Qk, n, 2))
        return 0; /* A square is composite. */

    /* Check Ds up to square root (in case, n is prime)
       or avoid overflows */
    maxD = (Qk->_mp_size == 1) ? Qk->_mp_d[0] - 1 : GMP_LIMB_MAX;

    D = 3;
    /* Search a D such that (D/n) = -1 in the sequence 5,-7,9,-11,.. */
    /* For those Ds we have (D/n) = (n/|D|) */
    do
    {
        if (D >= maxD)
            return 1 + (D != GMP_LIMB_MAX); /* (1 + ! ~ D) */
        D += 2;
        tl = mpz_tdiv_ui(n, D);
        if (tl == 0)
            return 0;
    } while (gmp_jacobi_coprime(tl, D) == 1);

    mpz_init(V);

    /* n-(D/n) = n+1 = d*2^{b0}, with d = (n>>b0) | 1 */
    b0 = mpz_scan0(n, 0);

    /* D= P^2 - 4Q; P = 1; Q = (1-D)/4 */
    Q = (D & 2) ? (long)(D >> 2) + 1 : -(long)(D >> 2);

    if (!gmp_lucas_mod(V, Qk, Q, b0, n))      /* If Ud != 0 */
        while (V->_mp_size != 0 && --b0 != 0) /* while Vk != 0 */
            /* V <- V ^ 2 - 2Q^k */
            /* Q^{2k} = (Q^k)^2 */
            gmp_lucas_step_k_2k(V, Qk, n);

    mpz_clear(V);
    return (b0 != 0);
}

int gmp_millerrabin(const mpz_t n, const mpz_t nm1, mpz_t y,
                    const mpz_t q, mp_bitcnt_t k)
{
    assert(k > 0);

    /* Caller must initialize y to the base. */
    mpz_powm(y, y, q, n);

    if (mpz_cmp_ui(y, 1) == 0 || mpz_cmp(y, nm1) == 0)
        return 1;

    while (--k > 0)
    {
        mpz_powm_ui(y, y, 2, n);
        if (mpz_cmp(y, nm1) == 0)
            return 1;
        /* y == 1 means that the previous y was a non-trivial square root
     of 1 (mod n). y == 0 means that n is a power of the base.
     In either case, n is not prime. */
        if (mpz_cmp_ui(y, 1) <= 0)
            return 0;
    }
    return 0;
}

void mpz_abs_add_bit(mpz_t d, mp_bitcnt_t bit_index)
{
    mp_size_t dn, limb_index;
    mp_limb_t bit;
    mp_ptr dp;

    dn = GMP_ABS(d->_mp_size);

    limb_index = bit_index / GMP_LIMB_BITS;
    bit = (mp_limb_t)1 << (bit_index % GMP_LIMB_BITS);

    if (limb_index >= dn)
    {
        mp_size_t i;
        /* The bit should be set outside of the end of the number.
     We have to increase the size of the number. */
        dp = MPZ_REALLOC(d, limb_index + 1);

        dp[limb_index] = bit;
        for (i = dn; i < limb_index; i++)
            dp[i] = 0;
        dn = limb_index + 1;
    }
    else
    {
        mp_limb_t cy;

        dp = d->_mp_d;

        cy = mpn_add_1(dp + limb_index, dp + limb_index, dn - limb_index, bit);
        if (cy > 0)
        {
            dp = MPZ_REALLOC(d, dn + 1);
            dp[dn++] = cy;
        }
    }

    d->_mp_size = (d->_mp_size < 0) ? -dn : dn;
}

void mpz_abs_sub_bit(mpz_t d, mp_bitcnt_t bit_index)
{
    mp_size_t dn, limb_index;
    mp_ptr dp;
    mp_limb_t bit;

    dn = GMP_ABS(d->_mp_size);
    dp = d->_mp_d;

    limb_index = bit_index / GMP_LIMB_BITS;
    bit = (mp_limb_t)1 << (bit_index % GMP_LIMB_BITS);

    assert(limb_index < dn);

    gmp_assert_nocarry(mpn_sub_1(dp + limb_index, dp + limb_index,
                                 dn - limb_index, bit));
    dn = mpn_normalized_size(dp, dn);
    d->_mp_size = (d->_mp_size < 0) ? -dn : dn;
}

unsigned gmp_popcount_limb(mp_limb_t x)
{
    unsigned c;

    /* Do 16 bits at a time, to avoid limb-sized constants. */
    int LOCAL_SHIFT_BITS = 16;
    for (c = 0; x > 0;)
    {
        unsigned w = x - ((x >> 1) & 0x5555);
        w = ((w >> 2) & 0x3333) + (w & 0x3333);
        w = (w >> 4) + w;
        w = ((w >> 8) & 0x000f) + (w & 0x000f);
        c += w;
        if (GMP_LIMB_BITS > LOCAL_SHIFT_BITS)
            x >>= LOCAL_SHIFT_BITS;
        else
            x = 0;
    }
    return c;
}

/* MPZ base conversion. */

int gmp_detect_endian(void)
{
    const int i = 2;
    const unsigned char *p = (const unsigned char *)&i;
    return 1 - *p;
}
