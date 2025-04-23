/* bug-3080.c
   A crash in code generation for some I/O operations.
 */
 
#include <testfwk.h>

#if defined(__SDCC_pdk13) || defined(__SDCC_pdk14) || defined(__SDCC_pdk15)

#define PB_ADDR             0x13
__sfr __at(PB_ADDR)          pb;
#define PB                   pb

unsigned char test;

void f(void)
{
  PB = ~PB;
}

void g(void)
{
  PB |= 0;
}

#endif

void
testBug(void)
{
}

