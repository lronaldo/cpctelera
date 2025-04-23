/*
   bug1-2195.c

   frontend couldn't handle the addition to void * used in comparison.
*/

#include <testfwk.h>

void f(void *b, unsigned j)
{
    unsigned char *i = b;
    i <= b + j;
}

void testBug(void)
{
    f(0, 0);
}

