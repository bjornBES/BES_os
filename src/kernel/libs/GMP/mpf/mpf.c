#include "mpf.h"
#include "libs/GMP/gmp-config.h"
#include "libs/GMP/longlong.h"
#include "ctype.h"
#include "stdlib.h" /* for NULL */
#include "stdio.h"

#include <string.h>

#if HAVE_FLOAT_H
#include <float.h> /* for DBL_MAX */
#endif

#if HAVE_LANGINFO_H
#include <langinfo.h> /* for nl_langinfo */
#endif

#if HAVE_LOCALE_H
#include <locale.h> /* for localeconv */
#endif

mp_size_t __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC(53);

#define digit_value_tab __gmp_digit_value_tab
#define _GNU_SOURCE /* for DECIMAL_POINT in langinfo.h */

/* 1 if we (might) need two limbs for u */
#define U2 (GMP_NUMB_BITS < BITS_PER_ULONG)

void mpf_urandomb(mpf_t rop, gmp_randstate_t rstate, mp_bitcnt_t nbits)
{
  mp_ptr rp;
  mp_size_t nlimbs;
  mp_exp_t exp;
  mp_size_t prec;

  rp = PTR(rop);
  nlimbs = BITS_TO_LIMBS(nbits);
  prec = PREC(rop);

  if (nlimbs > prec + 1 || nlimbs == 0)
  {
    nlimbs = prec + 1;
    nbits = nlimbs * GMP_NUMB_BITS;
  }

  _gmp_rand(rp, rstate, nbits);

  /* If nbits isn't a multiple of GMP_NUMB_BITS, shift up.  */
  if (nbits % GMP_NUMB_BITS != 0)
    mpn_lshift(rp, rp, nlimbs, GMP_NUMB_BITS - nbits % GMP_NUMB_BITS);

  exp = 0;
  while (nlimbs != 0 && rp[nlimbs - 1] == 0)
  {
    nlimbs--;
    exp--;
  }
  EXP(rop) = exp;
  SIZ(rop) = nlimbs;
}

void mpf_reldiff(mpf_t rdiff, mpf_srcptr x, mpf_srcptr y)
{
  if (UNLIKELY(SIZ(x) == 0))
  {
    mpf_set_ui(rdiff, (unsigned long int)(mpf_sgn(y) != 0));
  }
  else
  {
    mp_size_t dprec;
    __mpf_struct d;
    TMP_DECL;

    TMP_MARK;
    dprec = PREC(rdiff) + ABSIZ(x);
    ASSERT(PREC(rdiff) + 1 == dprec - ABSIZ(x) + 1);

    (&d)->_mp_prec = dprec;
    (&d)->_mp_d = TMP_ALLOC_LIMBS(dprec + 1);

    mpf_sub(&d, x, y);
    (&d)->_mp_size = ABS((&d)->_mp_size);
    mpf_div(rdiff, &d, x);

    TMP_FREE;
  }
}

void mpf_add_ui(mpf_ptr sum, mpf_srcptr u, unsigned long int v)
{
  mp_srcptr up = u->_mp_d;
  mp_ptr sump = sum->_mp_d;
  mp_size_t usize, sumsize;
  mp_size_t prec = sum->_mp_prec;
  mp_exp_t uexp = u->_mp_exp;

  usize = u->_mp_size;
  if (usize <= 0)
  {
    if (usize == 0)
    {
      mpf_set_ui(sum, v);
      return;
    }
    else
    {
      __mpf_struct u_negated;
      u_negated._mp_size = -usize;
      u_negated._mp_exp = u->_mp_exp;
      u_negated._mp_d = u->_mp_d;
      mpf_sub_ui(sum, &u_negated, v);
      sum->_mp_size = -(sum->_mp_size);
      return;
    }
  }

  if (v == 0)
  {
  sum_is_u:
    if (u != sum)
    {
      sumsize = MIN(usize, prec + 1);
      MPN_COPY(sum->_mp_d, up + usize - sumsize, sumsize);
      sum->_mp_size = sumsize;
      sum->_mp_exp = u->_mp_exp;
    }
    return;
  }

  if (uexp > 0)
  {
    /* U >= 1.  */
    if (uexp > prec)
    {
      /* U >> V, V is not part of final result.  */
      goto sum_is_u;
    }
    else
    {
      /* U's "limb point" is somewhere between the first limb
         and the PREC:th limb.
         Both U and V are part of the final result.  */
      if (uexp > usize)
      {
        /*   uuuuuu0000. */
        /* +          v. */
        /* We begin with moving U to the top of SUM, to handle
     samevar(U,SUM).  */
        MPN_COPY_DECR(sump + uexp - usize, up, usize);
        sump[0] = v;
        MPN_ZERO(sump + 1, uexp - usize - 1);
#if 0 /* What is this??? */
	      if (sum == u)
		MPN_COPY (sum->_mp_d, sump, uexp);
#endif
        sum->_mp_size = uexp;
        sum->_mp_exp = uexp;
      }
      else
      {
        /*   uuuuuu.uuuu */
        /* +      v.     */
        mp_limb_t cy_limb;
        if (usize > prec)
        {
          /* Ignore excess limbs in U.  */
          up += usize - prec;
          usize -= usize - prec; /* Eq. usize = prec */
        }
        if (sump != up)
          MPN_COPY_INCR(sump, up, usize - uexp);
        cy_limb = mpn_add_1(sump + usize - uexp, up + usize - uexp,
                            uexp, (mp_limb_t)v);
        sump[usize] = cy_limb;
        sum->_mp_size = usize + cy_limb;
        sum->_mp_exp = uexp + cy_limb;
      }
    }
  }
  else
  {
    /* U < 1, so V > U for sure.  */
    /* v.         */
    /*  .0000uuuu */
    if ((-uexp) >= prec)
    {
      sump[0] = v;
      sum->_mp_size = 1;
      sum->_mp_exp = 1;
    }
    else
    {
      if (usize + (-uexp) + 1 > prec)
      {
        /* Ignore excess limbs in U.  */
        up += usize + (-uexp) + 1 - prec;
        usize -= usize + (-uexp) + 1 - prec;
      }
      if (sump != up)
        MPN_COPY_INCR(sump, up, usize);
      MPN_ZERO(sump + usize, -uexp);
      sump[usize + (-uexp)] = v;
      sum->_mp_size = usize + (-uexp) + 1;
      sum->_mp_exp = 1;
    }
  }
}

void mpf_init_set_si(mpf_ptr r, long int val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  mp_size_t size;
  mp_limb_t vl;

  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);

  vl = (mp_limb_t)ABS_CAST(unsigned long int, val);

  r->_mp_d[0] = vl & GMP_NUMB_MASK;
  size = vl != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  vl >>= GMP_NUMB_BITS;
  r->_mp_d[1] = vl;
  size += (vl != 0);
#endif

  r->_mp_exp = size;
  r->_mp_size = val >= 0 ? size : -size;
}

long mpf_get_si(mpf_srcptr f) 
{
  mp_exp_t exp;
  mp_size_t size, abs_size;
  mp_srcptr fp;
  mp_limb_t fl;

  exp = EXP(f);
  size = SIZ(f);
  fp = PTR(f);

  /* fraction alone truncates to zero
     this also covers zero, since we have exp==0 for zero */
  if (exp <= 0)
    return 0L;

  /* there are some limbs above the radix point */

  fl = 0;
  abs_size = ABS(size);
  if (abs_size >= exp)
    fl = fp[abs_size - exp];

#if BITS_PER_ULONG > GMP_NUMB_BITS
  if (exp > 1 && abs_size + 1 >= exp)
    fl |= fp[abs_size - exp + 1] << GMP_NUMB_BITS;
#endif

  if (size > 0)
    return fl & LONG_MAX;
  else
    /* this form necessary to correctly handle -0x80..00 */
    return -1 - (long)((fl - 1) & LONG_MAX);
}

void mpf_set_z(mpf_ptr r, mpz_srcptr u)
{
  mp_ptr rp, up;
  mp_size_t size, asize;
  mp_size_t prec;

  prec = PREC(r) + 1;
  size = SIZ(u);
  asize = ABS(size);
  rp = PTR(r);
  up = PTR(u);

  EXP(r) = asize;

  if (asize > prec)
  {
    up += asize - prec;
    asize = prec;
  }

  SIZ(r) = size >= 0 ? asize : -asize;
  MPN_COPY(rp, up, asize);
}

void mpf_ceil_or_floor(mpf_ptr r, mpf_srcptr u, int dir)
{
  mp_ptr rp, up, p;
  mp_size_t size, asize, prec;
  mp_exp_t exp;

  size = SIZ(u);
  if (size == 0)
  {
  zero:
    SIZ(r) = 0;
    EXP(r) = 0;
    return;
  }

  rp = PTR(r);
  exp = EXP(u);
  if (exp <= 0)
  {
    /* u is only a fraction */
    if ((size ^ dir) < 0)
      goto zero;
    rp[0] = 1;
    EXP(r) = 1;
    SIZ(r) = dir;
    return;
  }
  EXP(r) = exp;

  up = PTR(u);
  asize = ABS(size);
  up += asize;

  /* skip fraction part of u */
  asize = MIN(asize, exp);

  /* don't lose precision in the copy */
  prec = PREC(r) + 1;

  /* skip excess over target precision */
  asize = MIN(asize, prec);

  up -= asize;

  if ((size ^ dir) >= 0)
  {
    /* rounding direction matches sign, must increment if ignored part is
       non-zero */
    for (p = PTR(u); p != up; p++)
    {
      if (*p != 0)
      {
        if (mpn_add_1(rp, up, asize, CNST_LIMB(1)))
        {
          /* was all 0xFF..FFs, which have become zeros, giving just
             a carry */
          rp[0] = 1;
          asize = 1;
          EXP(r)
          ++;
        }
        SIZ(r) = (size >= 0 ? asize : -asize);
        return;
      }
    }
  }

  SIZ(r) = (size >= 0 ? asize : -asize);
  if (rp != up)
    MPN_COPY_INCR(rp, up, asize);
}

void mpf_ceil(mpf_ptr r, mpf_srcptr u)
{
  mpf_ceil_or_floor(r, u, 1);
}

void mpf_floor(mpf_ptr r, mpf_srcptr u)
{
  mpf_ceil_or_floor(r, u, -1);
}

void mpf_inits(mpf_ptr x, ...)
{
  va_list ap;

  va_start(ap, x);

  do
  {
    mpf_init(x);
    x = va_arg(ap, mpf_ptr);
  } while (x != NULL);

  va_end(ap);
}

mp_size_t mpn_pow_1_highpart(mp_ptr rp, mp_size_t *ignp, mp_limb_t base, mp_exp_t exp, mp_size_t prec, mp_ptr tp)
{
  mp_size_t ign; /* counts number of ignored low limbs in r */
  mp_size_t off; /* keeps track of offset where value starts */
  mp_ptr passed_rp = rp;
  mp_size_t rn;
  int cnt;
  int i;

  if (exp == 0)
  {
    rp[0] = 1;
    *ignp = 0;
    return 1;
  }

  rp[0] = base;
  rn = 1;
  off = 0;
  ign = 0;
  count_leading_zeros(cnt, exp);
  for (i = GMP_LIMB_BITS - cnt - 2; i >= 0; i--)
  {
    mpn_sqr(tp, rp + off, rn);
    rn = 2 * rn;
    rn -= tp[rn - 1] == 0;
    ign <<= 1;

    off = 0;
    if (rn > prec)
    {
      ign += rn - prec;
      off = rn - prec;
      rn = prec;
    }
    MP_PTR_SWAP(rp, tp);

    if (((exp >> i) & 1) != 0)
    {
      mp_limb_t cy;
      cy = mpn_mul_1(rp, rp + off, rn, base);
      rp[rn] = cy;
      rn += cy != 0;
      off = 0;
    }
  }

  if (rn > prec)
  {
    ign += rn - prec;
    rp += rn - prec;
    rn = prec;
  }

  MPN_COPY_INCR(passed_rp, rp + off, rn);
  *ignp = ign;
  return rn;
}

