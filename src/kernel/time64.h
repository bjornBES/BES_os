#pragma once

#include "types.h"

/* Define __TIME64_T_TYPE so that it is always a 64-bit type.  */

#if __TIMESIZE == 64
/* If we already have 64-bit time type then use it.  */
# define __TIME64_T_TYPE		__TIME_T_TYPE
#else
/* Define a 64-bit time type alongsize the 32-bit one.  */
# define __TIME64_T_TYPE		__SQUAD_TYPE
#endif