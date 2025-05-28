#include "mpn.h"
#include <limits.h>
#include "assert.h"
#include "libs/GMP/longlong.h"

void mpn_copyi(mp_ptr d, mp_srcptr s, mp_size_t n)
{
    mp_size_t i;
    for (i = 0; i < n; i++)
        d[i] = s[i];
}

void mpn_copyd(mp_ptr d, mp_srcptr s, mp_size_t n)
{
    while (--n >= 0)
        d[n] = s[n];
}

int mpn_cmp(mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
    while (--n >= 0)
    {
        if (ap[n] != bp[n])
            return ap[n] > bp[n] ? 1 : -1;
    }
    return 0;
}

int mpn_cmp4(mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
    if (an != bn)
        return an < bn ? -1 : 1;
    else
        return mpn_cmp(ap, bp, an);
}

mp_size_t mpn_normalized_size(mp_srcptr xp, mp_size_t n)
{
    while (n > 0 && xp[n - 1] == 0)
        --n;
    return n;
}

int mpn_zero_p(mp_srcptr rp, mp_size_t n)
{
    return mpn_normalized_size(rp, n) == 0;
}

void mpn_zero(mp_ptr rp, mp_size_t n)
{
    while (--n >= 0)
        rp[n] = 0;
}

mp_limb_t mpn_add_1(mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b)
{
    mp_size_t i;

    assert(n > 0);
    i = 0;
    do
    {
        mp_limb_t r = ap[i] + b;
        /* Carry out */
        b = (r < b);
        rp[i] = r;
    } while (++i < n);

    return b;
}

mp_limb_t mpn_add_n(mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
    mp_size_t i;
    mp_limb_t cy;

    for (i = 0, cy = 0; i < n; i++)
    {
        mp_limb_t a, b, r;
        a = ap[i];
        b = bp[i];
        r = a + cy;
        cy = (r < cy);
        r += b;
        cy += (r < b);
        rp[i] = r;
    }
    return cy;
}

mp_limb_t mpn_add(mp_ptr rp, mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
    mp_limb_t cy;

    assert(an >= bn);

    cy = mpn_add_n(rp, ap, bp, bn);
    if (an > bn)
        cy = mpn_add_1(rp + bn, ap + bn, an - bn, cy);
    return cy;
}

mp_limb_t mpn_sub_1(mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b)
{
    mp_size_t i;

    assert(n > 0);

    i = 0;
    do
    {
        mp_limb_t a = ap[i];
        /* Carry out */
        mp_limb_t cy = a < b;
        rp[i] = a - b;
        b = cy;
    } while (++i < n);

    return b;
}

mp_limb_t mpn_sub_n(mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
    mp_size_t i;
    mp_limb_t cy;

    for (i = 0, cy = 0; i < n; i++)
    {
        mp_limb_t a, b;
        a = ap[i];
        b = bp[i];
        b += cy;
        cy = (b < cy);
        cy += (a < b);
        rp[i] = a - b;
    }
    return cy;
}

mp_limb_t mpn_sub(mp_ptr rp, mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
    mp_limb_t cy;

    assert(an >= bn);

    cy = mpn_sub_n(rp, ap, bp, bn);
    if (an > bn)
        cy = mpn_sub_1(rp + bn, ap + bn, an - bn, cy);
    return cy;
}

mp_limb_t mpn_mul_1(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
    mp_limb_t ul = 0;
    mp_limb_t cl = 0;
    mp_limb_t hpl = 0;
    mp_limb_t lpl = 0;

    assert(n >= 1);

    cl = 0;
    do
    {
        ul = *up++;
        gmp_umul_ppmm(&hpl, &lpl, ul, vl);

        lpl += cl;
        cl = (lpl < cl) + hpl;

        *rp++ = lpl;
    } while (--n != 0);

    return cl;
}

mp_limb_t mpn_addmul_1(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
    mp_limb_t ul = 0;
    mp_limb_t cl = 0;
    mp_limb_t hpl = 0;
    mp_limb_t lpl = 0;
    mp_limb_t rl = 0;

    assert(n >= 1);

    cl = 0;
    do
    {
        ul = *up++;
        gmp_umul_ppmm(&hpl, &lpl, ul, vl);

        lpl += cl;
        cl = (lpl < cl) + hpl;

        rl = *rp;
        lpl = rl + lpl;
        cl += lpl < rl;
        *rp++ = lpl;
    } while (--n != 0);

    return cl;
}

