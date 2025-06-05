#include "mpz.h"
#include <limits.h>
#include "libs/GMP/gmpdefault.h"
#include "libs/GMP/gmp-impl.h"
#include "libs/GMP/longlong.h"

#if HAVE_LIMB_BIG_ENDIAN
#define HOST_ENDIAN     1
#endif
#if HAVE_LIMB_LITTLE_ENDIAN
#define HOST_ENDIAN     (-1)
#endif
#ifndef HOST_ENDIAN
static const mp_limb_t  endian_test = (CNST_LIMB(1) << (GMP_LIMB_BITS-7)) - 1;
#define HOST_ENDIAN     (* (signed char *) &endian_test)
#endif

/* MPZ interface */
void mpz_init(mpz_ptr r)
{
    const mp_limb_t dummy_limb = 0xc1a0;

    r->_mp_alloc = 0;
    r->_mp_size = 0;
    r->_mp_d = (mp_ptr)&dummy_limb;
}

void mpz_init2(mpz_ptr x, mp_bitcnt_t bits)
{
    mp_size_t rn;

    bits -= (bits != 0);
    rn = 1 + bits / GMP_LIMB_BITS;

    x->_mp_alloc = rn;
    x->_mp_size = 0;
    x->_mp_d = gmp_xalloc_limbs(rn);
}

void mpz_clear(mpz_ptr r)
{
    if (r->_mp_alloc)
        gmp_free(r->_mp_d);
}

void *mpz_export(void *data, size_t *countp, int order, size_t size, int endian, size_t nail, mpz_srcptr z)
{
    mp_size_t zsize;
    mp_srcptr zp;
    size_t count, dummy;
    unsigned long numb;
    unsigned align;

    ASSERT(order == 1 || order == -1);
    ASSERT(endian == 1 || endian == 0 || endian == -1);
    ASSERT(nail <= 8 * size);
    ASSERT(nail < 8 * size || SIZ(z) == 0); /* nail < 8*size+(SIZ(z)==0) */

    if (countp == NULL)
        countp = &dummy;

    zsize = SIZ(z);
    if (zsize == 0)
    {
        *countp = 0;
        return data;
    }

    zsize = ABS(zsize);
    zp = PTR(z);
    numb = 8 * size - nail;
    count = mpn_sizeinbase_2exp(zp, zsize, numb);
    *countp = count;

    if (data == NULL)
        data = (*gmp_allocate_func)(count * size);

    if (endian == 0)
        endian = HOST_ENDIAN;

    align = ((char *)data - (char *)NULL) % sizeof(mp_limb_t);

    if (nail == GMP_NAIL_BITS)
    {
        if (size == sizeof(mp_limb_t) && align == 0)
        {
            if (order == -1 && endian == HOST_ENDIAN)
            {
                MPN_COPY((mp_ptr)data, zp, (mp_size_t)count);
                return data;
            }
            if (order == 1 && endian == HOST_ENDIAN)
            {
                MPN_REVERSE((mp_ptr)data, zp, (mp_size_t)count);
                return data;
            }

            if (order == -1 && endian == -HOST_ENDIAN)
            {
                MPN_BSWAP((mp_ptr)data, zp, (mp_size_t)count);
                return data;
            }
            if (order == 1 && endian == -HOST_ENDIAN)
            {
                MPN_BSWAP_REVERSE((mp_ptr)data, zp, (mp_size_t)count);
                return data;
            }
        }
    }

    {
        mp_limb_t limb, wbitsmask;
        size_t i, numb;
        mp_size_t j, wbytes, woffset;
        unsigned char *dp;
        int lbits, wbits;
        mp_srcptr zend;

        numb = size * 8 - nail;

        /* whole bytes per word */
        wbytes = numb / 8;

        /* possible partial byte */
        wbits = numb % 8;
        wbitsmask = (CNST_LIMB(1) << wbits) - 1;

        /* offset to get to the next word */
        woffset = (endian >= 0 ? size : -(mp_size_t)size) + (order < 0 ? size : -(mp_size_t)size);

        /* least significant byte */
        dp = (unsigned char *)data + (order >= 0 ? (count - 1) * size : 0) + (endian >= 0 ? size - 1 : 0);

#define EXTRACT(N, MASK)                            \
    do                                              \
    {                                               \
        if (lbits >= (N))                           \
        {                                           \
            *dp = limb MASK;                        \
            limb >>= (N);                           \
            lbits -= (N);                           \
        }                                           \
        else                                        \
        {                                           \
            mp_limb_t newlimb;                      \
            newlimb = (zp == zend ? 0 : *zp++);     \
            *dp = (limb | (newlimb << lbits)) MASK; \
            limb = newlimb >> ((N) - lbits);        \
            lbits += GMP_NUMB_BITS - (N);           \
        }                                           \
    } while (0)

        zend = zp + zsize;
        lbits = 0;
        limb = 0;
        for (i = 0; i < count; i++)
        {
            for (j = 0; j < wbytes; j++)
            {
                EXTRACT(8, +0);
                dp -= endian;
            }
            if (wbits != 0)
            {
                EXTRACT(wbits, &wbitsmask);
                dp -= endian;
                j++;
            }
            for (; j < size; j++)
            {
                *dp = '\0';
                dp -= endian;
            }
            dp += woffset;
        }

        ASSERT(zp == PTR(z) + ABSIZ(z));

        /* low byte of word after most significant */
        ASSERT(dp == (unsigned char *)data + (order < 0 ? count * size : -(mp_size_t)size) + (endian >= 0 ? (mp_size_t)size - 1 : 0));
    }
    return data;
}