int mpf_set_str(mpf_ptr x, const char *str, int base)
{
  size_t str_size;
  char *s, *begs;
  size_t i, j;
  int c;
  int negative;
  char *dotpos;
  const char *expptr;
  int exp_base;
  const char *point = GMP_DECIMAL_POINT;
  size_t pointlen = strlen(point);
  const unsigned char *digit_value;
  int incr;
  size_t n_zeros_skipped;

  TMP_DECL;

  c = (unsigned char)*str;

  /* Skip whitespace.  */
  while (isspace(c))
    c = (unsigned char)*++str;

  negative = 0;
  if (c == '-')
  {
    negative = 1;
    c = (unsigned char)*++str;
  }

  /* Default base to decimal.  */
  if (base == 0)
    base = 10;

  exp_base = base;

  if (base < 0)
  {
    exp_base = 10;
    base = -base;
  }

  digit_value = digit_value_tab;
  if (base > 36)
  {
    /* For bases > 36, use the collating sequence
 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.  */
    digit_value += 208;
    if (base > 62)
      return -1; /* too large base */
  }

  /* Require at least one digit, possibly after an initial decimal point.  */
  if (digit_value[c] >= base)
  {
    /* not a digit, must be a decimal point */
    for (i = 0; i < pointlen; i++)
      if (str[i] != point[i])
        return -1;
    if (digit_value[(unsigned char)str[pointlen]] >= base)
      return -1;
  }

  /* Locate exponent part of the input.  Look from the right of the string,
     since the exponent is usually a lot shorter than the mantissa.  */
  expptr = NULL;
  str_size = strlen(str);
  for (i = str_size - 1; i > 0; i--)
  {
    c = (unsigned char)str[i];
    if (c == '@' || (base <= 10 && (c == 'e' || c == 'E')))
    {
      expptr = str + i + 1;
      str_size = i;
      break;
    }
  }

  TMP_MARK;
  s = begs = (char *)TMP_ALLOC(str_size + 1);

  incr = 0;
  n_zeros_skipped = 0;
  dotpos = NULL;

  /* Loop through mantissa, converting it from ASCII to raw byte values.  */
  for (i = 0; i < str_size; i++)
  {
    c = (unsigned char)*str;
    if (!isspace(c))
    {
      int dig;

      for (j = 0; j < pointlen; j++)
        if (str[j] != point[j])
          goto not_point;
      if (1)
      {
        if (dotpos != 0)
        {
          /* already saw a decimal point, another is invalid */
          TMP_FREE;
          return -1;
        }
        dotpos = s;
        str += pointlen - 1;
        i += pointlen - 1;
      }
      else
      {
      not_point:
        dig = digit_value[c];
        if (dig >= base)
        {
          TMP_FREE;
          return -1;
        }
        *s = dig;
        incr |= dig != 0;
        s += incr; /* Increment after first non-0 digit seen. */
        if (dotpos != NULL)
          /* Count skipped zeros between radix point and first non-0
             digit. */
          n_zeros_skipped += 1 - incr;
      }
    }
    c = (unsigned char)*++str;
  }

  str_size = s - begs;

  {
    long exp_in_base;
    mp_size_t ra, ma, rn, mn;
    int cnt;
    mp_ptr mp, tp, rp;
    mp_exp_t exp_in_limbs;
    mp_size_t prec = PREC(x) + 1;
    int divflag;
    mp_size_t madj, radj;

#if 0
    size_t n_chars_needed;

    /* This needs careful testing.  Leave disabled for now.  */
    /* Just consider the relevant leading digits of the mantissa.  */
    LIMBS_PER_DIGIT_IN_BASE (n_chars_needed, prec, base);
    if (str_size > n_chars_needed)
      str_size = n_chars_needed;
#endif

    if (str_size == 0)
    {
      SIZ(x) = 0;
      EXP(x) = 0;
      TMP_FREE;
      return 0;
    }

    LIMBS_PER_DIGIT_IN_BASE(ma, str_size, base);
    mp = TMP_ALLOC_LIMBS(ma);
    mn = mpn_set_str(mp, (unsigned char *)begs, str_size, base);

    madj = 0;
    /* Ignore excess limbs in MP,MSIZE.  */
    if (mn > prec)
    {
      madj = mn - prec;
      mp += mn - prec;
      mn = prec;
    }

    if (expptr != 0)
    {
      /* Scan and convert the exponent, in base exp_base.  */
      long dig, minus, plusminus;
      c = (unsigned char)*expptr;
      minus = -(long)(c == '-');
      plusminus = minus | -(long)(c == '+');
      expptr -= plusminus; /* conditional increment */
      c = (unsigned char)*expptr++;
      dig = digit_value[c];
      if (dig >= exp_base)
      {
        TMP_FREE;
        return -1;
      }
      exp_in_base = dig;
      c = (unsigned char)*expptr++;
      dig = digit_value[c];
      while (dig < exp_base)
      {
        exp_in_base = exp_in_base * exp_base;
        exp_in_base += dig;
        c = (unsigned char)*expptr++;
        dig = digit_value[c];
      }
      exp_in_base = (exp_in_base ^ minus) - minus; /* conditional negation */
    }
    else
      exp_in_base = 0;
    if (dotpos != 0)
      exp_in_base -= s - dotpos + n_zeros_skipped;
    divflag = exp_in_base < 0;
    exp_in_base = ABS(exp_in_base);

    if (exp_in_base == 0)
    {
      MPN_COPY(PTR(x), mp, mn);
      SIZ(x) = negative ? -mn : mn;
      EXP(x) = mn + madj;
      TMP_FREE;
      return 0;
    }

    ra = 2 * (prec + 1);
    TMP_ALLOC_LIMBS_2(rp, ra, tp, ra);
    rn = mpn_pow_1_highpart(rp, &radj, (mp_limb_t)base, exp_in_base, prec, tp);

    if (divflag)
    {
#if 0
	/* FIXME: Should use mpn_div_q here.  */
	...
	mpn_div_q (tp, mp, mn, rp, rn, scratch);
	...
#else
      mp_ptr qp;
      mp_limb_t qlimb;
      if (mn < rn)
      {
        /* Pad out MP,MSIZE for current divrem semantics.  */
        mp_ptr tmp = TMP_ALLOC_LIMBS(rn + 1);
        MPN_ZERO(tmp, rn - mn);
        MPN_COPY(tmp + rn - mn, mp, mn);
        mp = tmp;
        madj -= rn - mn;
        mn = rn;
      }
      if ((rp[rn - 1] & GMP_NUMB_HIGHBIT) == 0)
      {
        mp_limb_t cy;
        count_leading_zeros(cnt, rp[rn - 1]);
        cnt -= GMP_NAIL_BITS;
        mpn_lshift(rp, rp, rn, cnt);
        cy = mpn_lshift(mp, mp, mn, cnt);
        if (cy)
          mp[mn++] = cy;
      }

      qp = TMP_ALLOC_LIMBS(prec + 1);
      qlimb = mpn_divrem(qp, prec - (mn - rn), mp, mn, rp, rn);
      tp = qp;
      exp_in_limbs = qlimb + (mn - rn) + (madj - radj);
      rn = prec;
      if (qlimb != 0)
      {
        tp[prec] = qlimb;
        /* Skip the least significant limb not to overrun the destination
           variable.  */
        tp++;
      }
#endif
    }
    else
    {
      tp = TMP_ALLOC_LIMBS(rn + mn);
      if (rn > mn)
        mpn_mul(tp, rp, rn, mp, mn);
      else
        mpn_mul(tp, mp, mn, rp, rn);
      rn += mn;
      rn -= tp[rn - 1] == 0;
      exp_in_limbs = rn + madj + radj;

      if (rn > prec)
      {
        tp += rn - prec;
        rn = prec;
        exp_in_limbs += 0;
      }
    }

    MPN_COPY(PTR(x), tp, rn);
    SIZ(x) = negative ? -rn : rn;
    EXP(x) = exp_in_limbs;
    TMP_FREE;
    return 0;
  }
}

void mpf_init_set_ui(mpf_ptr r, unsigned long int val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  mp_size_t size;

  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);
  r->_mp_d[0] = val & GMP_NUMB_MASK;
  size = (val != 0);

#if BITS_PER_ULONG > GMP_NUMB_BITS
  val >>= GMP_NUMB_BITS;
  r->_mp_d[1] = val;
  size += (val != 0);
#endif

  r->_mp_size = size;
  r->_mp_exp = size;
}

void mpf_set_si(mpf_ptr dest, long val)
{
  mp_size_t size;
  mp_limb_t vl;

  vl = (mp_limb_t)ABS_CAST(unsigned long int, val);

  dest->_mp_d[0] = vl & GMP_NUMB_MASK;
  size = vl != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  vl >>= GMP_NUMB_BITS;
  dest->_mp_d[1] = vl;
  size += (vl != 0);
#endif

  dest->_mp_exp = size;
  dest->_mp_size = val >= 0 ? size : -size;
}

void mpf_init(mpf_ptr r)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);
}

int mpf_cmp_d(mpf_srcptr f, double d)
{
  mp_limb_t darray[LIMBS_PER_DOUBLE];
  __mpf_struct df;

  /* d=NaN has no sensible return value, so raise an exception.
     d=Inf or -Inf is always bigger than z.  */
  DOUBLE_NAN_INF_ACTION(d,
                        __gmp_invalid_operation(),
                        return (d < 0.0 ? 1 : -1));

  if (d == 0.0)
    return SIZ(f);

  (&df)->_mp_d = darray;
  (&df)->_mp_size = (d >= 0.0 ? LIMBS_PER_DOUBLE : -LIMBS_PER_DOUBLE);
  (&df)->_mp_exp = __gmp_extract_double(darray, ABS(d));

  return mpf_cmp(f, &df);
}

