/* mini-gmp, a minimalistic implementation of a GNU GMP subset.

Copyright 2011-2015, 2017, 2019-2020 Free Software Foundation, Inc.

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

/* About mini-gmp: This is a minimal implementation of a subset of the
   GMP interface. It is intended for inclusion into applications which
   have modest bignums needs, as a fallback when the real GMP library
   is not installed.

   This file defines the public interface. */

#ifndef __MINI_GMP_H__
#define __MINI_GMP_H__

/* For size_t */
#include <stddef.h>
#include "libs/GMP/gmp-impl.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef MINI_GMP_LIMB_TYPE
#define MINI_GMP_LIMB_TYPE long
#endif

struct gmp_div_inverse
{
  /* Normalization shift count. */
  unsigned shift;
  /* Normalized divisor (d0 unused for mpn_div_qr_1) */
  mp_limb_t d1, d0;
  /* Inverse, for 2/1 or 3/2. */
  mp_limb_t di;
};

unsigned mpn_base_power_of_two_p(unsigned b);
size_t mpn_get_str_bits(unsigned char *sp, unsigned bits, mp_srcptr up, mp_size_t un);
mp_ptr gmp_xalloc_limbs(mp_size_t size);
mp_ptr gmp_xrealloc_limbs(mp_ptr old, mp_size_t size);
mp_size_t mpn_normalized_size(mp_srcptr xp, mp_size_t n);

mp_size_t mpn_set_str_other(mp_ptr rp, const unsigned char *sp, size_t sn, mp_limb_t b, const mpn_base_info *info);
void mpn_get_base_info(mpn_base_info *info, mp_limb_t b);
void mpn_div_qr_1_invert(struct gmp_div_inverse *inv, mp_limb_t d);
size_t mpn_limb_get_str(unsigned char *sp, mp_limb_t w, const struct gmp_div_inverse *binv);
mp_limb_t mpn_div_qr_1_preinv(mp_ptr qp, mp_srcptr np, mp_size_t nn, const struct gmp_div_inverse *inv);


mp_bitcnt_t mpn_common_scan(mp_limb_t limb, mp_size_t i, mp_srcptr up, mp_size_t un, mp_limb_t ux);
mp_limb_t mpn_invert_3by2 (mp_limb_t, mp_limb_t);
#define mpn_invert_limb(x) mpn_invert_3by2 ((x), 0)

/* This long list taken from gmp.h. */
/* For reference, "defined(EOF)" cannot be used here.  In g++ 2.95.4,
   <iostream> defines EOF but not FILE.  */
#if defined (FILE)                                              \
  || defined (H_STDIO)                                          \
  || defined (_H_STDIO)               /* AIX */                 \
  || defined (_STDIO_H)               /* glibc, Sun, SCO */     \
  || defined (_STDIO_H_)              /* BSD, OSF */            \
  || defined (__STDIO_H)              /* Borland */             \
  || defined (__STDIO_H__)            /* IRIX */                \
  || defined (_STDIO_INCLUDED)        /* HPUX */                \
  || defined (__dj_include_stdio_h_)  /* DJGPP */               \
  || defined (_FILE_DEFINED)          /* Microsoft */           \
  || defined (__STDIO__)              /* Apple MPW MrC */       \
  || defined (_MSL_STDIO_H)           /* Metrowerks */          \
  || defined (_STDIO_H_INCLUDED)      /* QNX4 */		\
  || defined (_ISO_STDIO_ISO_H)       /* Sun C++ */		\
  || defined (__STDIO_LOADED)         /* VMS */			\
  || defined (__DEFINED_FILE)         /* musl */
size_t mpz_out_str (FILE *, int, const mpz_t);
#endif

#if defined (__cplusplus)
}
#endif
#endif /* __MINI_GMP_H__ */