void mpz_mod(mpz_ptr rem, mpz_srcptr dividend, mpz_srcptr divisor)
{
    mp_size_t rn, bn;
    mpz_t temp_divisor;
    TMP_DECL;

    TMP_MARK;

    bn = ABSIZ(divisor);

    /* We need the original value of the divisor after the remainder has been
       preliminary calculated.  We have to copy it to temporary space if it's
       the same variable as REM.  */
    if (rem == divisor)
    {
        PTR(temp_divisor) = TMP_ALLOC_LIMBS(bn);
        MPN_COPY(PTR(temp_divisor), PTR(divisor), bn);
    }
    else
    {
        PTR(temp_divisor) = PTR(divisor);
    }
    SIZ(temp_divisor) = bn;
    divisor = temp_divisor;

    mpz_tdiv_r(rem, dividend, divisor);

    rn = SIZ(rem);
    if (rn < 0)
        mpz_add(rem, rem, divisor);

    TMP_FREE;
}

mp_ptr mpz_realloc(mpz_ptr r, mp_size_t size)
{
    size = GMP_MAX(size, 1);

    if (r->_mp_alloc)
        r->_mp_d = gmp_xrealloc_limbs(r->_mp_d, size);
    else
        r->_mp_d = gmp_xalloc_limbs(size);
    r->_mp_alloc = size;

    if (GMP_ABS(r->_mp_size) > size)
        r->_mp_size = 0;

    return r->_mp_d;
}

/* Realloc for an mpz_struct* WHAT if it has less than NEEDED limbs.  */

void mpz_set_ui(mpz_struct *r, unsigned long int x)
{
    if (x > 0)
    {
        r->_mp_size = 1;
        MPZ_REALLOC(r, 1)
        [0] = x;
    }
    else
        r->_mp_size = 0;
}

void mpz_set(mpz_struct *r, mpz_srcptr x)
{
    /* Allow the NOP r == x */
    if (r != x)
    {
        mp_size_t n;
        mp_ptr rp;

        n = GMP_ABS(x->_mp_size);
        rp = MPZ_REALLOC(r, n);

        mpn_copyi(rp, x->_mp_d, n);
        r->_mp_size = x->_mp_size;
    }
}

void mpz_init_set_ui(mpz_struct *r, unsigned long int x)
{
    mpz_init(r);
    mpz_set_ui(r, x);
}

void mpz_init_set(mpz_struct *r, mpz_srcptr x)
{
    mpz_init(r);
    mpz_set(r, x);
}

int mpz_fits_slong_p(mpz_srcptr u)
{
    mp_size_t us = u->_mp_size;

    if (us == 1)
        return u->_mp_d[0] < GMP_LIMB_HIGHBIT;
    else if (us == -1)
        return u->_mp_d[0] <= GMP_LIMB_HIGHBIT;
    else
        return (us == 0);
}