int mpf_eq(mpf_srcptr u, mpf_srcptr v, mp_bitcnt_t n_bits)
{
  mp_srcptr up, vp, p;
  mp_size_t usize, vsize, minsize, maxsize, n_limbs, i, size;
  mp_exp_t uexp, vexp;
  mp_limb_t diff;
  int cnt;

  uexp = u->_mp_exp;
  vexp = v->_mp_exp;

  usize = u->_mp_size;
  vsize = v->_mp_size;

  /* 1. Are the signs different?  */
  if ((usize ^ vsize) >= 0)
  {
    /* U and V are both non-negative or both negative.  */
    if (usize == 0)
      return vsize == 0;
    if (vsize == 0)
      return 0;

    /* Fall out.  */
  }
  else
  {
    /* Either U or V is negative, but not both.  */
    return 0;
  }

  /* U and V have the same sign and are both non-zero.  */

  /* 2. Are the exponents different?  */
  if (uexp != vexp)
    return 0;

  usize = ABS(usize);
  vsize = ABS(vsize);

  up = u->_mp_d;
  vp = v->_mp_d;

  up += usize; /* point just above most significant limb */
  vp += vsize; /* point just above most significant limb */

  count_leading_zeros(cnt, up[-1]);
  if ((vp[-1] >> (GMP_LIMB_BITS - 1 - cnt)) != 1)
    return 0; /* msb positions different */

  n_bits += cnt - GMP_NAIL_BITS;
  n_limbs = (n_bits + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;

  usize = MIN(usize, n_limbs);
  vsize = MIN(vsize, n_limbs);

#if 0
  /* Ignore zeros at the low end of U and V.  */
  while (up[0] == 0)
    up++, usize--;
  while (vp[0] == 0)
    vp++, vsize--;
#endif

  minsize = MIN(usize, vsize);
  maxsize = usize + vsize - minsize;

  up -= minsize; /* point at most significant common limb */
  vp -= minsize; /* point at most significant common limb */

  /* Compare the most significant part which has explicit limbs for U and V. */
  for (i = minsize - 1; i > 0; i--)
  {
    if (up[i] != vp[i])
      return 0;
  }

  n_bits -= (maxsize - 1) * GMP_NUMB_BITS;

  size = maxsize - minsize;
  if (size != 0)
  {
    if (up[0] != vp[0])
      return 0;

    /* Now either U or V has its limbs consumed, i.e, continues with an
 infinite number of implicit zero limbs.  Check that the other operand
 has just zeros in the corresponding, relevant part.  */

    if (usize > vsize)
      p = up - size;
    else
      p = vp - size;

    for (i = size - 1; i > 0; i--)
    {
      if (p[i] != 0)
        return 0;
    }

    diff = p[0];
  }
  else
  {
    /* Both U or V has its limbs consumed.  */

    diff = up[0] ^ vp[0];
  }

  if (n_bits < GMP_NUMB_BITS)
    diff >>= GMP_NUMB_BITS - n_bits;

  return diff == 0;
}

void mpf_mul(mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_size_t sign_product;
  mp_size_t prec = r->_mp_prec;
  mp_size_t rsize;
  mp_limb_t cy_limb;
  mp_ptr rp, tp;
  mp_size_t adj;
  TMP_DECL;

  if (u == v)
  {
    mp_srcptr up;
    mp_size_t usize;

    usize = u->_mp_size;
    sign_product = 0;

    usize = ABS(usize);

    up = u->_mp_d;
    if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }

    if (usize == 0)
    {
      r->_mp_size = 0;
      r->_mp_exp = 0; /* ??? */
      return;
    }
    else
    {
      TMP_MARK;
      rsize = 2 * usize;
      tp = TMP_ALLOC_LIMBS(rsize);

      mpn_sqr(tp, up, usize);
      cy_limb = tp[rsize - 1];
    }
  }
  else
  {
    mp_srcptr up, vp;
    mp_size_t usize, vsize;

    usize = u->_mp_size;
    vsize = v->_mp_size;
    sign_product = usize ^ vsize;

    usize = ABS(usize);
    vsize = ABS(vsize);

    up = u->_mp_d;
    vp = v->_mp_d;
    if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }
    if (vsize > prec)
    {
      vp += vsize - prec;
      vsize = prec;
    }

    if (usize == 0 || vsize == 0)
    {
      r->_mp_size = 0;
      r->_mp_exp = 0;
      return;
    }
    else
    {
      TMP_MARK;
      rsize = usize + vsize;
      tp = TMP_ALLOC_LIMBS(rsize);
      cy_limb = (usize >= vsize
                     ? mpn_mul(tp, up, usize, vp, vsize)
                     : mpn_mul(tp, vp, vsize, up, usize));
    }
  }

  adj = cy_limb == 0;
  rsize -= adj;
  prec++;
  if (rsize > prec)
  {
    tp += rsize - prec;
    rsize = prec;
  }
  rp = r->_mp_d;
  MPN_COPY(rp, tp, rsize);
  r->_mp_exp = u->_mp_exp + v->_mp_exp - adj;
  r->_mp_size = sign_product >= 0 ? rsize : -rsize;

  TMP_FREE;
}

void mpf_init2(mpf_ptr r, mp_bitcnt_t prec_in_bits)
{
  mp_size_t prec;

  prec = __GMPF_BITS_TO_PREC(prec_in_bits);
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);
}

void mpf_trunc(mpf_ptr r, mpf_srcptr u)
{
  mp_ptr rp;
  mp_srcptr up;
  mp_size_t size, asize, prec;
  mp_exp_t exp;

  exp = EXP(u);
  size = SIZ(u);
  if (size == 0 || exp <= 0)
  {
    /* u is only a fraction */
    SIZ(r) = 0;
    EXP(r) = 0;
    return;
  }

  up = PTR(u);
  EXP(r) = exp;
  asize = ABS(size);
  up += asize;

  /* skip fraction part of u */
  asize = MIN(asize, exp);

  /* don't lose precision in the copy */
  prec = PREC(r) + 1;

  /* skip excess over target precision */
  asize = MIN(asize, prec);

  up -= asize;
  rp = PTR(r);
  SIZ(r) = (size >= 0 ? asize : -asize);
  if (rp != up)
    MPN_COPY_INCR(rp, up, asize);
}

void mpf_set_default_prec(mp_bitcnt_t prec_in_bits) 
{
  __gmp_default_fp_limb_precision = __GMPF_BITS_TO_PREC(prec_in_bits);
}

void mpf_init_set(mpf_ptr r, mpf_srcptr s)
{
  mp_ptr rp, sp;
  mp_size_t ssize, size;
  mp_size_t prec;

  prec = __gmp_default_fp_limb_precision;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);
  r->_mp_prec = prec;

  prec++; /* lie not to lose precision in assignment */
  ssize = s->_mp_size;
  size = ABS(ssize);

  rp = r->_mp_d;
  sp = s->_mp_d;

  if (size > prec)
  {
    sp += size - prec;
    size = prec;
  }

  r->_mp_exp = s->_mp_exp;
  r->_mp_size = ssize >= 0 ? size : -size;

  MPN_COPY(rp, sp, size);
}

char * mpf_get_str(char *dbuf, mp_exp_t *exp, int base, size_t n_digits, mpf_srcptr u)
{
  mp_exp_t ue;
  mp_size_t n_limbs_needed;
  size_t max_digits;
  mp_ptr up, pp, tp;
  mp_size_t un, pn, tn;
  unsigned char *tstr;
  mp_exp_t exp_in_base;
  size_t n_digits_computed;
  mp_size_t i;
  const char *num_to_text;
  size_t alloc_size = 0;
  char *dp;
  TMP_DECL;

  up = PTR(u);
  un = ABSIZ(u);
  ue = EXP(u);

  num_to_text = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  if (base > 1)
  {
    if (base <= 36)
      num_to_text = "0123456789abcdefghijklmnopqrstuvwxyz";
    else if (UNLIKELY(base > 62))
      return NULL;
  }
  else if (base > -2)
  {
    base = 10;
  }
  else
  {
    base = -base;
    if (UNLIKELY(base > 36))
      return NULL;
  }

  MPF_SIGNIFICANT_DIGITS(max_digits, base, PREC(u));
  if (n_digits == 0 || n_digits > max_digits)
    n_digits = max_digits;

  if (dbuf == 0)
  {
    /* We didn't get a string from the user.  Allocate one (and return
 a pointer to it) with space for `-' and terminating null.  */
    alloc_size = n_digits + 2;
    dbuf = __GMP_ALLOCATE_FUNC_TYPE(n_digits + 2, char);
  }

  if (un == 0)
  {
    *exp = 0;
    *dbuf = 0;
    n_digits = 0;
    goto done;
  }

  TMP_MARK;

  /* Allocate temporary digit space.  We can't put digits directly in the user
     area, since we generate more digits than requested.  (We allocate
     2 * GMP_LIMB_BITS extra bytes because of the digit block nature of the
     conversion.)  */
  tstr = (unsigned char *)TMP_ALLOC(n_digits + 2 * GMP_LIMB_BITS + 3);

  LIMBS_PER_DIGIT_IN_BASE(n_limbs_needed, n_digits, base);

  if (un > n_limbs_needed)
  {
    up += un - n_limbs_needed;
    un = n_limbs_needed;
  }

  TMP_ALLOC_LIMBS_2(pp, 2 * n_limbs_needed + 4,
                    tp, 2 * n_limbs_needed + 4);

  if (ue <= n_limbs_needed)
  {
    /* We need to multiply number by base^n to get an n_digits integer part.  */
    mp_size_t n_more_limbs_needed, ign, off;
    unsigned long e;

    n_more_limbs_needed = n_limbs_needed - ue;
    DIGITS_IN_BASE_PER_LIMB(e, n_more_limbs_needed, base);

    pn = mpn_pow_1_highpart(pp, &ign, (mp_limb_t)base, e, n_limbs_needed + 1, tp);
    if (un > pn)
      mpn_mul(tp, up, un, pp, pn); /* FIXME: mpn_mul_highpart */
    else
      mpn_mul(tp, pp, pn, up, un); /* FIXME: mpn_mul_highpart */
    tn = un + pn;
    tn -= tp[tn - 1] == 0;
    off = un - ue - ign;
    if (off < 0)
    {
      MPN_COPY_DECR(tp - off, tp, tn);
      MPN_ZERO(tp, -off);
      tn -= off;
      off = 0;
    }
    n_digits_computed = mpn_get_str(tstr, base, tp + off, tn - off);

    exp_in_base = n_digits_computed - e;
  }
  else
  {
    /* We need to divide number by base^n to get an n_digits integer part.  */
    mp_size_t n_less_limbs_needed, ign, off, xn;
    unsigned long e;
    mp_ptr dummyp, xp;

    n_less_limbs_needed = ue - n_limbs_needed;
    DIGITS_IN_BASE_PER_LIMB(e, n_less_limbs_needed, base);

    pn = mpn_pow_1_highpart(pp, &ign, (mp_limb_t)base, e, n_limbs_needed + 1, tp);

    xn = n_limbs_needed + (n_less_limbs_needed - ign);
    xp = TMP_ALLOC_LIMBS(xn);
    off = xn - un;
    MPN_ZERO(xp, off);
    MPN_COPY(xp + off, up, un);

    dummyp = TMP_ALLOC_LIMBS(pn);
    mpn_tdiv_qr(tp, dummyp, (mp_size_t)0, xp, xn, pp, pn);
    tn = xn - pn + 1;
    tn -= tp[tn - 1] == 0;
    n_digits_computed = mpn_get_str(tstr, base, tp, tn);

    exp_in_base = n_digits_computed + e;
  }

  /* We should normally have computed too many digits.  Round the result
     at the point indicated by n_digits.  */
  if (n_digits_computed > n_digits)
  {
    size_t i;
    /* Round the result.  */
    if (tstr[n_digits] * 2 >= base)
    {
      n_digits_computed = n_digits;
      for (i = n_digits - 1;; i--)
      {
        unsigned int x;
        x = ++(tstr[i]);
        if (x != base)
          break;
        n_digits_computed--;
        if (i == 0)
        {
          /* We had something like `bbbbbbb...bd', where 2*d >= base
             and `b' denotes digit with significance base - 1.
             This rounds up to `1', increasing the exponent.  */
          tstr[0] = 1;
          n_digits_computed = 1;
          exp_in_base++;
          break;
        }
      }
    }
  }

  /* We might have fewer digits than requested as a result of rounding above,
     (i.e. 0.999999 => 1.0) or because we have a number that simply doesn't
     need many digits in this base (e.g., 0.125 in base 10).  */
  if (n_digits > n_digits_computed)
    n_digits = n_digits_computed;

  /* Remove trailing 0.  There can be many zeros.  */
  while (n_digits != 0 && tstr[n_digits - 1] == 0)
    n_digits--;

  dp = dbuf + (SIZ(u) < 0);

  /* Translate to ASCII and copy to result string.  */
  for (i = 0; i < n_digits; i++)
    dp[i] = num_to_text[tstr[i]];
  dp[n_digits] = 0;

  *exp = exp_in_base;

  if (SIZ(u) < 0)
  {
    dbuf[0] = '-';
    n_digits++;
  }

  TMP_FREE;

done:
  /* If the string was alloced then resize it down to the actual space
     required.  */
  if (alloc_size != 0)
  {
    __GMP_REALLOCATE_FUNC_MAYBE_TYPE(dbuf, alloc_size, n_digits + 1, char);
  }

  return dbuf;
}

