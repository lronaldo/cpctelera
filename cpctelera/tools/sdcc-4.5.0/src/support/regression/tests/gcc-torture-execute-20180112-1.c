/*
20180112-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stdint.h>

/* PR rtl-optimization/83565 */
/* Testcase by Sergei Trofimovich <slyfox@inbox.ru> */

typedef uint32_t u32;

u32 bug (u32 * result);
u32 bug (u32 * result)
{
  volatile u32 ss = 0xFFFFffff;
  volatile u32 d  = 0xEEEEeeee;
  u32 tt = d & 0x00800000;
  u32 r  = tt << 8;

  r = (r >> 31) | (r <<  1);

  u32 u = r^ss;
  u32 off = u >> 1;

  *result = tt;
  return off;
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  u32 l;
  u32 off = bug(&l);
  if (off != 0x7fffffff)
    ASSERT (0);
  return;
#endif
}
