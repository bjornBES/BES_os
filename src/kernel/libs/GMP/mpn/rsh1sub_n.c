#include "libs/GMP/defaultincs.h"
#include "debug.h"

int byte_overlap_p(const void *v_xp, mp_size_t xsize,
                   const void *v_yp, mp_size_t ysize)
{
    const char *xp = (const char *)v_xp;
    const char *yp = (const char *)v_yp;

    ASSERT(xsize >= 0);
    ASSERT(ysize >= 0);

    /* no wraparounds */
    ASSERT(xp + xsize >= xp);
    ASSERT(yp + ysize >= yp);

    if (xp + xsize <= yp)
        return 0;

    if (yp + ysize <= xp)
        return 0;

    return 1;
}
int overlap_p(mp_srcptr xp, mp_size_t xsize, mp_srcptr yp, mp_size_t ysize)
{
    return byte_overlap_p(xp, xsize * GMP_LIMB_BYTES,
                          yp, ysize * GMP_LIMB_BYTES);
}
int overlap_fullonly_p(mp_srcptr dst, mp_srcptr src, mp_size_t size)
{
    return (dst == src || !overlap_p(dst, size, src, size));
}
int overlap_fullonly_two_p(mp_srcptr dst, mp_srcptr src1, mp_srcptr src2,
                                  mp_size_t size)
{
    return (overlap_fullonly_p(dst, src1, size) && overlap_fullonly_p(dst, src2, size));
}

mp_limb_t mpn_rsh1add_n(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
    mp_limb_t cya, cys;

    /*
    
    ASSERT(overlap_fullonly_two_p(rp, up, vp, n));
    ASSERT(n >= 1);
    ASSERT_MPN(up, n);
    ASSERT_MPN(vp, n);
    */

    cya = mpn_add_n(rp, up, vp, n);
    cys = mpn_rshift(rp, rp, n, 1) >> (GMP_NUMB_BITS - 1);
    rp[n - 1] |= cya << (GMP_NUMB_BITS - 1);
    return cys;
}
mp_limb_t mpn_rsh1sub_n(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n)
{
    mp_limb_t cya, cys;

    /*
    ASSERT(overlap_fullonly_two_p(rp, up, vp, n));
    ASSERT(n >= 1);
    ASSERT_MPN(up, n);
    ASSERT_MPN(vp, n);
    */

    cya = mpn_sub_n(rp, up, vp, n);
    cys = mpn_rshift(rp, rp, n, 1) >> (GMP_NUMB_BITS - 1);
    rp[n - 1] |= cya << (GMP_NUMB_BITS - 1);
    return cys;
}

mp_limb_t mpn_rsh1add_nc(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t xp)
{
    FUNC_NOT_IMPLEMENTED("MPN", "mpn_rsh1add_nc");
    return 0;
}

mp_limb_t mpn_rsh1sub_nc(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t xp)
{
    FUNC_NOT_IMPLEMENTED("MPN", "mpn_rsh1sub_nc");
    return 0;
}
