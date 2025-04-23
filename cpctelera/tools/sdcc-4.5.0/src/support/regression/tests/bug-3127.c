/* bug-3126.c
   32-bit addition used a register that held already-computed result bytes as temporary for upper bytes of literal operand.
 */

#include <testfwk.h>

#include <limits.h>
#include <stdarg.h>

#ifndef __SDCC_pdk14 // Lack of memory

#if ULONG_MAX == 0xffffffff
typedef unsigned long UINT4;
#elif UINT_MAX == 0xffffffff
typedef unsigned UINT4;
#elif USHRT_MAX == 0xffffffff
#else
#error No suitable UINT4 type
#endif

void check(const char *format, ...)
{
  va_list arg;
  static int i;

  va_start (arg, format);
  
  ASSERT(va_arg(arg, UINT4) == (i ? 0xd76ae33a : 0x67452301));
  ASSERT(va_arg(arg, UINT4) == 0xefcdab89);
  ASSERT(va_arg(arg, UINT4) == 0x98badcfe);
  ASSERT(va_arg(arg, UINT4) == (i > 1 ? 0xc8d2cb98 : 0x10325476));
  
  i++;
  
  va_end (arg);
}

void func(UINT4 *buf, UINT4 *in)
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  check("%lx %lx %lx %lx\n",a,b,c,d);

  {( a ) +=  ((( ( b ) ) & ( ( c ) )) | ((~ ( b ) ) & ( ( d ) )))  + ( in[ 0] ) + (UINT4)( 3614090360 );} // Bug affected addition here

  check("%lx %lx %lx %lx\n",a,b,c,d);

  {( d ) +=  ((( ( a ) ) & ( ( b ) )) | ((~ ( a ) ) & ( ( c ) )))  + ( in[ 1] ) + (UINT4)( 3905402710 ); }

  check("%lx %lx %lx %lx\n",a,b,c,d);
}
#endif

void
testBug (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
   UINT4 buf[] = {0x67452301,0xefcdab89,0x98badcfe,0x10325476};
   UINT4 in[] = {0x3ec3,0x0,0x3dc3,0x0,0x3ac3};

   func(buf, in);
#endif
}

