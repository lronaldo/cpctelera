/*
   bug3537338.c
*/

#include <testfwk.h>

#if defined (__SDCC)
 #include <sdcc-lib.h> /* just to get _AUTOMEM or _STATMEM */
#else
 #define _STATMEM
#endif

int globals[3] = {1, 2, 3};
int _STATMEM * ptr = &globals[1];

int get_signed(signed char index)
{
	return *(ptr+index);
}

int get_signedi(int index)
{
	return *(ptr+index);
}

int get_unsigned(unsigned char index)
{
	return *(ptr+index);
}

void testBug(void)
{
#ifndef __SDCC_pic16
    ASSERT (get_signed(-1) == globals[0]);
    ASSERT (get_signedi(-1) == globals[0]);
    ASSERT (get_signed(1) == globals[2]);
    ASSERT (get_unsigned(1) == globals[2]);
#endif
}

