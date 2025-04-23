/*
   bug-2842.c
   Duplicate typedefs to same type were rejected even in C11 mode.
*/

#include <testfwk.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
typedef int i_t;
typedef int i_t;
#endif

void testBug(void)
{
}

