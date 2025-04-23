/*
   bug-3132.c
   A bug in pdk code generation for jump on & if all bytes of the mask are 0xff or 0x00 and the highest byte is 0x00.
 */

#include <testfwk.h>

#include <stdint.h>

volatile uint8_t block = 0;

void f(void)
{

  for( uint16_t t=0; t<0x101; t++ )
  {
    if( 0 == (t&0xFF) ) // BUG: this compare is compiled to "if( 0 == t )"
      block++;
  }
}

void
testBug (void)
{
  f();
  ASSERT (block == 2);
}

