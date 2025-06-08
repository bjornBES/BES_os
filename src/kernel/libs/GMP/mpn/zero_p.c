/* mpn_zero_p (x,xsize) -- Return 1 if X is zero, 0 if it is non-zero.

Copyright 2015 Free Software Foundation, Inc.

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

#define __GMP_FORCE_mpn_zero_p 1

#include "libs/GMP/defaultincs.h"

int mpn_zero_p(mp_srcptr __gmp_p, mp_size_t __gmp_n)
{
   /* if (__GMP_LIKELY (__gmp_n > 0)) */
   do
   {
      if (__gmp_p[--__gmp_n] != 0)
         return 0;
   } while (__gmp_n != 0);
   return 1;
}
