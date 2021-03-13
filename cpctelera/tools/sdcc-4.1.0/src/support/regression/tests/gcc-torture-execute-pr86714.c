/*
   pr86714.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <string.h>

#ifdef __SDCC
#pragma disable_warning 147 
#endif

/* PR tree-optimization/86714 - tree-ssa-forwprop.c confused by too
   long initializer

   The excessively long initializer for a[0] is undefined but this
   test verifies that the excess elements are not considered a part
   of the value of the array as a matter of QoI.  */

const char a[2][3] = { "1234", "xyz" };
char b[6];

void *pb = b;

void
testTortureExecute (void)
{
   memcpy (b, a, 4);
   memset (b + 4, 'a', 2);

   if (b[0] != '1' || b[1] != '2' || b[2] != '3'
       || b[3] != 'x' || b[4] != 'a' || b[5] != 'a')
     ASSERT (0);

   if (memcmp (pb, "123xaa", 6))
     ASSERT (0);

   return;
}

