/* mpz_tdiv_qr(quot,rem,dividend,divisor) -- Set QUOT to DIVIDEND/DIVISOR,
   and REM to DIVIDEND mod DIVISOR.

Copyright 1991, 1993, 1994, 2000, 2001, 2005, 2011, 2012 Free Software
Foundation, Inc.

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

#include "libs/GMP/gmp-impl.h"
#include "libs/GMP/longlong.h"

void
mpz_tdiv_qr (mpz_ptr quot, mpz_ptr rem, mpz_srcptr num, mpz_srcptr den)
{
  mp_size_t ql;
  mp_size_t ns, ds, nl, dl;
  mp_ptr np, dp, qp, rp;
  TMP_DECL;

  ns = SIZ (num);
  ds = SIZ (den);
  nl = ABS (ns);
  dl = ABS (ds);
  ql = nl - dl + 1;

  if (UNLIKELY (dl == 0))
    DIVIDE_BY_ZERO;

  rp = MPZ_REALLOC (rem, dl);

  if (ql <= 0)
    {
      if (num != rem)
	{
	  np = PTR (num);
	  MPN_COPY (rp, np, nl);
	  SIZ (rem) = SIZ (num);
	}
      /* This needs to follow the assignment to rem, in case the
	 numerator and quotient are the same.  */
      SIZ (quot) = 0;
      return;
    }

  qp = MPZ_REALLOC (quot, ql);

  TMP_MARK;
  np = PTR (num);
  dp = PTR (den);

  /* FIXME: We should think about how to handle the temporary allocation.
     Perhaps mpn_tdiv_qr should handle it, since it anyway often needs to
     allocate temp space.  */

  /* Copy denominator to temporary space if it overlaps with the quotient
     or remainder.  */
  if (dp == rp || dp == qp)
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (dl);
      MPN_COPY (tp, dp, dl);
      dp = tp;
    }
  /* Copy numerator to temporary space if it overlaps with the quotient or
     remainder.  */
  if (np == rp || np == qp)
    {
      mp_ptr tp;
      tp = TMP_ALLOC_LIMBS (nl);
      MPN_COPY (tp, np, nl);
      np = tp;
    }

  mpn_tdiv_qr (qp, rp, 0L, np, nl, dp, dl);

  ql -=  qp[ql - 1] == 0;
  MPN_NORMALIZE (rp, dl);

  SIZ (quot) = (ns ^ ds) >= 0 ? ql : -ql;
  SIZ (rem) = ns >= 0 ? dl : -dl;
  TMP_FREE;
}
