/* mpz_abs(dst, src) -- Assign the absolute value of SRC to DST.

Copyright 1991, 1993-1995, 2001, 2012 Free Software Foundation, Inc.

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

#define __GMP_FORCE_mpz_abs 1

#include "libs/GMP/gmp-impl.h"

void
mpz_abs (mpz_ptr w, mpz_srcptr u)
{
  mp_ptr wp;
  mp_srcptr up;
  mp_size_t size;

  size = ABSIZ (u);

  if (u != w)
    {
      wp = MPZ_NEWALLOC (w, size);

      up = PTR (u);

      MPN_COPY (wp, up, size);
    }

  SIZ (w) = size;
}
