/* mpz_init_set (src_integer) -- Make a new multiple precision number with
   a value copied from SRC_INTEGER.

Copyright 1991, 1993, 1994, 1996, 2000-2002, 2012 Free Software Foundation,
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

void
mpz_init_set (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp, up;
  mp_size_t usize, size;

  usize = SIZ (u);
  size = ABS (usize);

  ALLOC (w) = MAX (size, 1);
  wp = __GMP_ALLOCATE_FUNC_LIMBS (ALLOC (w));

  PTR (w) = wp;
  up = PTR (u);

  MPN_COPY (wp, up, size);
  SIZ (w) = usize;
}
