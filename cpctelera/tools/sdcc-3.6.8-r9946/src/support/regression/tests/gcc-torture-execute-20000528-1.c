/*
   20000528-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2000  Free Software Foundation  */
/* Contributed by Alexandre Oliva <aoliva@cygnus.com> */

unsigned long l = (unsigned long)-2;
unsigned short s;

void
testTortureExecute (void) {
  long t = l;
  s = t;
  if (s != (unsigned short)-2)
    ASSERT (0);
  return;
}