mp_limb_t mpn_submul_1(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
    mp_limb_t ul = 0;
    mp_limb_t cl = 0;
    mp_limb_t hpl = 0;
    mp_limb_t lpl = 0;
    mp_limb_t rl = 0;

    assert(n >= 1);

    cl = 0;
    do
    {
        ul = *up++;
        gmp_umul_ppmm(&hpl, &lpl, ul, vl);

        lpl += cl;
        cl = (lpl < cl) + hpl;

        rl = *rp;
        lpl = rl - lpl;
        cl += lpl > rl;
        *rp++ = lpl;
    } while (--n != 0);

    return cl;
}

mp_limb_t mpn_mul(mp_ptr rp, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn)
{
    assert(un >= vn);
    assert(vn >= 1);

    /* We first multiply by the low order limb. This result can be
       stored, not added, to rp. We also avoid a loop for zeroing this
       way. */

    rp[un] = mpn_mul_1(rp, up, un, vp[0]);

    /* Now accumulate the product of up[] and the next higher limb from
       vp[]. */

    while (--vn >= 1)
    {
        rp += 1, vp += 1;
        rp[un] = mpn_addmul_1(rp, up, un, vp[0]);
    }
    return rp[un];
}

void mpn_mul_n(mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
    mpn_mul(rp, ap, n, bp, n);
}

void mpn_sqr(mp_ptr rp, mp_srcptr ap, mp_size_t n)
{
    mpn_mul(rp, ap, n, ap, n);
}

mp_limb_t mpn_lshift(mp_ptr rp, mp_srcptr up, mp_size_t n, unsigned int cnt)
{
    mp_limb_t high_limb, low_limb;
    unsigned int tnc;
    mp_limb_t retval;

    assert(n >= 1);
    assert(cnt >= 1);
    assert(cnt < GMP_LIMB_BITS);

    up += n;
    rp += n;

    tnc = GMP_LIMB_BITS - cnt;
    low_limb = *--up;
    retval = low_limb >> tnc;
    high_limb = (low_limb << cnt);

    while (--n != 0)
    {
        low_limb = *--up;
        *--rp = high_limb | (low_limb >> tnc);
        high_limb = (low_limb << cnt);
    }
    *--rp = high_limb;

    return retval;
}

mp_limb_t mpn_rshift(mp_ptr rp, mp_srcptr up, mp_size_t n, unsigned int cnt)
{
    mp_limb_t high_limb, low_limb;
    unsigned int tnc;
    mp_limb_t retval;

    assert(n >= 1);
    assert(cnt >= 1);
    assert(cnt < GMP_LIMB_BITS);

    tnc = GMP_LIMB_BITS - cnt;
    high_limb = *up++;
    retval = (high_limb << tnc);
    low_limb = high_limb >> cnt;

    while (--n != 0)
    {
        high_limb = *up++;
        *rp++ = low_limb | (high_limb << tnc);
        low_limb = high_limb >> cnt;
    }
    *rp = low_limb;

    return retval;
}
/*
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
*/

mp_bitcnt_t
mpn_scan1(mp_srcptr ptr, mp_bitcnt_t bit)
{
    mp_size_t i;
    i = bit / GMP_LIMB_BITS;
    
    return mpn_common_scan(ptr[i] & (GMP_LIMB_MAX << (bit % GMP_LIMB_BITS)),
    i, ptr, i, 0);
}

mp_bitcnt_t mpn_scan0(mp_srcptr ptr, mp_bitcnt_t bit)
{
    mp_size_t i;
    i = bit / GMP_LIMB_BITS;

    return mpn_common_scan(~ptr[i] & (GMP_LIMB_MAX << (bit % GMP_LIMB_BITS)),
                           i, ptr, i, GMP_LIMB_MAX);
}

void mpn_com(mp_ptr rp, mp_srcptr up, mp_size_t n)
{
    while (--n >= 0)
        *rp++ = ~*up++;
}

mp_limb_t mpn_neg(mp_ptr rp, mp_srcptr up, mp_size_t n)
{
    while (*up == 0)
    {
        *rp = 0;
        if (!--n)
            return 0;
        ++up;
        ++rp;
    }
    *rp = -*up;
    mpn_com(++rp, ++up, --n);
    return 1;
}
/* MPN division interface. */

