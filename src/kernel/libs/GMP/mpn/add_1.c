/* mpn_add_1 - add limb to mpn.

Copyright 2001 Free Software Foundation, Inc.

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

#define __GMP_FORCE_mpn_add_1 1

#include "libs/GMP/defaultincs.h"

mp_limb_t mpn_add_1(mp_ptr __gmp_dst, mp_srcptr __gmp_src, mp_size_t __gmp_size, mp_limb_t __gmp_n)
{
   mp_limb_t __gmp_c;
   __GMPN_ADD_1(__gmp_c, __gmp_dst, __gmp_src, __gmp_size, __gmp_n);
   return __gmp_c;
}
