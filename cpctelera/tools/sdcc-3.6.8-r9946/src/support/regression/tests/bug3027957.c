/*
   bug3027957.c
 */

#include <testfwk.h>

#ifdef __SDCC_stm8
#define ADDRESS 0x1000
#else
#define ADDRESS 0xF000
#endif

void foo(void)
{
  ((unsigned char __xdata *)ADDRESS)[100] = 0x12;
}

/* bug 3034400: this  should not give a warning/error */
char * correct(void)
{
  return (char __code *) 0x1234;
}

void testBug(void)
{
#ifdef __SDCC
  foo();
  ASSERT (*(unsigned char __xdata *)(ADDRESS + 0x64) == 0x12);
#endif
}

