/* mpn_neg - negate an mpn.

Copyright 2001, 2009 Free Software Foundation, Inc.

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

#define __GMP_FORCE_mpn_neg 1

#include "libs/GMP/defaultincs.h"

mp_limb_t mpn_neg(mp_ptr __gmp_rp, mp_srcptr __gmp_up, mp_size_t __gmp_n)
{
   while (*__gmp_up == 0) /* Low zero limbs are unchanged by negation. */
   {
      *__gmp_rp = 0;
      if (!--__gmp_n) /* All zero */
         return 0;
      ++__gmp_up;
      ++__gmp_rp;
   }

   *__gmp_rp = (-*__gmp_up) & GMP_NUMB_MASK;

   if (--__gmp_n) /* Higher limbs get complemented. */
      mpn_com(++__gmp_rp, ++__gmp_up, __gmp_n);

   return 1;
}
