/* bug-3463.c
   A codegen issue related to writing a bit-filed to a pointer (similar to bug #3461, but this test case triggers it for different backends).
 */

#include <testfwk.h>
#include <stdlib.h>
#include <stdint.h>

unsigned char *ptemp;
unsigned char temp;

struct {
  unsigned char b0 : 1;
  unsigned char b1 : 1;
  unsigned char b2 : 1;
  unsigned char b3 : 1;
  unsigned char b4_7 : 4;
} sb_bf;


void
test(void)
{
  ptemp=&temp;
  sb_bf.b4_7=2;
  ASSERT( sb_bf.b4_7==2 );
  *ptemp=0xFF;
  ASSERT( *ptemp == 0xFF );
  *ptemp=sb_bf.b4_7;
  ASSERT( *ptemp == 2 );
  ASSERT( *ptemp != 0x2F );
  *ptemp=(unsigned char) sb_bf.b4_7;
  ASSERT( *ptemp == 2 );
  ASSERT( *ptemp != 0x2F );
  temp=0xFF;
  ASSERT( temp == 0xFF );
  temp=sb_bf.b4_7;
  ASSERT( temp == 2 );
}

