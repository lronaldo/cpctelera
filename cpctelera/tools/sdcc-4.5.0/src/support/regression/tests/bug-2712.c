/*
   bug-2458.c
   In backends that support 16x16->32 multiplication, there was a bug when
   operands to such a multiplication were used in multiple such multiplications.
*/

#include <testfwk.h>

#ifndef __SDCC_pdk14 // Lack of memory - see RFE #606
int dummy(int x)
{
   return x;
}

void dummy2(long x)
{
   ASSERT(x == 42 * 42 || x == -42 * 42);
}
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
   int i, j;

   // Double use of i and j.
   i = dummy(42);
   j = dummy(42);
   dummy2((long)i * (long)j);
   i = -i;
   dummy2((long)i * (long)j);

   // Single use of i and j.
   i = dummy(42);
   j = dummy(42);
   dummy2((long)i * (long)j);
#endif
}

