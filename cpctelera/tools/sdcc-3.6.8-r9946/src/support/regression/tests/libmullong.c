/* Test _mullong.c from library

    type: asm, c
 */
#include <testfwk.h>

#define type_{type}

#if defined(PORT_HOST)
#  include "sdccconf.h"
#  define mullong(a,b) mullong_wrapper(a,b)
#  if defined(type_c) && !defined(WORDS_BIGENDIAN)
#    define _SDCC_NO_ASM_LIB_FUNCS 1
#    define __near
#    define long int
#    include "device/lib/_mullong.c"
#  endif
#else
#  if defined(type_c)
#    define _SDCC_NO_ASM_LIB_FUNCS 1
#  endif
#  include "device/lib/_mullong.c"
#  define mullong _mullong
#endif

/* gcc 2.95.2 on usf-cf-x86-linux-1 (debian 2.2) has a bug with
 * packing structs
 */
#if defined(PORT_HOST)

#define TYPE_TARGET_CHAR  TYPE_BYTE
#define TYPE_TARGET_INT   TYPE_WORD
#define TYPE_TARGET_LONG  TYPE_DWORD
#define TYPE_TARGET_UCHAR TYPE_UBYTE
#define TYPE_TARGET_UINT  TYPE_UWORD
#define TYPE_TARGET_ULONG TYPE_UDWORD

#if defined(type_c) && !defined(WORDS_BIGENDIAN)
struct
{
  char c1;
  short i;
  char c2;
} pack_test;

TYPE_TARGET_LONG
mullong_wrapper (TYPE_TARGET_LONG a, TYPE_TARGET_LONG b)
{
  if (sizeof(pack_test) == 4)
    /* length of struct ok: use SDCC library */
    return _mullong (a, b);
  else
    {
      /* buggy gcc: use generic multiplication */
      return a * b;
    }
}

#else

TYPE_TARGET_LONG
mullong_wrapper (TYPE_TARGET_LONG a, TYPE_TARGET_LONG b)
{
    return a * b;
}

#endif

#endif

void
testlibmullong(void)
{
  ASSERT(mullong (         0,          0) ==          0);
  ASSERT(mullong (     0x100,      0x100) ==    0x10000);
  ASSERT(mullong (0x01020304,          3) == 0x0306090c);
  ASSERT(mullong (         3, 0x01020304) == 0x0306090c);
  ASSERT(mullong (0x000000ff,          2) == 0x000001fe);
  ASSERT(mullong (         2, 0x000000ff) == 0x000001fe);
  ASSERT(mullong (0x00007fff,          4) == 0x0001fffc);
  ASSERT(mullong (         4, 0x00007fff) == 0x0001fffc);
  ASSERT(mullong (0x003fffff,          8) == 0x01fffff8);
  ASSERT(mullong (         8, 0x003fffff) == 0x01fffff8);

  ASSERT(mullong (      0x33,       0x34) == 0x00000a5c);
  ASSERT(mullong (      0x34,       0x33) == 0x00000a5c);
  ASSERT(mullong (    0x3334,     0x3536) == 0x0aa490f8);
  ASSERT(mullong (    0x3536,     0x3334) == 0x0aa490f8);
  ASSERT(mullong (  0x333435,   0x363738) == 0x0e98ce98);
  ASSERT(mullong (  0x363738,   0x333435) == 0x0e98ce98);
  ASSERT(mullong (0x33343536, 0x3738393a) == 0x777d143c);
  ASSERT(mullong (0x3738393a, 0x33343536) == 0x777d143c);

  ASSERT(mullong (      0xff,       0xfe) == 0x0000fd02);
  ASSERT(mullong (      0xfe,       0xff) == 0x0000fd02);
  ASSERT(mullong (    0xfffe,     0xfdfc) == 0xfdfa0408);
  ASSERT(mullong (    0xfdfc,     0xfffe) == 0xfdfa0408);
  ASSERT(mullong (  0xfffefd,   0xfcfbfa) == 0xfa0d1212);
  ASSERT(mullong (  0xfcfbfa,   0xfffefd) == 0xfa0d1212);
  ASSERT(mullong (0xfffefdfc, 0xfbfaf9f8) == 0x20282820);
  ASSERT(mullong (0xfbfaf9f8, 0xfffefdfc) == 0x20282820);

  ASSERT(mullong (0xff000000, 0xff000000) ==          0);
  ASSERT(mullong (0xffff0000, 0xffff0000) ==          0);
  ASSERT(mullong (0xfffffe00, 0xfffffd00) == 0x00060000);
  ASSERT(mullong (0xfffffd00, 0xfffffe00) == 0x00060000);
  ASSERT(mullong (0xfffffefd, 0xfffffcfb) == 0x00030e0f);
  ASSERT(mullong (0xfffffcfb, 0xfffffefd) == 0x00030e0f);

  ASSERT(mullong (0xffffffff, 0xffffffff) ==          1);
}
