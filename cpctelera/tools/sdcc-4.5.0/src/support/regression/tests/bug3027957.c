/*
   bug3027957.c
 */

#include <testfwk.h>

#ifdef __SDCC_stm8
#define ADDRESS 0x1000
#elif defined(__SDCC_f8)
#define ADDRESS 0x3000
#elif defined(__SDCC_pic14)
#define ADDRESS 0x01A0
#else
#define ADDRESS 0xF000
#endif

#if defined(__SDCC_pic14) // Bank size limit
#  define OFFSET_DECIMAL 70
#  define OFFSET_HEX     0x46
#else
#  define OFFSET_DECIMAL 100
#  define OFFSET_HEX     0x64
#endif

void foo(void)
{
  ((unsigned char __xdata *)ADDRESS)[OFFSET_DECIMAL] = 0x12;
}

/* bug 3034400: this  should not give a warning/error */
char * correct(void)
{
  return (char __code *) 0x1234;
}

void testBug(void)
{
#if !defined (__SDCC_pdk14) && !defined (__SDCC_pdk15) // I have no idea yet, how integers cast to pointers should behave here on pdk14
#ifdef __SDCC
  foo();
  ASSERT (*(unsigned char __xdata *)(ADDRESS + OFFSET_HEX) == 0x12);
#endif
#endif
}

