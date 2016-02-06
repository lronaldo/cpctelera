/*
   bug3027957.c
 */

#include <testfwk.h>

void foo(void)
{
  ((unsigned char __xdata *)0xF000)[100] = 0x12;
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
  ASSERT (*(unsigned char __xdata *)(0xF064) == 0x12);
#endif
}
