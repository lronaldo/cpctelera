/* bug-3639.c
   The Rabbit assembler failed to assemble some Rabbit instructions not used by the compiler.
*/

#include <testfwk.h>

void
f (void)
{
#if defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka)
  __asm
  ldp hl,(hl)         ; ED 6C
  ldp hl,(ix)         ; DD 6C
  ldp hl,(iy)         ; FD 6C

  ldp hl,(0x1234)     ; ED 6D 34 12
  ldp ix,(0x5678)     ; DD 6D 78 56
  ldp iy,(0x9abc)     ; FD 6D BC 9A

  ldp (hl),hl         ; ED 64
  ldp (ix),hl         ; DD 64
  ldp (iy),hl         ; FD 64

  ldp (0x1234),hl     ; ED 65 34 12
  ldp (0x5678),ix     ; DD 65 78 56
  ldp (0x9abc),iy     ; FD 65 BC 9A
__endasm;
#endif
}

void
testBug (void)
{
}