int mpz_fits_ulong_p(mpz_srcptr u)
{
    mp_size_t us = u->_mp_size;

    return (us == (us > 0));
}

long int mpz_get_si(mpz_struct *u)
{
    if (u->_mp_size < 0)
        /* This expression is necessary to properly handle 0x80000000 */
        return -1 - (long)((u->_mp_d[0] - 1) & ~GMP_LIMB_HIGHBIT);
    else
        return (long)(mpz_get_ui(u) & ~GMP_LIMB_HIGHBIT);
}

unsigned long int mpz_get_ui(mpz_struct *u)
{
    return u->_mp_size == 0 ? 0 : u->_mp_d[0];
}

void mpz_limbs_finish(mpz_struct *x, mp_size_t xs)
{
    mp_size_t xn;
    xn = mpn_normalized_size(x->_mp_d, GMP_ABS(xs));
    x->_mp_size = xs < 0 ? -xn : xn;
}

mpz_struct *mpz_roinit_n(mpz_struct *x, unsigned long *xp, mp_size_t xs)
{
    x->_mp_alloc = 0;
    x->_mp_d = (mp_ptr)xp;
    mpz_limbs_finish(x, xs);
    return x;
}

/* MPZ comparisons and the like. */
int mpz_sgn(mpz_struct *u)
{
    return GMP_CMP(u->_mp_size, 0);
}

int _mpz_cmp_si(mpz_struct *u, long v)
{
    mp_size_t usize = u->_mp_size;

    if (usize < -1)
        return -1;
    else if (v >= 0)
        return mpz_cmp_ui(u, v);
    else if (usize >= 0)
        return 1;
    else /* usize == -1 */
        return GMP_CMP(GMP_NEG_CAST(mp_limb_t, v), u->_mp_d[0]);
    return 0;
}

int _mpz_cmp_ui(mpz_ptr u, unsigned long v)
{
    mp_size_t usize = u->_mp_size;

    if (usize > 1)
        return 1;
    else if (usize < 0)
        return -1;
    else
        return GMP_CMP(mpz_get_ui(u), v);
}

int mpz_cmp(mpz_srcptr a, mpz_srcptr b)
{
    mp_size_t asize = a->_mp_size;
    mp_size_t bsize = b->_mp_size;

    if (asize != bsize)
        return (asize < bsize) ? -1 : 1;
    else if (asize >= 0)
        return mpn_cmp(a->_mp_d, b->_mp_d, asize);
    else
        return mpn_cmp(b->_mp_d, a->_mp_d, -asize);
}

int mpz_cmpabs_ui(mpz_srcptr u, unsigned long v)
{
    if (GMP_ABS(u->_mp_size) > 1)
        return 1;
    else
        return GMP_CMP(mpz_get_ui((mpz_struct *)u), v);
}

int mpz_cmpabs(mpz_srcptr u, mpz_srcptr v)
{
    return mpn_cmp4((mp_srcptr)u->_mp_d, GMP_ABS(u->_mp_size), (mp_srcptr)v->_mp_d, GMP_ABS(v->_mp_size));
}

void mpz_abs(mpz_struct *r, mpz_srcptr u)
{
    mpz_set(r, u);
    r->_mp_size = GMP_ABS(r->_mp_size);
}

void mpz_neg(mpz_struct *r, mpz_srcptr u)
{
    mpz_set(r, u);
    r->_mp_size = -r->_mp_size;
}

void mpz_swap(mpz_struct *u, mpz_struct *v)
{
    MP_SIZE_T_SWAP(u->_mp_size, v->_mp_size);
    MP_SIZE_T_SWAP(u->_mp_alloc, v->_mp_alloc);
    MP_PTR_SWAP(u->_mp_d, v->_mp_d);
}

/* MPZ addition and subtraction */

/* Adds to the absolute value. Returns new size, but doesn't store it. */
mp_size_t mpz_abs_add_ui(mpz_struct *r, mpz_srcptr a, unsigned long b)
{
    mp_size_t an;
    mp_ptr rp;
    mp_limb_t cy;

    an = GMP_ABS(a->_mp_size);
    if (an == 0)
    {
        MPZ_REALLOC(r, 1)
        [0] = b;
        return b > 0;
    }

    rp = MPZ_REALLOC(r, an + 1);

    cy = mpn_add_1(rp, a->_mp_d, an, b);
    rp[an] = cy;
    an += cy;

    return an;
}

