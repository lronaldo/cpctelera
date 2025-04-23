/* bug-3626.c
   The Rabbit assembler failed to assemble some Rabbit instructions not used by the compiler.
*/

#include <testfwk.h>

void
f (void)
{
#if defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka)
  __asm__ ("ex de', hl");
  __asm__ ("ld bc', bc");

  __asm__ ("ld de', bc");
  __asm__ ("ld hl', bc");

  __asm__ ("ld bc', de");
  __asm__ ("ld de', de");
  __asm__ ("ld hl', de");
#endif
}

void
testBug (void)
{
}

