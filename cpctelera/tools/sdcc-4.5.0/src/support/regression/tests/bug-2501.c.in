/* bug-2501.c
   A bug in the z80 and stm8 register allocators when dealing with results from dummy volatile long long reads via pointer.

   type: long, long long
 */

#include <testfwk.h>

#pragma disable_warning 85

typedef {type} l_t;

extern l_t x_llabs(l_t i);
extern char *x_lltoa(l_t i, char *buf, int radix);

struct long_s
{
   unsigned long ls32;
   unsigned long ms32;
};

union u64_s
{
   l_t a;
   struct long_s b;
};

#ifndef __SDCC_pdk14 // Lack of memory
char buff[10];

union u64_s z = {0xaa55};
#endif

void testBug(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
   z.a = x_llabs(z.a);
   x_lltoa(z.a, buff, 10);
#endif
}

l_t x_llabs(l_t i)
{
   return i >= 0 ? i : -1;
}

char *x_lltoa(l_t i, char *buf, int radix)
{
   ASSERT (i == 0xaa55);
}

