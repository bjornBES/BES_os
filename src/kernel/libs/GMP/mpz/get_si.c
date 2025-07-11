/* mpz_get_si(integer) -- Return the least significant digit from INTEGER.

Copyright 1991, 1993-1995, 2000-2002, 2006, 2012 Free Software Foundation, Inc.

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

signed long int mpz_get_si(mpz_srcptr z)
{
  mp_ptr zp = PTR(z);
  mp_size_t size = SIZ(z);
  mp_limb_t zl = zp[0];

#if GMP_NAIL_BITS != 0
  if (ULONG_MAX > GMP_NUMB_MAX && ABS(size) >= 2)
    zl |= zp[1] << GMP_NUMB_BITS;
#endif

  if (size > 0)
    return zl & LONG_MAX;
  else if (size < 0)
    /* This expression is necessary to properly handle 0x80000000 */
    return -1 - (long)((zl - 1) & LONG_MAX);
  else
    return 0;
}