void mpf_set_prec(mpf_ptr x, mp_bitcnt_t new_prec_in_bits)
{
  mp_size_t old_prec, new_prec, new_prec_plus1;
  mp_size_t size, sign;
  mp_ptr xp;

  new_prec = __GMPF_BITS_TO_PREC(new_prec_in_bits);
  old_prec = PREC(x);

  /* do nothing if already the right precision */
  if (new_prec == old_prec)
    return;

  PREC(x) = new_prec;
  new_prec_plus1 = new_prec + 1;

  /* retain most significant limbs */
  sign = SIZ(x);
  size = ABS(sign);
  xp = PTR(x);
  if (size > new_prec_plus1)
  {
    SIZ(x) = (sign >= 0 ? new_prec_plus1 : -new_prec_plus1);
    MPN_COPY_INCR(xp, xp + size - new_prec_plus1, new_prec_plus1);
  }

  PTR(x) = __GMP_REALLOCATE_FUNC_LIMBS(xp, old_prec + 1, new_prec_plus1);
}
/* mpf_integer_p -- test whether an mpf is an integer */

int mpf_integer_p(mpf_srcptr f) 
{
  mp_srcptr fp;
  mp_exp_t exp;
  mp_size_t size;

  size = SIZ(f);
  exp = EXP(f);
  if (exp <= 0)
    return (size == 0); /* zero is an integer,
         others have only fraction limbs */
  size = ABS(size);

  /* Ignore zeroes at the low end of F.  */
  for (fp = PTR(f); *fp == 0; ++fp)
    --size;

  /* no fraction limbs */
  return size <= exp;
}

void mpf_swap(mpf_ptr u, mpf_ptr v) 
{
  mp_ptr tptr;
  mp_size_t tprec;
  mp_size_t tsiz;
  mp_exp_t texp;

  tprec = PREC(u);
  PREC(u) = PREC(v);
  PREC(v) = tprec;

  tsiz = SIZ(u);
  SIZ(u) = SIZ(v);
  SIZ(v) = tsiz;

  texp = EXP(u);
  EXP(u) = EXP(v);
  EXP(v) = texp;

  tptr = PTR(u);
  PTR(u) = PTR(v);
  PTR(v) = tptr;
}

mp_bitcnt_t mpf_get_default_prec(void) 
{
  return __GMPF_PREC_TO_BITS(__gmp_default_fp_limb_precision);
}

