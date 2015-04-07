/* Test operandOperation() in SDCCicode.c

    type: char, short, LONG
 */
#include <testfwk.h>

/* 64 bit hosts */
#if defined(__alpha__) || defined(__x86_64__) || defined(__sparc64__) || defined(__PPC64__)
#  define LONG int
#else
#  define LONG long
#endif

typedef signed {type} stype;
typedef unsigned {type} utype;

volatile char is8 = 8;

signed char  sc;
signed short ss;
signed LONG  sl;
unsigned char  uc;
unsigned short us;
unsigned LONG  ul;
volatile signed char  vsc;
volatile signed short vss;
volatile signed LONG  vsl;
volatile unsigned char  vuc;
volatile unsigned short vus;
volatile unsigned LONG  vul;
stype s;
volatile stype vs;
utype u;
volatile utype vu;

unsigned LONG t1, t2;

int
mulWrapper (int a, int b)
{
  return a * b;
}

void
testOpOp (void)
{
  /* mul signedness: usualBinaryConversions() */
  vsc = 0x7f;
  vuc = 0xfe;

  sc = vsc * vsc;
  ASSERT (sc == 1);
  sc = vuc * vsc;
  ASSERT (sc == 2);
  sc = vuc * vuc;
  ASSERT (sc == 4);

  ss = vsc * vsc;
  ASSERT (ss == 0x3f01);
  ss = vuc * vsc;
  ASSERT (ss == 0x7e02);
  ss = vuc * vuc;
  ASSERT (ss == (short) 0xfc04);
#ifdef __SDCC
  /* after promotion the result of the multiplication is 'signed int', which overflows! */
  ASSERT(vuc * vuc < 1);
#endif

  /* mul ast: valMult() */
  ASSERT ((stype) -3 * (stype) -1 == (stype)  3);
  ASSERT ((stype) -3 * (stype)  1 == (stype) -3);
  ASSERT ((stype)  3 * (stype) -1 == (stype) -3);

  ASSERT ((stype)  1 * (utype) 0xfffffff7 == (utype) 0xfffffff7);

  ASSERT ((unsigned char ) 0xfffffff8 * (unsigned char ) 0xfffffff7 == 0xef48);
  ASSERT (mulWrapper ((unsigned short) 0xfffffff8, (unsigned short) 0xfffffff7) == (sizeof(int) == 2 ? 0x0048 : (unsigned int)0xffef0048));
  ASSERT ((unsigned LONG ) 0xfffffff8 * (unsigned LONG ) 0xfffffff7 == 0x0048);

  ASSERT ((stype         ) 0xfffffff8 * (stype         ) 0xfffffff7 == 72);

  ASSERT ((signed char ) -1 * (unsigned char ) 0xfffffff7 == (sizeof(int) == 2 ? 0xff09 : 0xffffff09));
  ASSERT ((signed short) -1 * (unsigned short) 0xfffffff7 == (sizeof(int) == 2 ?     9u : 0xffff0009));
  ASSERT ((signed LONG ) -1 * (unsigned LONG ) 0xfffffff7 == 9u);

  ASSERT ((signed char ) -2 * (unsigned char ) 0x8004 == (sizeof(int) == 2 ? 0xfff8 : 0xfffffff8));
  ASSERT ((signed short) -2 * (unsigned short) 0x8004 == (sizeof(int) == 2 ? 0xfff8 : 0xfffefff8));
  ASSERT ((signed LONG ) -2 * (unsigned LONG ) 0x8004 == 0xfffefff8);

  ASSERT (-1 * 0xfff7 == (sizeof(int) == 2 ? 9 : 0xffff0009)); // 0xfff7 is stored in 'unsigned int'
  // but:
  ASSERT (-1 * 65527  == -65527); // 65527 (== 0xfff7) is stored in 'signed LONG'
  ASSERT (-1 * 33000  == -33000);

  ASSERT (1 *  10000  * is8 == (sizeof(int) == 2 ? 14464  :  80000)); /* int      */
  ASSERT (1 *  10000l * is8 == 80000);                                /* LONG     */
  ASSERT (1 *  40000u * is8 == (sizeof(int) == 2 ? 57856u : 320000)); /* unsigned */
  ASSERT (1 *  40000  * is8 == 320000);                               /* LONG     */
  ASSERT (1 * 0x4000  * is8 == (sizeof(int) == 2 ? 0 : 0x20000));     /* unsigned */

  ASSERT (-2 * 1  < 1); /* comparison with 0 is optimized, so let's use 1 instead */
  ASSERT (-2 * 1u > 1);
  ASSERT (0x7fffu     * 2  > 1);
  ASSERT (0x7fffffffu * 2  > 1);
  if (sizeof (int) == 2)
    ASSERT (0x7fff * (unsigned char) 2 < 1);
  else
    ASSERT (mulWrapper (0x7fffffff, (unsigned char) 2) < 1);
  ASSERT (mulWrapper (0x7fffffff, (unsigned short) 2) < 1);

  /* mul icode: operandOperation() */
  s = -3;
  ASSERT (s * (stype) -1 == (stype)  3);
  ASSERT (s * (stype)  1 == (stype) -3);
  s =  3;
  ASSERT (s * (stype) -1 == (stype) -3);

  s = 1;
  ASSERT (s * (utype) 0xfffffff7 == (utype) 0xfffffff7);
  uc = (unsigned char ) 0xfffffff8;
  ASSERT (uc * (unsigned char ) 0xfffffff7 == 0xef48);
  us = (unsigned short) 0xfffffff8;
#if !(defined(PORT_HOST) && defined(__NetBSD__) && defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ == 1))
  /* this test fails on i386 and sparc64 NetBSD gcc 4.1 when compiled with -O2:
   * the result of us * (unsigned short) 0xfffffff7 is 0x7fffffff */
  ASSERT (us * (unsigned short) 0xfffffff7 == (sizeof(int) == 2 ? 0x0048 : 0xffef0048));
#endif
  ul = (unsigned LONG ) 0xfffffff8;
  ASSERT (ul * (unsigned LONG ) 0xfffffff7 == 0x0048);
  ul = (unsigned LONG ) 0xfffffff8;

  ASSERT ((stype         ) 0xfffffff8 * (stype         ) 0xfffffff7 == 72);

  ASSERT ((signed char ) -1 * (unsigned char ) 0xfffffff7 == (sizeof(int) == 2 ? 0xff09 : 0xffffff09));
  ASSERT ((signed short) -1 * (unsigned short) 0xfffffff7 == (sizeof(int) == 2 ?     9u : 0xffff0009));
  ASSERT ((signed LONG ) -1 * (unsigned LONG ) 0xfffffff7 == 9u);

  ASSERT ((signed char ) -2 * (unsigned char ) 0x8004 == (sizeof(int) == 2 ? 0xfff8 : 0xfffffff8));
  ASSERT ((signed short) -2 * (unsigned short) 0x8004 == (sizeof(int) == 2 ? 0xfff8 : 0xfffefff8));
  ASSERT ((signed LONG ) -2 * (unsigned LONG ) 0x8004 == 0xfffefff8);

  /* div ast: valDiv() */
  ASSERT ((stype) -12 / (stype) -3 == (stype)  4);
  ASSERT ((stype) -12 / (stype)  3 == (stype) -4);
  ASSERT ((stype)  12 / (stype) -3 == (stype) -4);

  ASSERT ((unsigned char ) -12 / (signed char ) -3 == (sizeof(int) == 2 ? 0xffaf : 0xffffffaf));
  ASSERT ((unsigned short) -12 / (signed short) -3 == (sizeof(int) == 2 ?      0 : 0xffffaaaf));
  ASSERT ((unsigned LONG ) -12 / (signed LONG ) -3 == 0);
  ASSERT ((utype)          -12 / (stype)         3 == (stype) 0x55555551);
  ASSERT ((unsigned char )  12 / (signed char ) -3 == -4);
  ASSERT ((unsigned short)  12 / (signed short) -3 == (sizeof(int) == 2 ?      0 : 0xfffffffc));
  ASSERT ((unsigned LONG )  12 / (signed LONG ) -3 == 0);

  ASSERT ((stype)        -12 / (utype)          -3 == 0);
  ASSERT ((signed char ) -12 / (unsigned char )  3 == -4);
  ASSERT ((signed short) -12 / (unsigned short)  3 == (sizeof(int) == 2 ? 0x5551 :  -4));
  ASSERT ((signed LONG ) -12 / (unsigned LONG )  3 == 0x55555551);
  ASSERT ((stype)         12 / (utype)          -3 == 0);

  ASSERT (12u / 3 * 10000 == 40000);

  ASSERT (-1 / 1 < 0);

  /* div icode: operandOperation() */
  s = -12;
  ASSERT (s / (stype) -3 == (stype)  4);
  s = -12;
  ASSERT (s / (stype)  3 == (stype) -4);
  s = 12;
  ASSERT (s / (stype) -3 == (stype) -4);

  uc = -12;
  ASSERT (uc / (signed char ) -3 == (sizeof(int) == 2 ? 0xffaf : 0xffffffaf));
  us = -12;
  ASSERT (us / (signed short) -3 == (sizeof(int) == 2 ?      0 : 0xffffaaaf));
  ul = -12;
  ASSERT (ul / (signed LONG ) -3 == 0);
  u  = -12;
  ASSERT (u  / (stype)         3 == (stype) 0x55555551);
  uc = 12;
  ASSERT (uc / (signed char ) -3 == -4);
  us = 12;
  ASSERT (us / (signed short) -3 == (sizeof(int) == 2 ?      0 : 0xfffffffc));
  ul = 12;
  ASSERT (ul / (signed LONG ) -3 == 0);

  s  = -12;
  ASSERT (s  / (utype)          -3 == 0);
  sc = -12;
  ASSERT (sc / (unsigned char )  3 == -4);
  ss = -12;
  ASSERT (ss / (unsigned short)  3 == (sizeof(int) == 2 ? 0x5551 :  -4));
  sl = -12;
  ASSERT (sl / (unsigned LONG )  3 == 0x55555551);
  s  = 12;
  ASSERT (s  / (utype)          -3 == 0);


  /* mod ast: valMod() */
  /* -11 : 0xfff5 */
  /* -17 : 0xffef */
  ASSERT ((stype) -17 % (stype) -11 == (stype) -6);
  ASSERT ((stype) -17 % (stype)  11 == (stype) -6);
  ASSERT ((stype)  17 % (stype) -11 == (stype)  6);
  ASSERT ((unsigned char ) -17 % (signed char ) -11 ==   8);
  ASSERT ((unsigned short) -17 % (signed short) -11 == (sizeof(int) == 2 ? -17 : 3));
  ASSERT ((unsigned LONG ) -17 % (signed LONG ) -11 == -17);
  ASSERT ((unsigned char ) -17 % (signed char )  11 ==   8);
  ASSERT ((unsigned short) -17 % (signed short)  11 ==   3);
  ASSERT ((unsigned LONG ) -17 % (signed LONG )  11 ==   9);
  ASSERT ((unsigned char )  17 % (signed char ) -11 ==   6);
  ASSERT ((unsigned short)  17 % (signed short) -11 == (sizeof(int) == 2 ? 17 : 6));
  ASSERT ((unsigned LONG )  17 % (signed LONG ) -11 ==  17);

  ASSERT (-3 % 2 < 0);


  /* add */
  ASSERT ( 80  +  80  == 160);
  ASSERT (150  + 150  == 300);
  ASSERT (160u + 160u == 320);
}

