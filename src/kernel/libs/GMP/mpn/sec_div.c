/* mpn_sec_div_qr, mpn_sec_div_r -- Compute Q = floor(U / V), U = U mod V.
   Side-channel silent under the assumption that the used instructions are
   side-channel silent.

   Contributed to the GNU project by Torbjörn Granlund.

Copyright 2011-2015 Free Software Foundation, Inc.

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

#include "libs/GMP/defaultincs.h"

#define Q(q)
#define OPERATION_sec_div_r 1

/*
mp_size_t mpn_sec_div_r_itch (mp_size_t nn, mp_size_t dn)
{
      return nn + 2 * dn + 2;
      
}
*/

/*
void mpn_sec_div_r (Q(mp_ptr qp) mp_ptr np, mp_size_t nn, mp_srcptr dp, mp_size_t dn, mp_ptr tp)
{
      mp_limb_t d1, d0;
      unsigned int cnt;
      mp_limb_t inv32;
      
      ASSERT (dn >= 1);
      ASSERT (nn >= dn);
      ASSERT (dp[dn - 1] != 0);
      
      d1 = dp[dn - 1];
      count_leading_zeros (cnt, d1);
      
      if (cnt != 0)
      {
            mp_limb_t qh, cy;
            mp_ptr np2, dp2;
            dp2 = tp;					
            mpn_lshift (dp2, dp, dn, cnt);
            
            np2 = tp + dn;
            cy = mpn_lshift (np2, np, nn, cnt);
            np2[nn++] = cy;
            
            d0 = dp2[dn - 1];
            d0 += (~d0 != 0);
            invert_limb (inv32, d0);
            
            #if OPERATION_sec_div_qr
            qh = mpn_sec_pi1_div_qr (np2 + dn, np2, nn, dp2, dn, inv32, tp + nn + dn);
            ASSERT (qh == 0);		
            MPN_COPY (qp, np2 + dn, nn - dn - 1);
            qh = np2[nn - 1];
            #else
            mpn_sec_pi1_div_r (np2, nn, dp2, dn, inv32, tp + nn + dn);
            #endif
            
            mpn_rshift (np, np2, dn, cnt);
            
            #if OPERATION_sec_div_qr
            return qh;
            #endif
      }
      else
      {
            d0 = dp[dn - 1];
            d0 += (~d0 != 0);
            invert_limb (inv32, d0);
            
            #if OPERATION_sec_div_qr
            return mpn_sec_pi1_div_qr (qp, np, nn, dp, dn, inv32, tp);
            #else
            mpn_sec_pi1_div_r (np, nn, dp, dn, inv32, tp);
            #endif
      }
}
*/
