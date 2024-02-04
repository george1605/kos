#ifndef _STUBS_
#define _STUBS_ 1

#if defined __x86_64__
# define __WORDSIZE        64
# define __WORDSIZE_COMPAT32        1
#else
# define __WORDSIZE        32
#endif

#if __WORDSIZE == 32
# include "gnu/stubs-32.h"
#elif __WORDSIZE == 64
# include <gnu/stubs-64.h>
#else
# error "unexpected value for __WORDSIZE macro"
#endif

#endif