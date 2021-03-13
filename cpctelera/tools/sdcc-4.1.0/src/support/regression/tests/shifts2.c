/** extended tests for shift operations, added by
    Frieder Ferlemann to check for bugs related to bug 1533966

    size: 8,16,32
    right: 0,1
    vol: 0,1
    sign: u,
    goal: 0, 1, 2
*/
#include <testfwk.h>
#ifdef __sun__
#  include <inttypes.h>
#else
#  include <stdint.h>
#endif

#define SIZE    ({size})
#define RIGHT   ({right})
#define VOL     ({vol})
#define GOAL    ({goal})

#if SIZE == 8
#  define TYPE {sign}int8_t
#  define TEST_VECT 0xa4
#endif

#if SIZE == 16
#  define TYPE {sign}int16_t
#  define TEST_VECT 0xa8ce
#endif

#if SIZE == 32
#  define TYPE {sign}int32_t
#  define TEST_VECT 0xa8b5c4d6
#endif

#if RIGHT == 0
#  define SHIFT(x,y) (TYPE)((TYPE)(x)<<(y))
#else
#  define SHIFT(x,y) (TYPE)((TYPE)(x)>>(y))
#endif

#if GOAL == 1
#pragma opt_code_size
#elif GOAL == 2
#pragma opt_code_speed
#endif

#if VOL == 0
  volatile TYPE s = TEST_VECT;
           TYPE t;
  #define TESTSHIFT(x) t=s; t = SHIFT (t,(x)); ASSERT( t == SHIFT (TEST_VECT, (x)));
#else
  volatile TYPE t = TEST_VECT;
  #define TESTSHIFT(x) t=TEST_VECT; t = SHIFT (t,(x)); ASSERT( t == SHIFT (TEST_VECT, (x)));
#endif

static void
testShift(void)
{
#ifndef __SDCC_pdk14 // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
    TESTSHIFT(0);
    TESTSHIFT(1);
    TESTSHIFT(2);
    TESTSHIFT(3);
    TESTSHIFT(4);
    TESTSHIFT(5);
    TESTSHIFT(6);
    TESTSHIFT(7);

#if SIZE >= 16
    TESTSHIFT(8);
    TESTSHIFT(9);
    TESTSHIFT(10);
    TESTSHIFT(11);
    TESTSHIFT(12);
    TESTSHIFT(13);
    TESTSHIFT(14);
    TESTSHIFT(15);
#endif

#if SIZE >= 32
    TESTSHIFT(16);
    TESTSHIFT(17);
    TESTSHIFT(18);
    TESTSHIFT(19);
    TESTSHIFT(20);
    TESTSHIFT(21);
    TESTSHIFT(22);
    TESTSHIFT(23);

    TESTSHIFT(24);
    TESTSHIFT(25);
    TESTSHIFT(26);
    TESTSHIFT(27);
    TESTSHIFT(28);
    TESTSHIFT(29);
    TESTSHIFT(30);
    TESTSHIFT(31);
#endif
#endif
#endif
}
