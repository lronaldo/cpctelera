/*
   980701-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int ns_name_skip (unsigned char **x, unsigned char *y)
{
    *x = 0;
    (void)y;
    return 0;
}

unsigned char a[2];

int dn_skipname(unsigned char *ptr, unsigned char *eom)
{
    unsigned char *saveptr = ptr;

    if (ns_name_skip(&ptr, eom) == -1)
	    return (-1);
    return (ptr - saveptr);
}

void
testTortureExecute (void)
{
    ASSERT(dn_skipname (&a[0], &a[1]) != 0);
    return;
}

