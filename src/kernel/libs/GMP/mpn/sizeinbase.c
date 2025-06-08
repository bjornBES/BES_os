/* mpn_sizeinbase -- approximation to chars required for an mpn.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 1991, 1993-1995, 2001, 2002, 2011, 2012 Free Software Foundation,
Inc.

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

/* Same as mpz_sizeinbase, meaning exact for power-of-2 bases, and either
   exact or 1 too big for other bases.  */

size_t
mpn_sizeinbase(mp_srcptr xp, mp_size_t xsize, int base)
{
    size_t result;

    int __lb_base, __cnt;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    size_t __totbits;
    #pragma GCC diagnostic pop

    ASSERT((xsize) >= 0);
    ASSERT((base) >= 2);
    ASSERT((base) < numberof(mp_bases));

    /* Special case for X == 0.  */
    if ((xsize) == 0)
    {
        result = 1;
    }
    else
    {
        /* Calculate the total number of significant bits of X.  */
        count_leading_zeros(__cnt, (xp)[(xsize) - 1]);
        __totbits = (size_t)(xsize) * GMP_NUMB_BITS - (__cnt - GMP_NAIL_BITS);

        if (POW2_P(base))
        {
            __lb_base = mp_bases[base].big_base;
            result = (__totbits + __lb_base - 1) / __lb_base;
        }
        else
        {
            mp_limb_t _ph, _dummy;
            size_t _nbits = (__totbits);
            umul_ppmm(_ph, _dummy, mp_bases[base].logb2 + 1, _nbits);
            result = _ph + 1;
        }
    }
    return result;
}