/* The 3/2 inverse is defined as

     m = floor( (B^3-1) / (B u1 + u0)) - B
*/
mp_limb_t mpn_invert_3by2(mp_limb_t u1, mp_limb_t u0)
{
    mp_limb_t r, m;

    {
        mp_limb_t p, ql;
        unsigned ul, uh, qh;

        /* For notation, let b denote the half-limb base, so that B = b^2.
           Split u1 = b uh + ul. */
        ul = u1 & GMP_LLIMB_MASK;
        uh = u1 >> (GMP_LIMB_BITS / 2);

        /* Approximation of the high half of quotient. Differs from the 2/1
           inverse of the half limb uh, since we have already subtracted
           u0. */
        qh = (u1 ^ GMP_LIMB_MAX) / uh;

        /* Adjust to get a half-limb 3/2 inverse, i.e., we want

           qh' = floor( (b^3 - 1) / u) - b = floor ((b^3 - b u - 1) / u
           = floor( (b (~u) + b-1) / u),

           and the remainder

           r = b (~u) + b-1 - qh (b uh + ul)
           = b (~u - qh uh) + b-1 - qh ul

           Subtraction of qh ul may underflow, which implies adjustments.
           But by normalization, 2 u >= B > qh ul, so we need to adjust by
           at most 2.
        */

        r = ((~u1 - (mp_limb_t)qh * uh) << (GMP_LIMB_BITS / 2)) | GMP_LLIMB_MASK;

        p = (mp_limb_t)qh * ul;
        /* Adjustment steps taken from udiv_qrnnd_c */
        if (r < p)
        {
            qh--;
            r += u1;
            if (r >= u1) /* i.e. we didn't get carry when adding to r */
                if (r < p)
                {
                    qh--;
                    r += u1;
                }
        }
        r -= p;

        /* Low half of the quotient is

           ql = floor ( (b r + b-1) / u1).

           This is a 3/2 division (on half-limbs), for which qh is a
           suitable inverse. */

        p = (r >> (GMP_LIMB_BITS / 2)) * qh + r;
        /* Unlike full-limb 3/2, we can add 1 without overflow. For this to
           work, it is essential that ql is a full mp_limb_t. */
        ql = (p >> (GMP_LIMB_BITS / 2)) + 1;

        /* By the 3/2 trick, we don't need the high half limb. */
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

    /* Now m is the 2/1 inverse of u1. If u0 > 0, adjust it to become a
       3/2 inverse. */
    if (u0 > 0)
    {
        mp_limb_t th = 0;
        mp_limb_t tl = 0;
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

size_t mpn_get_str_other(unsigned char *sp, int base, const mpn_base_info *info, mp_ptr up, mp_size_t un)
{
    struct gmp_div_inverse binv;
    size_t sn;
    size_t i;

    mpn_div_qr_1_invert(&binv, base);

    sn = 0;

    if (un > 1)
    {
        struct gmp_div_inverse bbinv;
        mpn_div_qr_1_invert(&bbinv, info->bb);

        do
        {
            mp_limb_t w;
            size_t done;
            w = mpn_div_qr_1_preinv(up, up, un, &bbinv);
            un -= (up[un - 1] == 0);
            done = mpn_limb_get_str(sp + sn, w, &binv);

            for (sn += done; done < info->exp; done++)
                sp[sn++] = 0;
        } while (un > 1);
    }
    sn += mpn_limb_get_str(sp + sn, up[0], &binv);

    /* Reverse order */
    for (i = 0; 2 * i + 1 < sn; i++)
    {
        unsigned char t = sp[i];
        sp[i] = sp[sn - i - 1];
        sp[sn - i - 1] = t;
    }

    return sn;
}

size_t mpn_get_str(unsigned char *sp, int base, mp_ptr up, mp_size_t un)
{
    unsigned bits;

    assert(un > 0);
    assert(up[un - 1] > 0);

    bits = mpn_base_power_of_two_p(base);
    if (bits)
        return mpn_get_str_bits(sp, bits, up, un);
    else
    {
        mpn_base_info info;

        mpn_get_base_info(&info, base);
        return mpn_get_str_other(sp, base, &info, up, un);
    }
    return 0;
}