void mpf_mul_2exp(mpf_ptr r, mpf_srcptr u, mp_bitcnt_t exp)
{
  mp_srcptr up;
  mp_ptr rp = r->_mp_d;
  mp_size_t usize;
  mp_size_t abs_usize;
  mp_size_t prec = r->_mp_prec;
  mp_exp_t uexp = u->_mp_exp;

  usize = u->_mp_size;

  if (UNLIKELY(usize == 0))
  {
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

  abs_usize = ABS(usize);
  up = u->_mp_d;

  if (exp % GMP_NUMB_BITS == 0)
  {
    prec++; /* retain more precision here as we don't need
     to account for carry-out here */
    if (abs_usize > prec)
    {
      up += abs_usize - prec;
      abs_usize = prec;
    }
    if (rp != up)
      MPN_COPY_INCR(rp, up, abs_usize);
    r->_mp_exp = uexp + exp / GMP_NUMB_BITS;
  }
  else
  {
    mp_limb_t cy_limb;
    mp_size_t adj;
    if (abs_usize > prec)
    {
      up += abs_usize - prec;
      abs_usize = prec;
      /* Use mpn_rshift since mpn_lshift operates downwards, and we
         therefore would clobber part of U before using that part, in case
         R is the same variable as U.  */
      cy_limb = mpn_rshift(rp + 1, up, abs_usize,
                           GMP_NUMB_BITS - exp % GMP_NUMB_BITS);
      rp[0] = cy_limb;
      adj = rp[abs_usize] != 0;
    }
    else
    {
      cy_limb = mpn_lshift(rp, up, abs_usize, exp % GMP_NUMB_BITS);
      rp[abs_usize] = cy_limb;
      adj = cy_limb != 0;
    }

    abs_usize += adj;
    r->_mp_exp = uexp + exp / GMP_NUMB_BITS + adj;
  }
  r->_mp_size = usize >= 0 ? abs_usize : -abs_usize;
}

size_t mpf_size(mpf_srcptr f) 
{
  return __GMP_ABS(f->_mp_size);
}

mp_bitcnt_t mpf_get_prec(mpf_srcptr x) 
{
  return __GMPF_PREC_TO_BITS(x->_mp_prec);
}

size_t mpf_inp_str(mpf_ptr rop, fd_t stream, int base)
{
  char *str;
  size_t alloc_size, str_size;
  int c;
  int res;
  size_t nread;

  if (stream == 0)
    stream = stdin;

  alloc_size = 100;
  str = __GMP_ALLOCATE_FUNC_TYPE(alloc_size, char);
  str_size = 0;
  nread = 0;

  /* Skip whitespace.  */
  do
  {
    c = getc(stream);
    nread++;
  } while (isspace(c));

  for (;;)
  {
    if (str_size >= alloc_size)
    {
      size_t old_alloc_size = alloc_size;
      alloc_size = alloc_size * 3 / 2;
      str = __GMP_REALLOCATE_FUNC_TYPE(str, old_alloc_size, alloc_size, char);
    }
    if (c == EOF || isspace(c))
      break;
    str[str_size++] = c;
    c = getc(stream);
  }
  ungetc(c, stream);
  nread--;

  if (str_size >= alloc_size)
  {
    size_t old_alloc_size = alloc_size;
    alloc_size = alloc_size * 3 / 2;
    str = __GMP_REALLOCATE_FUNC_TYPE(str, old_alloc_size, alloc_size, char);
  }
  str[str_size] = 0;

  res = mpf_set_str(rop, str, base);
  (*gmp_free_func)(str, alloc_size);

  if (res == -1)
    return 0; /* error */

  return str_size + nread;
}

void mpf_sub(mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr rp, tp;
  mp_size_t usize, vsize, rsize;
  mp_size_t prec;
  mp_exp_t exp;
  mp_size_t ediff;
  int negate;
  TMP_DECL;

  usize = SIZ(u);
  vsize = SIZ(v);

  /* Handle special cases that don't work in generic code below.  */
  if (usize == 0)
  {
    mpf_neg(r, v);
    return;
  }
  if (vsize == 0)
  {
    if (r != u)
      mpf_set(r, u);
    return;
  }

  /* If signs of U and V are different, perform addition.  */
  if ((usize ^ vsize) < 0)
  {
    __mpf_struct v_negated;
    v_negated._mp_size = -vsize;
    v_negated._mp_exp = EXP(v);
    v_negated._mp_d = PTR(v);
    mpf_add(r, u, &v_negated);
    return;
  }

  TMP_MARK;

  /* Signs are now known to be the same.  */
  negate = usize < 0;

  /* Make U be the operand with the largest exponent.  */
  if (EXP(u) < EXP(v))
  {
    mpf_srcptr t;
    t = u;
    u = v;
    v = t;
    negate ^= 1;
    usize = SIZ(u);
    vsize = SIZ(v);
  }

  usize = ABS(usize);
  vsize = ABS(vsize);
  up = PTR(u);
  vp = PTR(v);
  rp = PTR(r);
  prec = PREC(r) + 1;
  exp = EXP(u);
  ediff = exp - EXP(v);

  /* If ediff is 0 or 1, we might have a situation where the operands are
     extremely close.  We need to scan the operands from the most significant
     end ignore the initial parts that are equal.  */
  if (ediff <= 1)
  {
    if (ediff == 0)
    {
      /* Skip leading limbs in U and V that are equal.  */
      /* This loop normally exits immediately.  Optimize for that.  */
      while (up[usize - 1] == vp[vsize - 1])
      {
        usize--;
        vsize--;
        exp--;

        if (usize == 0)
        {
          /* u cancels high limbs of v, result is rest of v */
          negate ^= 1;
        cancellation:
          /* strip high zeros before truncating to prec */
          while (vsize != 0 && vp[vsize - 1] == 0)
          {
            vsize--;
            exp--;
          }
          if (vsize > prec)
          {
            vp += vsize - prec;
            vsize = prec;
          }
          MPN_COPY_INCR(rp, vp, vsize);
          rsize = vsize;
          goto done;
        }
        if (vsize == 0)
        {
          vp = up;
          vsize = usize;
          goto cancellation;
        }
      }

      if (up[usize - 1] < vp[vsize - 1])
      {
        /* For simplicity, swap U and V.  Note that since the loop above
     wouldn't have exited unless up[usize - 1] and vp[vsize - 1]
     were non-equal, this if-statement catches all cases where U
     is smaller than V.  */
        MPN_SRCPTR_SWAP(up, usize, vp, vsize);
        negate ^= 1;
        /* negating ediff not necessary since it is 0.  */
      }

      /* Check for
         x+1 00000000 ...
          x  ffffffff ... */
      if (up[usize - 1] != vp[vsize - 1] + 1)
        goto general_case;
      usize--;
      vsize--;
      exp--;
    }
    else /* ediff == 1 */
    {
      /* Check for
         1 00000000 ...
         0 ffffffff ... */

      if (up[usize - 1] != 1 || vp[vsize - 1] != GMP_NUMB_MAX || (usize >= 2 && up[usize - 2] != 0))
        goto general_case;

      usize--;
      exp--;
    }

    /* Skip sequences of 00000000/ffffffff */
    while (vsize != 0 && usize != 0 && up[usize - 1] == 0 && vp[vsize - 1] == GMP_NUMB_MAX)
    {
      usize--;
      vsize--;
      exp--;
    }

    if (usize == 0)
    {
      while (vsize != 0 && vp[vsize - 1] == GMP_NUMB_MAX)
      {
        vsize--;
        exp--;
      }
    }
    else if (usize > prec - 1)
    {
      up += usize - (prec - 1);
      usize = prec - 1;
    }
    if (vsize > prec - 1)
    {
      vp += vsize - (prec - 1);
      vsize = prec - 1;
    }

    tp = TMP_ALLOC_LIMBS(prec);
    {
      mp_limb_t cy_limb;
      if (vsize == 0)
      {
        MPN_COPY(tp, up, usize);
        tp[usize] = 1;
        rsize = usize + 1;
        exp++;
        goto normalized;
      }
      if (usize == 0)
      {
        cy_limb = mpn_neg(tp, vp, vsize);
        rsize = vsize;
      }
      else if (usize >= vsize)
      {
        /* uuuu     */
        /* vv       */
        mp_size_t size;
        size = usize - vsize;
        MPN_COPY(tp, up, size);
        cy_limb = mpn_sub_n(tp + size, up + size, vp, vsize);
        rsize = usize;
      }
      else /* (usize < vsize) */
      {
        /* uuuu     */
        /* vvvvvvv  */
        mp_size_t size;
        size = vsize - usize;
        cy_limb = mpn_neg(tp, vp, size);
        cy_limb = mpn_sub_nc(tp + size, up, vp + size, usize, cy_limb);
        rsize = vsize;
      }
      if (cy_limb == 0)
      {
        tp[rsize] = 1;
        rsize++;
        exp++;
        goto normalized;
      }
      goto normalize;
    }
  }

general_case:
  /* If U extends beyond PREC, ignore the part that does.  */
  if (usize > prec)
  {
    up += usize - prec;
    usize = prec;
  }

  /* If V extends beyond PREC, ignore the part that does.
     Note that this may make vsize negative.  */
  if (vsize + ediff > prec)
  {
    vp += vsize + ediff - prec;
    vsize = prec - ediff;
  }

  if (ediff >= prec)
  {
    /* V completely cancelled.  */
    if (rp != up)
      MPN_COPY(rp, up, usize);
    rsize = usize;
  }
  else
  {
    /* Allocate temp space for the result.  Allocate
 just vsize + ediff later???  */
    tp = TMP_ALLOC_LIMBS(prec);

    /* Locate the least significant non-zero limb in (the needed
 parts of) U and V, to simplify the code below.  */
    for (;;)
    {
      if (vsize == 0)
      {
        MPN_COPY(rp, up, usize);
        rsize = usize;
        goto done;
      }
      if (vp[0] != 0)
        break;
      vp++, vsize--;
    }
    for (;;)
    {
      if (usize == 0)
      {
        MPN_COPY(rp, vp, vsize);
        rsize = vsize;
        negate ^= 1;
        goto done;
      }
      if (up[0] != 0)
        break;
      up++, usize--;
    }

    /* uuuu     |  uuuu     |  uuuu     |  uuuu     |  uuuu    */
    /* vvvvvvv  |  vv       |    vvvvv  |    v      |       vv */

    if (usize > ediff)
    {
      /* U and V partially overlaps.  */
      if (ediff == 0)
      {
        /* Have to compare the leading limbs of u and v
     to determine whether to compute u - v or v - u.  */
        if (usize >= vsize)
        {
          /* uuuu     */
          /* vv       */
          mp_size_t size;
          size = usize - vsize;
          MPN_COPY(tp, up, size);
          mpn_sub_n(tp + size, up + size, vp, vsize);
          rsize = usize;
        }
        else /* (usize < vsize) */
        {
          /* uuuu     */
          /* vvvvvvv  */
          mp_size_t size;
          size = vsize - usize;
          ASSERT_CARRY(mpn_neg(tp, vp, size));
          mpn_sub_nc(tp + size, up, vp + size, usize, CNST_LIMB(1));
          rsize = vsize;
        }
      }
      else
      {
        if (vsize + ediff <= usize)
        {
          /* uuuu     */
          /*   v      */
          mp_size_t size;
          size = usize - ediff - vsize;
          MPN_COPY(tp, up, size);
          mpn_sub(tp + size, up + size, usize - size, vp, vsize);
          rsize = usize;
        }
        else
        {
          /* uuuu     */
          /*   vvvvv  */
          mp_size_t size;
          rsize = vsize + ediff;
          size = rsize - usize;
          ASSERT_CARRY(mpn_neg(tp, vp, size));
          mpn_sub(tp + size, up, usize, vp + size, usize - ediff);
          /* Should we use sub_nc then sub_1? */
          MPN_DECR_U(tp + size, usize, CNST_LIMB(1));
        }
      }
    }
    else
    {
      /* uuuu     */
      /*      vv  */
      mp_size_t size, i;
      size = vsize + ediff - usize;
      ASSERT_CARRY(mpn_neg(tp, vp, vsize));
      for (i = vsize; i < size; i++)
        tp[i] = GMP_NUMB_MAX;
      mpn_sub_1(tp + size, up, usize, (mp_limb_t)1);
      rsize = size + usize;
    }

  normalize:
    /* Full normalize.  Optimize later.  */
    while (rsize != 0 && tp[rsize - 1] == 0)
    {
      rsize--;
      exp--;
    }
  normalized:
    MPN_COPY(rp, tp, rsize);
  }

done:
  TMP_FREE;
  if (rsize == 0)
  {
    SIZ(r) = 0;
    EXP(r) = 0;
  }
  else
  {
    SIZ(r) = negate ? -rsize : rsize;
    EXP(r) = exp;
  }
}

int mpf_cmp_ui(mpf_srcptr u, unsigned long int vval) 
{
  mp_srcptr up;
  mp_size_t usize;
  mp_exp_t uexp;
  mp_limb_t ulimb;

  usize = SIZ(u);

  /* 1. Is U negative?  */
  if (usize < 0)
    return -1;
  /* We rely on usize being non-negative in the code that follows.  */

  if (vval == 0)
    return usize != 0;

  /* 2. Are the exponents different (V's exponent == 1)?  */
  uexp = EXP(u);

#if GMP_NAIL_BITS != 0
  if (uexp != 1 + (vval > GMP_NUMB_MAX))
    return (uexp < 1 + (vval > GMP_NUMB_MAX)) ? -1 : 1;
#else
  if (uexp != 1)
    return (uexp < 1) ? -1 : 1;
#endif

  up = PTR(u);

  ASSERT(usize > 0);
  ulimb = up[--usize];
#if GMP_NAIL_BITS != 0
  if (uexp == 2)
  {
    if ((ulimb >> GMP_NAIL_BITS) != 0)
      return 1;
    ulimb = (ulimb << GMP_NUMB_BITS);
    if (usize != 0)
      ulimb |= up[--usize];
  }
#endif

  /* 3. Compare the most significant mantissa limb with V.  */
  if (ulimb != vval)
    return (ulimb < vval) ? -1 : 1;

  /* Ignore zeroes at the low end of U.  */
  for (; *up == 0; ++up)
    --usize;

  /* 4. Now, if the number of limbs are different, we have a difference
     since we have made sure the trailing limbs are not zero.  */
  return (usize > 0);
}

size_t mpf_out_str(fd_t stream, int base, size_t n_digits, mpf_srcptr op)
{
  char *str;
  mp_exp_t exp;
  size_t written;
  TMP_DECL;

  TMP_MARK;

  if (base == 0)
    base = 10;
  if (n_digits == 0)
    MPF_SIGNIFICANT_DIGITS(n_digits, base, op->_mp_prec);

  if (stream == 0)
    stream = stdout;

  /* Consider these changes:
     * Don't allocate memory here for huge n_digits; pass NULL to mpf_get_str.
     * Make mpf_get_str allocate extra space when passed NULL, to avoid
       allocating two huge string buffers.
     * Implement more/other allocation reductions tricks.  */

  str = (char *)TMP_ALLOC(n_digits + 2); /* extra for minus sign and \0 */

  mpf_get_str(str, &exp, base, n_digits, op);
  n_digits = strlen(str);

  written = 0;

  /* Write sign */
  if (str[0] == '-')
  {
    str++;
    fputc('-', stream);
    written = 1;
    n_digits--;
  }

  {
    const char *point = GMP_DECIMAL_POINT;
    size_t pointlen = strlen(point);
    putc('0', stream);
    fwrite(point, 1, pointlen, stream);
    written += pointlen + 1;
  }

  /* Write mantissa */
  {
    size_t fwret;
    fwret = fwrite(str, 1, n_digits, stream);
    written += fwret;
  }

  /* Write exponent */
  {
    int fpret;
    fpret = fprintf(stream, (base <= 10 ? "e%ld" : "@%ld"), exp);
    written += fpret;
  }

  TMP_FREE;
  return ferror(stream) ? 0 : written;
}

void mpf_set(mpf_ptr r, mpf_srcptr u)
{
  mp_ptr rp, up;
  mp_size_t size, asize;
  mp_size_t prec;

  prec = r->_mp_prec + 1; /* lie not to lose precision in assignment */
  size = u->_mp_size;
  asize = ABS(size);
  rp = r->_mp_d;
  up = u->_mp_d;

  if (asize > prec)
  {
    up += asize - prec;
    asize = prec;
  }

  r->_mp_exp = u->_mp_exp;
  r->_mp_size = size >= 0 ? asize : -asize;
  MPN_COPY_INCR(rp, up, asize);
}

void mpf_clear(mpf_ptr x)
{
  __GMP_FREE_FUNC_LIMBS(PTR(x), PREC(x) + 1);
}

void mpf_div(mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr rp, tp, new_vp;
  mp_size_t usize, vsize, rsize, prospective_rsize, tsize, zeros;
  mp_size_t sign_quotient, prec, high_zero, chop;
  mp_exp_t rexp;
  int copy_u;
  TMP_DECL;

  usize = SIZ(u);
  vsize = SIZ(v);

  if (UNLIKELY(vsize == 0))
    DIVIDE_BY_ZERO;

  if (usize == 0)
  {
    SIZ(r) = 0;
    EXP(r) = 0;
    return;
  }

  sign_quotient = usize ^ vsize;
  usize = ABS(usize);
  vsize = ABS(vsize);
  prec = PREC(r);

  TMP_MARK;
  rexp = EXP(u) - EXP(v) + 1;

  rp = PTR(r);
  up = PTR(u);
  vp = PTR(v);

  prospective_rsize = usize - vsize + 1; /* quot from using given u,v sizes */
  rsize = prec + 1;                      /* desired quot */

  zeros = rsize - prospective_rsize; /* padding u to give rsize */
  copy_u = (zeros > 0 || rp == up);  /* copy u if overlap or padding */

  chop = MAX(-zeros, 0); /* negative zeros means shorten u */
  up += chop;
  usize -= chop;
  zeros += chop; /* now zeros >= 0 */

  tsize = usize + zeros; /* size for possible copy of u */

  /* copy and possibly extend u if necessary */
  if (copy_u)
  {
    tp = TMP_ALLOC_LIMBS(tsize + 1); /* +1 for mpn_div_q's scratch needs */
    MPN_ZERO(tp, zeros);
    MPN_COPY(tp + zeros, up, usize);
    up = tp;
    usize = tsize;
  }
  else
  {
    tp = TMP_ALLOC_LIMBS(usize + 1);
  }

  /* ensure divisor doesn't overlap quotient */
  if (rp == vp)
  {
    new_vp = TMP_ALLOC_LIMBS(vsize);
    MPN_COPY(new_vp, vp, vsize);
    vp = new_vp;
  }

  ASSERT(usize - vsize + 1 == rsize);
  mpn_div_q(rp, up, usize, vp, vsize, tp);

  /* strip possible zero high limb */
  high_zero = (rp[rsize - 1] == 0);
  rsize -= high_zero;
  rexp -= high_zero;

  SIZ(r) = sign_quotient >= 0 ? rsize : -rsize;
  EXP(r) = rexp;
  TMP_FREE;
}

int mpf_init_set_str(mpf_ptr r, const char *s, int base)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_size = 0;
  r->_mp_exp = 0;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);

  return mpf_set_str(r, s, base);
}

void mpf_set_ui(mpf_ptr f, unsigned long val)
{
  mp_size_t size;

  f->_mp_d[0] = val & GMP_NUMB_MASK;
  size = val != 0;

#if BITS_PER_ULONG > GMP_NUMB_BITS
  val >>= GMP_NUMB_BITS;
  f->_mp_d[1] = val;
  size += (val != 0);
#endif

  f->_mp_exp = f->_mp_size = size;
}

double mpf_get_d(mpf_srcptr src)
{
  mp_size_t size, abs_size;
  long exp;

  size = SIZ(src);
  if (UNLIKELY(size == 0))
    return 0.0;

  abs_size = ABS(size);
  exp = (EXP(src) - abs_size) * GMP_NUMB_BITS;
  return mpn_get_d(PTR(src), abs_size, size, exp);
}

