#include "stdlib.h"

#include "libs/GMP/gmp-impl.h"
#include "libs/GMP/gmpdefault.h"

int gmp_errno = 0;

void __gmp_exception (int error_bit)
{
  gmp_errno |= error_bit;
  __gmp_junk = 10 / __gmp_0;
  abort ();
}

void __gmp_sqrt_of_negative (void)
{
  __gmp_exception (GMP_ERROR_SQRT_OF_NEGATIVE);
}
void __gmp_divide_by_zero (void)
{
  __gmp_exception (GMP_ERROR_DIVISION_BY_ZERO);
}
