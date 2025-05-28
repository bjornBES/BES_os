#pragma once

#include "wordsize.h"

#if defined __x86_64__ && defined __ILP32__
/* For x32, time is 64-bit even though word size is 32-bit.  */
# define __TIMESIZE	64
#else
/* For others, time size is word size.  */
# define __TIMESIZE	__WORDSIZE
#endif
