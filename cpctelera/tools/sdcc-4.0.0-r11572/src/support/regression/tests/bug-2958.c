/*
   bug-2958.c

   The pdk backends didn't handle comparions of __sfr for equality correctly, resulting in invalid asm.
*/

#include <testfwk.h>

// Test for all backends that can have an __sfr at 0x1f.
#if (defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_ez80_z80))
__sfr __at(0x1f) _tm3ct;
#elif defined(__SDCC_mcs51)
__sfr __at(0x99) _tm3ct;
#else
unsigned char _tm3ct;
#endif

void f(void)
{
  unsigned char p=0;
  for(;;)
  {
    if(p == _tm3ct) continue;

    do {

    } while(++p != _tm3ct);
  }
}

void testBug(void)
{
}