/* Subtract from the absolute value. Returns new size, (or -1 on underflow),
   but doesn't store it. */
mp_size_t mpz_abs_sub_ui(mpz_struct *r, mpz_srcptr a, unsigned long b)
{
    mp_size_t an = GMP_ABS(a->_mp_size);
    mp_ptr rp;

    if (an == 0)
    {
        MPZ_REALLOC(r, 1)
        [0] = b;
        return -(b > 0);
    }
    rp = MPZ_REALLOC(r, an);
    if (an == 1 && a->_mp_d[0] < b)
    {
        rp[0] = b - a->_mp_d[0];
        return -1;
    }
    else
    {
        gmp_assert_nocarry(mpn_sub_1(rp, a->_mp_d, an, b));
        return mpn_normalized_size(rp, an);
    }
}

void mpz_add_ui(mpz_struct *r, mpz_srcptr a, unsigned long b)
{
    if (a->_mp_size >= 0)
        r->_mp_size = mpz_abs_add_ui(r, a, b);
    else
        r->_mp_size = -mpz_abs_sub_ui(r, a, b);
}

void mpz_sub_ui(mpz_struct *r, mpz_srcptr a, unsigned long b)
{
    if (a->_mp_size < 0)
        r->_mp_size = -mpz_abs_add_ui(r, a, b);
    else
        r->_mp_size = mpz_abs_sub_ui(r, a, b);
}

void mpz_ui_sub(mpz_struct *r, unsigned long a, mpz_srcptr b)
{
    if (b->_mp_size < 0)
        r->_mp_size = mpz_abs_add_ui(r, b, a);
    else
        r->_mp_size = -mpz_abs_sub_ui(r, b, a);
}

mp_size_t mpz_abs_add(mpz_struct *r, mpz_srcptr a, mpz_srcptr b)
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

mp_size_t mpz_abs_sub(mpz_struct *r, mpz_srcptr a, mpz_srcptr b)
{
    mp_size_t an = GMP_ABS(a->_mp_size);
    mp_size_t bn = GMP_ABS(b->_mp_size);
    int cmp;
    mp_ptr rp;

    cmp = mpn_cmp4(a->_mp_d, an, b->_mp_d, bn);
    if (cmp > 0)
    {
        rp = MPZ_REALLOC(r, an);
        gmp_assert_nocarry(mpn_sub(rp, a->_mp_d, an, b->_mp_d, bn));
        return mpn_normalized_size(rp, an);
    }
    else if (cmp < 0)
    {
        rp = MPZ_REALLOC(r, bn);
        gmp_assert_nocarry(mpn_sub(rp, b->_mp_d, bn, a->_mp_d, an));
        return -mpn_normalized_size(rp, bn);
    }
    else
        return 0;
}

void mpz_add(mpz_struct *r, mpz_srcptr a, mpz_srcptr b)
{
    mp_size_t rn;

    if ((a->_mp_size ^ b->_mp_size) >= 0)
        rn = mpz_abs_add(r, a, b);
    else
        rn = mpz_abs_sub(r, a, b);

    r->_mp_size = a->_mp_size >= 0 ? rn : -rn;
}

void mpz_sub(mpz_struct *r, mpz_srcptr a, mpz_srcptr b)
{
    mp_size_t rn;

    if ((a->_mp_size ^ b->_mp_size) >= 0)
        rn = mpz_abs_sub(r, a, b);
    else
        rn = mpz_abs_add(r, a, b);

    r->_mp_size = a->_mp_size >= 0 ? rn : -rn;
}

/* MPZ multiplication */
void mpz_mul_si(mpz_struct *r, mpz_srcptr u, long int v)
{
    if (v < 0)
    {
        mpz_mul_ui(r, u, GMP_NEG_CAST(unsigned long int, v));
        mpz_neg(r, r);
    }
    else
        mpz_mul_ui(r, u, (unsigned long int)v);
}

