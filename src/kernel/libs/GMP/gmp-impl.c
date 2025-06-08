#include "libs/GMP/defaultincs.h"

size_t digits_in_basegt2_from_bits(size_t totbits, int base) {
    double log2_base = (double)log2((int)base);  // You need to `#include <math.h>`
    return (size_t)((double)totbits / log2_base + 0.999999999);  // Equivalent to ceiling
}

/*
size_t mpn_sizeinbase(mp_srcptr ptr, mp_size_t size, int base) {
    int __lb_base = 0;
    int __cnt = 0;
    size_t __totbits;
    
    assert(size >= 0);
    assert(base >= 2);
    assert(base < numberof(mp_bases));
    
    if (size == 0) {
        return 1;
    } else {
        count_leading_zeros(__cnt, ptr[size - 1]);
        __totbits = (size_t)size * GMP_NUMB_BITS - (__cnt - GMP_NAIL_BITS);
        
        if (POW2_P(base)) {
            __lb_base = mp_bases[base].big_base;
            return (__totbits + __lb_base - 1) / __lb_base;
        } else {
            return digits_in_basegt2_from_bits(__totbits, base);
        }
    }
}
*/

size_t mpn_sizeinbase_2exp(mp_srcptr ptr, mp_size_t size, unsigned long base2exp) {
    int __cnt;
    mp_bitcnt_t __totbits;

    assert(size > 0);
    assert(ptr[size - 1] != 0);

    count_leading_zeros(__cnt, ptr[size - 1]);
    __totbits = (mp_bitcnt_t)size * GMP_NUMB_BITS - (__cnt - GMP_NAIL_BITS);

    return (__totbits + base2exp - 1) / base2exp;
}

mp_limb_t mpn_divexact_by3(mp_ptr dst, mp_srcptr src, mp_size_t size) 
{
    mp_limb_t ret = mpn_bdiv_dbm1(dst, src, size, __GMP_CAST(mp_limb_t, GMP_NUMB_MASK / 3));
    return (3 & ret);
}

mp_limb_t mpn_divexact_by15(mp_ptr dst, mp_srcptr src, mp_size_t size)
{
    mp_limb_t ret = mpn_bdiv_dbm1(dst, src, size, __GMP_CAST(mp_limb_t, GMP_NUMB_MASK / 15));
    return (15 & 1 * ret);
}