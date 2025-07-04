/* mpz_ior -- Logical inclusive or.

Copyright 1991, 1993, 1994, 1996, 1997, 2000, 2001, 2005, 2012, 2013,
2015-2018 Free Software Foundation, Inc.

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
mpz_ior (mpz_ptr res, mpz_srcptr op1, mpz_srcptr op2)
{
  mp_srcptr op1_ptr, op2_ptr;
  mp_size_t op1_size, op2_size;
  mp_ptr res_ptr;
  mp_size_t res_size;
  mp_size_t i;

  op1_size = SIZ(op1);
  op2_size = SIZ(op2);

  if (op1_size < op2_size)
    {
      MPZ_SRCPTR_SWAP (op1, op2);
      MP_SIZE_T_SWAP (op1_size, op2_size);
    }

  op1_ptr = PTR(op1);
  res_ptr = PTR(res);

  if (op2_size >= 0)
    {
      if (res_ptr != op1_ptr)
	{
	  res_ptr = MPZ_REALLOC (res, op1_size);
	  /* No overlapping possible: op1_ptr = PTR(op1); */
	  MPN_COPY (res_ptr + op2_size, op1_ptr + op2_size,
		    op1_size - op2_size);
	}
      if (LIKELY (op2_size != 0))
	mpn_ior_n (res_ptr, op1_ptr, PTR(op2), op2_size);

      SIZ(res) = op1_size;
    }
  else
    {
      mp_ptr opx;
      TMP_DECL;

      TMP_MARK;
      if (op1_size < 0)
	{
	  mp_ptr opy;

	  /* Both operands are negative, so will be the result.
	     -((-OP1) | (-OP2)) = -(~(OP1 - 1) | ~(OP2 - 1)) =
	     = ~(~(OP1 - 1) | ~(OP2 - 1)) + 1 =
	     = ((OP1 - 1) & (OP2 - 1)) + 1      */

	  res_size = -op1_size;

	  /* Possible optimization: Decrease mpn_sub precision,
	     as we won't use the entire res of both.  */
	  TMP_ALLOC_LIMBS_2 (opx, res_size, opy, res_size);
	  mpn_sub_1 (opx, op1_ptr, res_size, (mp_limb_t) 1);
	  op1_ptr = opx;

	  mpn_sub_1 (opy, PTR(op2), res_size, (mp_limb_t) 1);
	  op2_ptr = opy;

	  /* First loop finds the size of the result.  */
	  for (i = res_size; --i >= 0;)
	    if ((op1_ptr[i] & op2_ptr[i]) != 0)
	      break;
	  res_size = i + 1;

	  res_ptr = MPZ_NEWALLOC (res, res_size + 1);

	  if (res_size != 0)
	    {
	      /* Second loop computes the real result.  */
	      mpn_and_n (res_ptr, op1_ptr, op2_ptr, res_size);

	      res_ptr[res_size] = 0;
	      MPN_INCR_U (res_ptr, res_size + 1, 1);
	      res_size += res_ptr[res_size];
	    }
	  else
	    {
	      res_ptr[0] = 1;
	      res_size = 1;
	    }

	  SIZ(res) = -res_size;
	}
      else
	{
	  mp_limb_t cy;
	  mp_size_t count;

	  /* Operand 2 negative, so will be the result.
	     -(OP1 | (-OP2)) = -(OP1 | ~(OP2 - 1)) =
	     = ~(OP1 | ~(OP2 - 1)) + 1 =
	     = (~OP1 & (OP2 - 1)) + 1      */

	  op2_size = -op2_size;

	  res_ptr = MPZ_REALLOC (res, op2_size);
	  op1_ptr = PTR(op1);

	  opx = TMP_ALLOC_LIMBS (op2_size);
	  mpn_sub_1 (opx, PTR(op2), op2_size, (mp_limb_t) 1);
	  op2_ptr = opx;
	  op2_size -= op2_ptr[op2_size - 1] == 0;

	  if (op1_size >= op2_size)
	    {
	      /* We can just ignore the part of OP1 that stretches above OP2,
		 because the result limbs are zero there.  */

	      /* First loop finds the size of the result.  */
	      for (i = op2_size; --i >= 0;)
		if ((~op1_ptr[i] & op2_ptr[i]) != 0)
		  break;
	      res_size = i + 1;
	      count = res_size;
	    }
	  else
	    {
	      res_size = op2_size;

	      /* Copy the part of OP2 that stretches above OP1, to RES.  */
	      MPN_COPY (res_ptr + op1_size, op2_ptr + op1_size, op2_size - op1_size);
	      count = op1_size;
	    }

	  if (res_size != 0)
	    {
	      /* Second loop computes the real result.  */
	      if (LIKELY (count != 0))
		mpn_andn_n (res_ptr, op2_ptr, op1_ptr, count);

	      cy = mpn_add_1 (res_ptr, res_ptr, res_size, (mp_limb_t) 1);
	      if (cy)
		{
		  res_ptr[res_size] = cy;
		  ++res_size;
		}
	    }
	  else
	    {
	      res_ptr[0] = 1;
	      res_size = 1;
	    }

	  SIZ(res) = -res_size;
	}
      TMP_FREE;
    }
}