void mpz_mul_ui(mpz_struct *r, mpz_srcptr u, unsigned long int v)
{
    mp_size_t un, us;
    mp_ptr tp;
    mp_limb_t cy;

    us = u->_mp_size;

    if (us == 0 || v == 0)
    {
        r->_mp_size = 0;
        return;
    }

    un = GMP_ABS(us);

    tp = MPZ_REALLOC(r, un + 1);
    cy = mpn_mul_1(tp, u->_mp_d, un, v);
    tp[un] = cy;

    un += (cy > 0);
    r->_mp_size = (us < 0) ? -un : un;
}

void mpz_mul(mpz_struct *r, mpz_srcptr u, mpz_srcptr v)
{
    int sign;
    mp_size_t un, vn, rn;
    mpz_struct *t = NULL;
    mp_ptr tp;

    un = u->_mp_size;
    vn = v->_mp_size;

    if (un == 0 || vn == 0)
    {
        r->_mp_size = 0;
        return;
    }

    sign = (un ^ vn) < 0;

    un = GMP_ABS(un);
    vn = GMP_ABS(vn);

    mpz_init2(t, (un + vn) * GMP_LIMB_BITS);

    tp = t->_mp_d;
    if (un >= vn)
        mpn_mul(tp, u->_mp_d, un, v->_mp_d, vn);
    else
        mpn_mul(tp, v->_mp_d, vn, u->_mp_d, un);

    rn = un + vn;
    rn -= tp[rn - 1] == 0;

    t->_mp_size = sign ? -rn : rn;
    mpz_swap(r, t);
    mpz_clear(t);
}

void mpz_addmul_ui(mpz_struct *r, mpz_srcptr u, unsigned long int v)
{
    mpz_struct *t = NULL;
    mpz_init(t);
    mpz_mul_ui(t, u, v);
    mpz_add(r, r, t);
    mpz_clear(t);
}

void mpz_submul_ui(mpz_struct *r, mpz_srcptr u, unsigned long int v)
{
    mpz_struct *t = NULL;
    mpz_init(t);
    mpz_mul_ui(t, u, v);
    mpz_sub(r, r, t);
    mpz_clear(t);
}

void mpz_addmul(mpz_struct *r, mpz_srcptr u, mpz_srcptr v)
{
    mpz_struct *t = NULL;
    mpz_init(t);
    mpz_mul(t, u, v);
    mpz_add(r, r, t);
    mpz_clear(t);
}

void mpz_submul(mpz_struct *r, mpz_srcptr u, mpz_srcptr v)
{
    mpz_struct *t = NULL;
    mpz_init(t);
    mpz_mul(t, u, v);
    mpz_sub(r, r, t);
    mpz_clear(t);
}

/* Higher level operations (sqrt, pow and root) */

void mpz_pow_ui(mpz_struct *r, mpz_struct *b, unsigned long e)
{
    unsigned long bit = 0;
    mpz_struct *tr = NULL;
    mpz_init_set_ui(tr, 1);

    bit = GMP_ULONG_HIGHBIT;
    do
    {
        mpz_mul(tr, tr, tr);
        if (e & bit)
            mpz_mul(tr, tr, b);
        bit >>= 1;
    } while (bit > 0);

    mpz_swap(r, tr);
    mpz_clear(tr);
}

void mpz_ui_pow_ui(mpz_struct *r, unsigned long blimb, mp_size_t e)
{
    mpz_struct *b = NULL;
    mpz_pow_ui(r, mpz_roinit_n(b, &blimb, 1), e);
}

void mpz_mul_2exp(mpz_ptr r, mpz_srcptr u, mp_bitcnt_t cnt)
{
    mp_size_t un, rn;
    mp_size_t limb_cnt;
    mp_ptr rp;
    mp_srcptr up;
    mp_limb_t rlimb;

    un = ABSIZ(u);
    limb_cnt = cnt / GMP_NUMB_BITS;
    rn = un + limb_cnt;

    if (un == 0)
        rn = 0;
    else
    {
        rp = MPZ_REALLOC(r, rn + 1);
        up = PTR(u);

        cnt %= GMP_NUMB_BITS;
        if (cnt != 0)
        {
            rlimb = mpn_lshift(rp + limb_cnt, up, un, cnt);
            rp[rn] = rlimb;
            rn += (rlimb != 0);
        }
        else
        {
            MPN_COPY_DECR(rp + limb_cnt, up, un);
        }

        /* Zero all whole limbs at low end.  Do it here and not before calling
       mpn_lshift, not to lose for U == R.  */
        MPN_ZERO(rp, limb_cnt);
    }

    SIZ(r) = SIZ(u) >= 0 ? rn : -rn;
}