int mpf_cmp_si(mpf_srcptr u, long int vval) 
{
  mp_srcptr up;
  mp_size_t usize;
  mp_exp_t uexp;
  mp_limb_t ulimb;
  int usign;
  unsigned long abs_vval;

  usize = SIZ(u);

  /* 1. Are the signs different?  */
  if ((usize < 0) == (vval < 0)) /* don't use xor, type size may differ */
  {
    /* U and V are both non-negative or both negative.  */
    if (usize == 0)
      /* vval >= 0 */
      return -(vval != 0);
    if (vval == 0)
      /* usize >= 0 */
      return usize != 0;
    /* Fall out.  */
  }
  else
  {
    /* Either U or V is negative, but not both.  */
    return usize >= 0 ? 1 : -1;
  }

  /* U and V have the same sign and are both non-zero.  */

  /* 2. Are the exponents different (V's exponent == 1)?  */
  uexp = EXP(u);
  usign = usize >= 0 ? 1 : -1;
  usize = ABS(usize);
  abs_vval = ABS_CAST(unsigned long, vval);

#if GMP_NAIL_BITS != 0
  if (uexp != 1 + (abs_vval > GMP_NUMB_MAX))
    return (uexp < 1 + (abs_vval > GMP_NUMB_MAX)) ? -usign : usign;
#else
  if (uexp != 1)
    return (uexp < 1) ? -usign : usign;
#endif

  up = PTR(u);

  ASSERT(usize > 0);
  ulimb = up[--usize];
#if GMP_NAIL_BITS != 0
  if (uexp == 2)
  {
    if ((ulimb >> GMP_NAIL_BITS) != 0)
      return usign;
    ulimb = (ulimb << GMP_NUMB_BITS);
    if (usize != 0)
      ulimb |= up[--usize];
  }
#endif

  /* 3. Compare the most significant mantissa limb with V.  */
  if (ulimb != abs_vval)
    return (ulimb < abs_vval) ? -usign : usign;

  /* Ignore zeroes at the low end of U.  */
  for (; *up == 0; ++up)
    --usize;

  /* 4. Now, if the number of limbs are different, we have a difference
     since we have made sure the trailing limbs are not zero.  */
  if (usize > 0)
    return usign;

  /* Wow, we got zero even if we tried hard to avoid it.  */
  return 0;
}

void mpf_set_d(mpf_ptr r, double d)
{
  int negative;

  DOUBLE_NAN_INF_ACTION(d,
                        __gmp_invalid_operation(),
                        __gmp_invalid_operation());

  if (UNLIKELY(d == 0))
  {
    SIZ(r) = 0;
    EXP(r) = 0;
    return;
  }
  negative = d < 0;
  d = ABS(d);

  SIZ(r) = negative ? -LIMBS_PER_DOUBLE : LIMBS_PER_DOUBLE;
  EXP(r) = __gmp_extract_double(PTR(r), d);
}

void mpf_div_2exp(mpf_ptr r, mpf_srcptr u, mp_bitcnt_t exp)
{
  mp_srcptr up;
  mp_ptr rp = r->_mp_d;
  mp_size_t usize;
  mp_size_t abs_usize;
  mp_size_t prec = r->_mp_prec;
  mp_exp_t uexp = u->_mp_exp;

  usize = u->_mp_size;

  if (UNLIKELY(usize == 0))
  {
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

  abs_usize = ABS(usize);
  up = u->_mp_d;

  if (exp % GMP_NUMB_BITS == 0)
  {
    prec++; /* retain more precision here as we don't need
     to account for carry-out here */
    if (abs_usize > prec)
    {
      up += abs_usize - prec;
      abs_usize = prec;
    }
    if (rp != up)
      MPN_COPY_INCR(rp, up, abs_usize);
    r->_mp_exp = uexp - exp / GMP_NUMB_BITS;
  }
  else
  {
    mp_limb_t cy_limb;
    mp_size_t adj;
    if (abs_usize > prec)
    {
      up += abs_usize - prec;
      abs_usize = prec;
      /* Use mpn_rshift since mpn_lshift operates downwards, and we
         therefore would clobber part of U before using that part, in case
         R is the same variable as U.  */
      cy_limb = mpn_rshift(rp + 1, up, abs_usize, exp % GMP_NUMB_BITS);
      rp[0] = cy_limb;
      adj = rp[abs_usize] != 0;
    }
    else
    {
      cy_limb = mpn_lshift(rp, up, abs_usize,
                           GMP_NUMB_BITS - exp % GMP_NUMB_BITS);
      rp[abs_usize] = cy_limb;
      adj = cy_limb != 0;
    }

    abs_usize += adj;
    r->_mp_exp = uexp - exp / GMP_NUMB_BITS - 1 + adj;
  }
  r->_mp_size = usize >= 0 ? abs_usize : -abs_usize;
}

void mpf_abs(mpf_ptr r, mpf_srcptr u)
{
  mp_size_t size;

  size = ABS(u->_mp_size);
  if (r != u)
  {
    mp_size_t prec;
    mp_ptr rp, up;

    prec = r->_mp_prec + 1; /* lie not to lose precision in assignment */
    rp = r->_mp_d;
    up = u->_mp_d;

    if (size > prec)
    {
      up += size - prec;
      size = prec;
    }

    MPN_COPY(rp, up, size);
    r->_mp_exp = u->_mp_exp;
  }
  r->_mp_size = size;
}

void mpf_add(mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
{
  mp_srcptr up, vp;
  mp_ptr rp, tp;
  mp_size_t usize, vsize, rsize;
  mp_size_t prec;
  mp_exp_t uexp;
  mp_size_t ediff;
  mp_limb_t cy;
  int negate;
  TMP_DECL;

  usize = u->_mp_size;
  vsize = v->_mp_size;

  /* Handle special cases that don't work in generic code below.  */
  if (usize == 0)
  {
  set_r_v_maybe:
    if (r != v)
      mpf_set(r, v);
    return;
  }
  if (vsize == 0)
  {
    v = u;
    goto set_r_v_maybe;
  }

  /* If signs of U and V are different, perform subtraction.  */
  if ((usize ^ vsize) < 0)
  {
    __mpf_struct v_negated;
    v_negated._mp_size = -vsize;
    v_negated._mp_exp = v->_mp_exp;
    v_negated._mp_d = v->_mp_d;
    mpf_sub(r, u, &v_negated);
    return;
  }

  TMP_MARK;

  /* Signs are now known to be the same.  */
  negate = usize < 0;

  /* Make U be the operand with the largest exponent.  */
  if (u->_mp_exp < v->_mp_exp)
  {
    mpf_srcptr t;
    t = u;
    u = v;
    v = t;
    usize = u->_mp_size;
    vsize = v->_mp_size;
  }

  usize = GMP_ABS(usize);
  vsize = GMP_ABS(vsize);
  up = u->_mp_d;
  vp = v->_mp_d;
  rp = r->_mp_d;
  prec = r->_mp_prec;
  uexp = u->_mp_exp;
  ediff = u->_mp_exp - v->_mp_exp;

  /* If U extends beyond PREC, ignore the part that does.  */
  if (usize > prec)
  {
    up += usize - prec;
    usize = prec;
  }

  /* If V extends beyond PREC, ignore the part that does.
     Note that this may make vsize negative.  */
  if (vsize + ediff > prec)
  {
    vp += vsize + ediff - prec;
    vsize = prec - ediff;
  }

#if 0
  /* Locate the least significant non-zero limb in (the needed parts
     of) U and V, to simplify the code below.  */
  while (up[0] == 0)
    up++, usize--;
  while (vp[0] == 0)
    vp++, vsize--;
#endif

  /* Allocate temp space for the result.  Allocate
     just vsize + ediff later???  */
  tp = TMP_ALLOC_LIMBS(prec);

  if (ediff >= prec)
  {
    /* V completely cancelled.  */
    if (rp != up)
      MPN_COPY_INCR(rp, up, usize);
    rsize = usize;
  }
  else
  {
    /* uuuu     |  uuuu     |  uuuu     |  uuuu     |  uuuu    */
    /* vvvvvvv  |  vv       |    vvvvv  |    v      |       vv */

    if (usize > ediff)
    {
      /* U and V partially overlaps.  */
      if (vsize + ediff <= usize)
      {
        /* uuuu     */
        /*   v      */
        mp_size_t size;
        size = usize - ediff - vsize;
        MPN_COPY(tp, up, size);
        cy = mpn_add(tp + size, up + size, usize - size, vp, vsize);
        rsize = usize;
      }
      else
      {
        /* uuuu     */
        /*   vvvvv  */
        mp_size_t size;
        size = vsize + ediff - usize;
        MPN_COPY(tp, vp, size);
        cy = mpn_add(tp + size, up, usize, vp + size, usize - ediff);
        rsize = vsize + ediff;
      }
    }
    else
    {
      /* uuuu     */
      /*      vv  */
      mp_size_t size;
      size = vsize + ediff - usize;
      MPN_COPY(tp, vp, vsize);
      MPN_ZERO(tp + vsize, ediff - usize);
      MPN_COPY(tp + size, up, usize);
      cy = 0;
      rsize = size + usize;
    }

    MPN_COPY(rp, tp, rsize);
    rp[rsize] = cy;
    rsize += cy;
    uexp += cy;
  }

  r->_mp_size = negate ? -rsize : rsize;
  r->_mp_exp = uexp;
  TMP_FREE;
}

void mpf_pow_ui(mpf_ptr r, mpf_srcptr b, unsigned long int e)
{
  __mpf_struct t;
  int cnt;

  if (e <= 1)
  {
    if (e == 0)
      mpf_set_ui(r, 1);
    else
      mpf_set(r, b);
    return;
  }

  count_leading_zeros(cnt, (mp_limb_t)e);
  cnt = GMP_LIMB_BITS - 1 - cnt;

  /* Increase computation precision as a function of the exponent.  Adding
     log2(popcount(e) + log2(e)) bits should be sufficient, but we add log2(e),
     i.e. much more.  With mpf's rounding of precision to whole limbs, this
     will be excessive only when limbs are artificially small.  */
  mpf_init2(&t, mpf_get_prec(r) + cnt);

  mpf_set(&t, b); /* consume most significant bit */
  while (--cnt > 0)
  {
    mpf_mul(&t, &t, &t);
    if ((e >> cnt) & 1)
      mpf_mul(&t, &t, b);
  }

  /* Do the last iteration specially in order to save a copy operation.  */
  if (e & 1)
  {
    mpf_mul(&t, &t, &t);
    mpf_mul(r, &t, b);
  }
  else
  {
    mpf_mul(r, &t, &t);
  }

  mpf_clear(&t);
}

void mpf_ui_sub(mpf_ptr r, unsigned long int u, mpf_srcptr v)
{
#if 1
  __mpf_struct uu;
  mp_limb_t ul;

  if (u == 0)
  {
    mpf_neg(r, v);
    return;
  }

  ul = u;
  uu._mp_size = 1;
  uu._mp_d = &ul;
  uu._mp_exp = 1;
  mpf_sub(r, &uu, v);

#else
  mp_srcptr up, vp;
  mp_ptr rp, tp;
  mp_size_t usize, vsize, rsize;
  mp_size_t prec;
  mp_exp_t uexp;
  mp_size_t ediff;
  int negate;
  mp_limb_t ulimb;
  TMP_DECL;

  vsize = v->_mp_size;

  /* Handle special cases that don't work in generic code below.  */
  if (u == 0)
  {
    mpf_neg(r, v);
    return;
  }
  if (vsize == 0)
  {
    mpf_set_ui(r, u);
    return;
  }

  /* If signs of U and V are different, perform addition.  */
  if (vsize < 0)
  {
    __mpf_struct v_negated;
    v_negated._mp_size = -vsize;
    v_negated._mp_exp = v->_mp_exp;
    v_negated._mp_d = v->_mp_d;
    mpf_add_ui(r, &v_negated, u);
    return;
  }

  /* Signs are now known to be the same.  */
  ASSERT(vsize > 0);
  ulimb = u;
  /* Make U be the operand with the largest exponent.  */
  negate = 1 < v->_mp_exp;
  prec = r->_mp_prec + negate;
  rp = r->_mp_d;
  if (negate)
  {
    usize = vsize;
    vsize = 1;
    up = v->_mp_d;
    vp = &ulimb;
    uexp = v->_mp_exp;
    ediff = uexp - 1;

    /* If U extends beyond PREC, ignore the part that does.  */
    if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }
    ASSERT(ediff > 0);
  }
  else
  {
    vp = v->_mp_d;
    ediff = 1 - v->_mp_exp;
    /* Ignore leading limbs in U and V that are equal.  Doing
       this helps increase the precision of the result.  */
    if (ediff == 0 && ulimb == vp[vsize - 1])
    {
      usize = 0;
      vsize--;
      uexp = 0;
      /* Note that V might now have leading zero limbs.
         In that case we have to adjust uexp.  */
      for (;;)
      {
        if (vsize == 0)
        {
          rsize = 0;
          uexp = 0;
          goto done;
        }
        if (vp[vsize - 1] != 0)
          break;
        vsize--, uexp--;
      }
    }
    else
    {
      usize = 1;
      uexp = 1;
      up = &ulimb;
    }
    ASSERT(usize <= prec);
  }

  if (ediff >= prec)
  {
    /* V completely cancelled.  */
    if (rp != up)
      MPN_COPY(rp, up, usize);
    rsize = usize;
  }
  else
  {
    /* If V extends beyond PREC, ignore the part that does.
       Note that this can make vsize neither zero nor negative.  */
    if (vsize + ediff > prec)
    {
      vp += vsize + ediff - prec;
      vsize = prec - ediff;
    }

    /* Locate the least significant non-zero limb in (the needed
 parts of) U and V, to simplify the code below.  */
    ASSERT(vsize > 0);
    for (;;)
    {
      if (vp[0] != 0)
        break;
      vp++, vsize--;
      if (vsize == 0)
      {
        MPN_COPY(rp, up, usize);
        rsize = usize;
        goto done;
      }
    }
    for (;;)
    {
      if (usize == 0)
      {
        MPN_COPY(rp, vp, vsize);
        rsize = vsize;
        negate ^= 1;
        goto done;
      }
      if (up[0] != 0)
        break;
      up++, usize--;
    }

    ASSERT(usize > 0 && vsize > 0);
    TMP_MARK;

    tp = TMP_ALLOC_LIMBS(prec);

    /* uuuu     |  uuuu     |  uuuu     |  uuuu     |  uuuu    */
    /* vvvvvvv  |  vv       |    vvvvv  |    v      |       vv */

    if (usize > ediff)
    {
      /* U and V partially overlaps.  */
      if (ediff == 0)
      {
        ASSERT(usize == 1 && vsize >= 1 && ulimb == *up); /* usize is 1>ediff, vsize >= 1 */
        if (1 < vsize)
        {
          /* u        */
          /* vvvvvvv  */
          rsize = vsize;
          vsize -= 1;
          /* mpn_cmp (up, vp + vsize - usize, usize) > 0 */
          if (ulimb > vp[vsize])
          {
            tp[vsize] = ulimb - vp[vsize] - 1;
            ASSERT_CARRY(mpn_neg(tp, vp, vsize));
          }
          else
          {
            /* vvvvvvv  */ /* Swap U and V. */
            /* u        */
            MPN_COPY(tp, vp, vsize);
            tp[vsize] = vp[vsize] - ulimb;
            negate = 1;
          }
        }
        else /* vsize == usize == 1 */
        {
          /* u     */
          /* v     */
          rsize = 1;
          negate = ulimb < vp[0];
          tp[0] = negate ? vp[0] - ulimb : ulimb - vp[0];
        }
      }
      else
      {
        ASSERT(vsize + ediff <= usize);
        ASSERT(vsize == 1 && usize >= 2 && ulimb == *vp);
        {
          /* uuuu     */
          /*   v      */
          mp_size_t size;
          size = usize - ediff - 1;
          MPN_COPY(tp, up, size);
          ASSERT_NOCARRY(mpn_sub_1(tp + size, up + size, usize - size, ulimb));
          rsize = usize;
        }
        /* Other cases are not possible */
        /* uuuu     */
        /*   vvvvv  */
      }
    }
    else
    {
      /* uuuu     */
      /*      vv  */
      mp_size_t size, i;
      ASSERT_CARRY(mpn_neg(tp, vp, vsize));
      rsize = vsize + ediff;
      size = rsize - usize;
      for (i = vsize; i < size; i++)
        tp[i] = GMP_NUMB_MAX;
      ASSERT_NOCARRY(mpn_sub_1(tp + size, up, usize, CNST_LIMB(1)));
    }

    /* Full normalize.  Optimize later.  */
    while (rsize != 0 && tp[rsize - 1] == 0)
    {
      rsize--;
      uexp--;
    }
    MPN_COPY(rp, tp, rsize);
    TMP_FREE;
  }

done:
  r->_mp_size = negate ? -rsize : rsize;
  r->_mp_exp = uexp;
#endif
}

int mpf_cmp(mpf_srcptr u, mpf_srcptr v) 
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_exp_t uexp, vexp;
  int cmp;
  int usign;

  usize = SIZ(u);
  vsize = SIZ(v);
  usign = usize >= 0 ? 1 : -1;

  /* 1. Are the signs different?  */
  if ((usize ^ vsize) >= 0)
  {
    /* U and V are both non-negative or both negative.  */
    if (usize == 0)
      /* vsize >= 0 */
      return -(vsize != 0);
    if (vsize == 0)
      /* usize >= 0 */
      return usize != 0;
    /* Fall out.  */
  }
  else
  {
    /* Either U or V is negative, but not both.  */
    return usign;
  }

  /* U and V have the same sign and are both non-zero.  */

  uexp = EXP(u);
  vexp = EXP(v);

  /* 2. Are the exponents different?  */
  if (uexp > vexp)
    return usign;
  if (uexp < vexp)
    return -usign;

  usize = ABS(usize);
  vsize = ABS(vsize);

  up = PTR(u);
  vp = PTR(v);

#define STRICT_MPF_NORMALIZATION 0
#if !STRICT_MPF_NORMALIZATION
  /* Ignore zeroes at the low end of U and V.  */
  do
  {
    mp_limb_t tl;
    tl = up[0];
    MPN_STRIP_LOW_ZEROS_NOT_ZERO(up, usize, tl);
    tl = vp[0];
    MPN_STRIP_LOW_ZEROS_NOT_ZERO(vp, vsize, tl);
  } while (0);
#endif

  if (usize > vsize)
  {
    cmp = mpn_cmp(up + usize - vsize, vp, vsize);
    /* if (cmp == 0) */
    /*	return usign; */
    ++cmp;
  }
  else if (vsize > usize)
  {
    cmp = mpn_cmp(up, vp + vsize - usize, usize);
    /* if (cmp == 0) */
    /*	return -usign; */
  }
  else
  {
    cmp = mpn_cmp(up, vp, usize);
    if (cmp == 0)
      return 0;
  }
  return cmp > 0 ? usign : -usign;
}

