#include "longlong.h"
#include "defaultincs.h"

UQItype __clz_tab[256] = {
  8, 7, 6, 6, 5, 5, 5, 5,
  4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3,
  3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
};

void sdiv_qrnnd(mp_limb_t *q, mp_limb_t *r, mp_limb_t n1, mp_limb_t n0, mp_limb_t d)
{
    int64_t dividend = ((int64_t)n1 << 32) | n0;
    *q = (int32_t)(dividend / d);
    *r = (int32_t)(dividend % d);
}