static void cfdiv_q_2exp(mpz_ptr w, mpz_srcptr u, mp_bitcnt_t cnt, int dir)
{
    mp_size_t wsize, usize, abs_usize, limb_cnt, i;
    mp_srcptr up;
    mp_ptr wp;
    mp_limb_t round, rmask;

    usize = SIZ(u);
    abs_usize = ABS(usize);
    limb_cnt = cnt / GMP_NUMB_BITS;
    wsize = abs_usize - limb_cnt;
    if (wsize <= 0)
    {
        /* u < 2**cnt, so result 1, 0 or -1 according to rounding */
        MPZ_NEWALLOC(w, 1)
        [0] = 1;
        SIZ(w) = (usize == 0 || (usize ^ dir) < 0 ? 0 : dir);
        return;
    }

    /* +1 limb to allow for mpn_add_1 below */
    wp = MPZ_REALLOC(w, wsize + 1);

    /* Check for rounding if direction matches u sign.
       Set round if we're skipping non-zero limbs.  */
    up = PTR(u);
    round = 0;
    rmask = ((usize ^ dir) >= 0 ? MP_LIMB_T_MAX : 0);
    if (rmask != 0)
        for (i = 0; i < limb_cnt && round == 0; i++)
            round = up[i];

    cnt %= GMP_NUMB_BITS;
    if (cnt != 0)
    {
        round |= rmask & mpn_rshift(wp, up + limb_cnt, wsize, cnt);
        wsize -= (wp[wsize - 1] == 0);
    }
    else
        MPN_COPY_INCR(wp, up + limb_cnt, wsize);

    if (round != 0)
    {
        if (wsize != 0)
        {
            mp_limb_t cy;
            cy = mpn_add_1(wp, wp, wsize, CNST_LIMB(1));
            wp[wsize] = cy;
            wsize += cy;
        }
        else
        {
            /* We shifted something to zero.  */
            wp[0] = 1;
            wsize = 1;
        }
    }
    SIZ(w) = (usize >= 0 ? wsize : -wsize);
}

void mpz_cdiv_q_2exp(mpz_ptr w, mpz_srcptr u, mp_bitcnt_t cnt)
{
    cfdiv_q_2exp(w, u, cnt, 1);
}

void mpz_fdiv_q_2exp(mpz_ptr w, mpz_srcptr u, mp_bitcnt_t cnt)
{
    cfdiv_q_2exp(w, u, cnt, -1);
}

char *mpz_get_str(char *sp, int base, mpz_struct *u)
{
    unsigned bits = 0;
    char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    mp_size_t un = 0;
    size_t i = 0;
    size_t sn = 0;

    if (base > 1)
    {
        if (base <= 36)
            digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        else if (base > 62)
            return NULL;
    }
    else if (base >= -1)
        base = 10;
    else
    {
        base = -base;
        if (base > 36)
            return NULL;
    }

    sn = 1 + mpz_sizeinbase(u, base);
    if (!sp)
        sp = (char *)gmp_xalloc(1 + sn);

    un = GMP_ABS(u->_mp_size);

    if (un == 0)
    {
        sp[0] = '0';
        sp[1] = '\0';
        return sp;
    }

    i = 0;

    if (u->_mp_size < 0)
        sp[i++] = '-';

    bits = mpn_base_power_of_two_p(base);

    if (bits)
        /* Not modified in this case. */
        sn = i + mpn_get_str_bits((unsigned char *)sp + i, bits, u->_mp_d, un);
    else
    {
        mp_limb_t *tp = NULL;

        tp = (mp_limb_t *)gmp_xalloc_limbs(un);
        mpn_copyi(tp, u->_mp_d, un);

        sn = i + mpn_get_str((unsigned char *)sp + i, base, tp, un);
        gmp_free(tp);
    }

    for (; i < sn; i++)
        sp[i] = digits[(unsigned char)sp[i]];

    sp[sn] = '\0';
    return sp;
}