void mpf_sqrt(mpf_ptr r, mpf_srcptr u)
{
  mp_size_t usize;
  mp_ptr up, tp;
  mp_size_t prec, tsize;
  mp_exp_t uexp, expodd;
  TMP_DECL;

  usize = u->_mp_size;
  if (UNLIKELY(usize <= 0))
  {
    if (usize < 0)
      SQRT_OF_NEGATIVE;
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

  TMP_MARK;

  uexp = u->_mp_exp;
  prec = r->_mp_prec;
  up = u->_mp_d;

  expodd = (uexp & 1);
  tsize = 2 * prec - expodd;
  r->_mp_size = prec;
  r->_mp_exp = (uexp + expodd) / 2; /* ceil(uexp/2) */

  /* root size is ceil(tsize/2), this will be our desired "prec" limbs */
  ASSERT((tsize + 1) / 2 == prec);

  tp = TMP_ALLOC_LIMBS(tsize);

  if (usize > tsize)
  {
    up += usize - tsize;
    usize = tsize;
    MPN_COPY(tp, up, tsize);
  }
  else
  {
    MPN_ZERO(tp, tsize - usize);
    MPN_COPY(tp + (tsize - usize), up, usize);
  }

  mpn_sqrtrem(r->_mp_d, NULL, tp, tsize);

  TMP_FREE;
}

void mpf_div_ui(mpf_ptr r, mpf_srcptr u, unsigned long int v)
{
  mp_srcptr up;
  mp_ptr rp, tp, rtp;
  mp_size_t usize;
  mp_size_t rsize, tsize;
  mp_size_t sign_quotient;
  mp_size_t prec;
  mp_limb_t q_limb;
  mp_exp_t rexp;
  TMP_DECL;

#if BITS_PER_ULONG > GMP_NUMB_BITS /* avoid warnings about shift amount */
  if (v > GMP_NUMB_MAX)
  {
    mpf_t vf;
    mp_limb_t vl[2];
    SIZ(vf) = 2;
    EXP(vf) = 2;
    PTR(vf) = vl;
    vl[0] = v & GMP_NUMB_MASK;
    vl[1] = v >> GMP_NUMB_BITS;
    mpf_div(r, u, vf);
    return;
  }
#endif

  if (UNLIKELY(v == 0))
    DIVIDE_BY_ZERO;

  usize = u->_mp_size;

  if (usize == 0)
  {
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

  sign_quotient = usize;
  usize = ABS(usize);
  prec = r->_mp_prec;

  TMP_MARK;

  rp = r->_mp_d;
  up = u->_mp_d;

  tsize = 1 + prec;
  tp = TMP_ALLOC_LIMBS(tsize + 1);

  if (usize > tsize)
  {
    up += usize - tsize;
    usize = tsize;
    rtp = tp;
  }
  else
  {
    MPN_ZERO(tp, tsize - usize);
    rtp = tp + (tsize - usize);
  }

  /* Move the dividend to the remainder.  */
  MPN_COPY(rtp, up, usize);

  mpn_divmod_1(rp, tp, tsize, (mp_limb_t)v);
  q_limb = rp[tsize - 1];

  rsize = tsize - (q_limb == 0);
  rexp = u->_mp_exp - (q_limb == 0);
  r->_mp_size = sign_quotient >= 0 ? rsize : -rsize;
  r->_mp_exp = rexp;
  TMP_FREE;
}

double mpf_get_d_2exp(signed long int *expptr, mpf_srcptr src)
{
  mp_size_t size, abs_size;
  mp_srcptr ptr;
  int cnt;

  size = SIZ(src);
  if (UNLIKELY(size == 0))
  {
    *expptr = 0;
    return 0.0;
  }

  ptr = PTR(src);
  abs_size = ABS(size);
  count_leading_zeros(cnt, ptr[abs_size - 1]);
  cnt -= GMP_NAIL_BITS;

  *expptr = EXP(src) * GMP_NUMB_BITS - cnt;
  return mpn_get_d(ptr, abs_size, size, -(abs_size * GMP_NUMB_BITS - cnt));
}

void mpf_sub_ui(mpf_ptr sum, mpf_srcptr u, unsigned long int v)
{
  __mpf_struct vv;
  mp_limb_t vl;

  if (v == 0)
  {
    mpf_set(sum, u);
    return;
  }

  vl = v;
  vv._mp_size = 1;
  vv._mp_d = &vl;
  vv._mp_exp = 1;
  mpf_sub(sum, u, &vv);
}

void mpf_neg(mpf_ptr r, mpf_srcptr u)
{
  mp_size_t size;

  size = -u->_mp_size;
  if (r != u)
  {
    mp_size_t prec;
    mp_size_t asize;
    mp_ptr rp, up;

    prec = r->_mp_prec + 1; /* lie not to lose precision in assignment */
    asize = ABS(size);
    rp = r->_mp_d;
    up = u->_mp_d;

    if (asize > prec)
    {
      up += asize - prec;
      asize = prec;
    }

    MPN_COPY(rp, up, asize);
    r->_mp_exp = u->_mp_exp;
    size = size >= 0 ? asize : -asize;
  }
  r->_mp_size = size;
}

unsigned long mpf_get_ui(mpf_srcptr f) 
{
  mp_size_t size;
  mp_exp_t exp;
  mp_srcptr fp;
  mp_limb_t fl;

  exp = EXP(f);
  size = SIZ(f);
  fp = PTR(f);

  fl = 0;
  if (exp > 0)
  {
    /* there are some limbs above the radix point */

    size = ABS(size);
    if (size >= exp)
      fl = fp[size - exp];

#if BITS_PER_ULONG > GMP_NUMB_BITS
    if (exp > 1 && size + 1 >= exp)
      fl += (fp[size - exp + 1] << GMP_NUMB_BITS);
#endif
  }

  return (unsigned long)fl;
}

void mpf_init_set_d(mpf_ptr r, double val)
{
  mp_size_t prec = __gmp_default_fp_limb_precision;
  r->_mp_prec = prec;
  r->_mp_d = __GMP_ALLOCATE_FUNC_LIMBS(prec + 1);

  mpf_set_d(r, val);
}

int mpf_cmp_z(mpf_srcptr u, mpz_srcptr v) 
{
  __mpf_struct vf;
  mp_size_t size = 0;

  (&vf)->_mp_size = size = v->_mp_size;
  (&vf)->_mp_exp = size = ABS(size);
  /* PREC (vf) = size; */
  (&vf)->_mp_d = PTR(v);

  return mpf_cmp(u, &vf);
}

void mpf_ui_div(mpf_ptr r, unsigned long int u, mpf_srcptr v)
{
  mp_srcptr vp;
  mp_ptr rp, tp, remp, new_vp;
  mp_size_t vsize;
  mp_size_t rsize, prospective_rsize, zeros, tsize, high_zero;
  mp_size_t sign_quotient;
  mp_size_t prec;
  mp_exp_t rexp;
  TMP_DECL;

  vsize = v->_mp_size;
  sign_quotient = vsize;

  if (UNLIKELY(vsize == 0))
    DIVIDE_BY_ZERO;

  if (UNLIKELY(u == 0))
  {
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

  vsize = ABS(vsize);
  prec = r->_mp_prec;

  TMP_MARK;
  rexp = 1 - v->_mp_exp + 1;

  rp = r->_mp_d;
  vp = v->_mp_d;

  prospective_rsize = 1 - vsize + 1; /* quot from using given u,v sizes */
  rsize = prec + 1;                  /* desired quot size */

  zeros = rsize - prospective_rsize; /* padding u to give rsize */
  tsize = 1 + zeros;                 /* u with zeros */

  if (WANT_TMP_DEBUG)
  {
    /* separate alloc blocks, for malloc debugging */
    remp = TMP_ALLOC_LIMBS(vsize);
    tp = TMP_ALLOC_LIMBS(tsize);
    new_vp = NULL;
    if (rp == vp)
      new_vp = TMP_ALLOC_LIMBS(vsize);
  }
  else
  {
    /* one alloc with calculated size, for efficiency */
    mp_size_t size = vsize + tsize + (rp == vp ? vsize : 0);
    remp = TMP_ALLOC_LIMBS(size);
    tp = remp + vsize;
    new_vp = tp + tsize;
  }

  /* ensure divisor doesn't overlap quotient */
  if (rp == vp)
  {
    MPN_COPY(new_vp, vp, vsize);
    vp = new_vp;
  }

  MPN_ZERO(tp, tsize - 1);

  tp[tsize - 1] = u & GMP_NUMB_MASK;
#if BITS_PER_ULONG > GMP_NUMB_BITS
  if (u > GMP_NUMB_MAX)
  {
    /* tsize-vsize+1 == rsize, so tsize >= rsize.  rsize == prec+1 >= 2,
       so tsize >= 2, hence there's room for 2-limb u with nails */
    ASSERT(tsize >= 2);
    tp[tsize - 1] = u >> GMP_NUMB_BITS;
    tp[tsize - 2] = u & GMP_NUMB_MASK;
    rexp++;
  }
#endif

  ASSERT(tsize - vsize + 1 == rsize);
  mpn_tdiv_qr(rp, remp, (mp_size_t)0, tp, tsize, vp, vsize);

  /* strip possible zero high limb */
  high_zero = (rp[rsize - 1] == 0);
  rsize -= high_zero;
  rexp -= high_zero;

  r->_mp_size = sign_quotient >= 0 ? rsize : -rsize;
  r->_mp_exp = rexp;
  TMP_FREE;
}

void mpf_set_prec_raw(mpf_ptr x, mp_bitcnt_t prec_in_bits) 
{
  x->_mp_prec = __GMPF_BITS_TO_PREC(prec_in_bits);
}

void mpf_dump(mpf_srcptr u)
{
  mp_exp_t exp;
  char *str;

  str = mpf_get_str(0, &exp, 10, 0, u);
  if (str[0] == '-')
    printf("-0.%se%ld\n", str + 1, exp);
  else
    printf("0.%se%ld\n", str, exp);
  (*gmp_free_func)(str, strlen(str) + 1);
}

void mpf_sqrt_ui(mpf_ptr r, unsigned long int u)
{
  mp_size_t rsize, zeros;
  mp_ptr tp;
  mp_size_t prec;
  TMP_DECL;

  if (UNLIKELY(u <= 1))
  {
    SIZ(r) = EXP(r) = u;
    *PTR(r) = u;
    return;
  }

  TMP_MARK;

  prec = PREC(r);
  zeros = 2 * prec - 2;
  rsize = zeros + 1 + U2;

  tp = TMP_ALLOC_LIMBS(rsize);

  MPN_ZERO(tp, zeros);
  tp[zeros] = u & GMP_NUMB_MASK;

#if U2
  {
    mp_limb_t uhigh = u >> GMP_NUMB_BITS;
    tp[zeros + 1] = uhigh;
    rsize -= (uhigh == 0);
  }
#endif

  mpn_sqrtrem(PTR(r), NULL, tp, rsize);

  SIZ(r) = prec;
  EXP(r) = 1;
  TMP_FREE;
}

void mpf_set_q(mpf_t r, mpq_srcptr q)
{
  mp_srcptr np, dp;
  mp_size_t prec, nsize, dsize, qsize, prospective_qsize, tsize, zeros;
  mp_size_t sign_quotient, high_zero;
  mp_ptr qp, tp;
  mp_exp_t exp;
  TMP_DECL;

  ASSERT(SIZ(&q->_mp_den) > 0); /* canonical q */

  nsize = SIZ(&q->_mp_num);
  dsize = SIZ(&q->_mp_den);

  if (UNLIKELY(nsize == 0))
  {
    SIZ(r) = 0;
    EXP(r) = 0;
    return;
  }

  TMP_MARK;

  prec = PREC(r);
  qp = PTR(r);

  sign_quotient = nsize;
  nsize = ABS(nsize);
  np = PTR(&q->_mp_num);
  dp = PTR(&q->_mp_den);

  prospective_qsize = nsize - dsize + 1; /* q from using given n,d sizes */
  exp = prospective_qsize;               /* ie. number of integer limbs */
  qsize = prec + 1;                      /* desired q */

  zeros = qsize - prospective_qsize; /* n zeros to get desired qsize */
  tsize = nsize + zeros;             /* size of intermediate numerator */
  tp = TMP_ALLOC_LIMBS(tsize + 1);   /* +1 for mpn_div_q's scratch */

  if (zeros > 0)
  {
    /* pad n with zeros into temporary space */
    MPN_ZERO(tp, zeros);
    MPN_COPY(tp + zeros, np, nsize);
    np = tp; /* mpn_div_q allows this overlap */
  }
  else
  {
    /* shorten n to get desired qsize */
    np -= zeros;
  }

  ASSERT(tsize - dsize + 1 == qsize);
  mpn_div_q(qp, np, tsize, dp, dsize, tp);

  /* strip possible zero high limb */
  high_zero = (qp[qsize - 1] == 0);
  qsize -= high_zero;
  exp -= high_zero;

  EXP(r) = exp;
  SIZ(r) = sign_quotient >= 0 ? qsize : -qsize;

  TMP_FREE;
}

void mpf_mul_ui(mpf_ptr r, mpf_srcptr u, unsigned long int v)
{
  mp_srcptr up;
  mp_size_t usize;
  mp_size_t size;
  mp_size_t prec, excess;
  mp_limb_t cy_limb, vl, cbit, cin;
  mp_ptr rp;

  usize = u->_mp_size;
  if (UNLIKELY(v == 0) || UNLIKELY(usize == 0))
  {
    r->_mp_size = 0;
    r->_mp_exp = 0;
    return;
  }

#if BITS_PER_ULONG > GMP_NUMB_BITS /* avoid warnings about shift amount */
  if (v > GMP_NUMB_MAX)
  {
    mpf_t vf;
    mp_limb_t vp[2];
    vp[0] = v & GMP_NUMB_MASK;
    vp[1] = v >> GMP_NUMB_BITS;
    PTR(vf) = vp;
    SIZ(vf) = 2;
    ASSERT_CODE(PREC(vf) = 2);
    EXP(vf) = 2;
    mpf_mul(r, u, vf);
    return;
  }
#endif

  size = ABS(usize);
  prec = r->_mp_prec;
  up = u->_mp_d;
  vl = v;
  excess = size - prec;
  cin = 0;

  if (excess > 0)
  {
    /* up is bigger than desired rp, shorten it to prec limbs and
       determine a carry-in */

    mp_limb_t vl_shifted = vl << GMP_NAIL_BITS;
    mp_limb_t hi, lo, next_lo, sum;
    mp_size_t i;

    /* high limb of top product */
    i = excess - 1;
    umul_ppmm(cin, lo, up[i], vl_shifted);

    /* and carry bit out of products below that, if any */
    for (;;)
    {
      i--;
      if (i < 0)
        break;

      umul_ppmm(hi, next_lo, up[i], vl_shifted);
      lo >>= GMP_NAIL_BITS;
      ADDC_LIMB(cbit, sum, hi, lo);
      cin += cbit;
      lo = next_lo;

      /* Continue only if the sum is GMP_NUMB_MAX.  GMP_NUMB_MAX is the
         only value a carry from below can propagate across.  If we've
         just seen the carry out (ie. cbit!=0) then sum!=GMP_NUMB_MAX,
         so this test stops us for that case too.  */
      if (LIKELY(sum != GMP_NUMB_MAX))
        break;
    }

    up += excess;
    size = prec;
  }

  rp = r->_mp_d;
#if HAVE_NATIVE_mpn_mul_1c
  cy_limb = mpn_mul_1c(rp, up, size, vl, cin);
#else
  cy_limb = mpn_mul_1(rp, up, size, vl);
  __GMPN_ADD_1(cbit, rp, rp, size, cin);
  cy_limb += cbit;
#endif
  rp[size] = cy_limb;
  cy_limb = cy_limb != 0;
  r->_mp_exp = u->_mp_exp + cy_limb;
  size += cy_limb;
  r->_mp_size = usize >= 0 ? size : -size;
}

void mpf_random2(mpf_ptr x, mp_size_t xs, mp_exp_t exp)
{
  mp_size_t xn;
  mp_size_t prec;
  mp_limb_t elimb;

  xn = ABS(xs);
  prec = PREC(x);

  if (xn == 0)
  {
    EXP(x) = 0;
    SIZ(x) = 0;
    return;
  }

  if (xn > prec + 1)
    xn = prec + 1;

  /* General random mantissa.  */
  mpn_random2(PTR(x), xn);

  /* Generate random exponent.  */
  _gmp_rand(&elimb, RANDS, GMP_NUMB_BITS);
  exp = ABS(exp);
  exp = elimb % (2 * exp + 1) - exp;

  EXP(x) = exp;
  SIZ(x) = xs < 0 ? -xn : xn;
}

void mpf_clears(mpf_ptr x, ...)
{
  va_list ap;

  va_start(ap, x);

  do
  {
    __GMP_FREE_FUNC_LIMBS(PTR(x), PREC(x) + 1);
    x = va_arg(ap, mpf_ptr);
  } while (x != NULL);

  va_end(ap);
}
