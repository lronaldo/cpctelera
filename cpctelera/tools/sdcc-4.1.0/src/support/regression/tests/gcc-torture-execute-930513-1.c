/*
   930513-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

/* { dg-additional-options "-Wl,-u,_printf_float" { target newlib_nano_io } } */

#include <stdio.h>
char buf[2];

#if 0 // TODO: enable when SDCC support K&R-style
f (fp)
     int (*fp)(char *, const char *, ...);
{
  (*fp)(buf, "%.0f", 5.0);
}
#endif

void
testTortureExecute (void)
{
#if 0
  f (&sprintf);
  if (buf[0] != '5' || buf[1] != 0)
    ASSERT (0);
  return;
#endif
}

