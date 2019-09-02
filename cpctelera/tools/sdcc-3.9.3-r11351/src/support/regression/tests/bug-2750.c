/*
   bug1-2195.c

   frontend used | in place of ^.
*/

#include <testfwk.h>

typedef struct
  {
  unsigned LATC0                : 1;
  unsigned LATC1                : 1;
  unsigned LATC2                : 1;
  unsigned LATC3                : 1;
  unsigned LATC4                : 1;
  unsigned LATC5                : 1;
  unsigned LATC6                : 1;
  unsigned LATC7                : 1;
  } LATCbits_t;

volatile LATCbits_t LATCbits;

unsigned short mask;

void set_bits(unsigned char sn)
{
    LATCbits.LATC0 = ((mask >> sn) & 1) ^ ~0x1;
}

volatile unsigned char bit;

void testBug(void)
{
    LATCbits.LATC0 = 1;
    mask = 0x00;
    bit = 0;
    set_bits(bit);
    ASSERT(!LATCbits.LATC0);
}

