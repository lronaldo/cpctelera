/*
   pr60454.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#include <stdint.h>
#include <limits.h>

#define fake_const_swab32(x) ((uint32_t)(			      \
        (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |            \
        (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |            \
        (((uint32_t)(x) & (uint32_t)0x000000ffUL) <<  8) |            \
        (((uint32_t)(x) & (uint32_t)0x0000ff00UL)      ) |            \
        (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

/* Previous version of bswap optimization would detect byte swap when none
   happen. This test aims at catching such wrong detection to avoid
   regressions.  */

#ifndef __SDCC_pdk14 // Lack of memory
uint32_t
fake_swap32 (uint32_t in)
{
  return fake_const_swab32 (in);
}
#endif

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  if (sizeof (uint32_t) * CHAR_BIT != 32)
    return;
  if (fake_swap32 (0x12345678UL) != 0x78567E12UL)
    ASSERT (0);
#endif
}
