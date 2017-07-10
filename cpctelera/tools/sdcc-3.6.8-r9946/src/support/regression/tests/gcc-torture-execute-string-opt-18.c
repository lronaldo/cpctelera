/*
   string-opt-18.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2003  Free Software Foundation.

   Test equal pointer optimizations don't break anything.

   Written by Roger Sayle, July 14, 2003.  */

#include <string.h>


void test1 (void *ptr)
{
  if (memcpy(ptr,ptr,8) != ptr)
    ASSERT (0);
}

#if 0
// GNU-specific function
void test2 (char *ptr)
{
  if (mempcpy(ptr,ptr,8) != ptr+8)
    ASSERT (0);
}
#endif

void test3 (void *ptr)
{
  if (memmove(ptr,ptr,8) != ptr)
    ASSERT (0);
}

void test4 (char *ptr)
{
  if (strcpy(ptr,ptr) != ptr)
    ASSERT (0);
}

void test5 (void *ptr)
{
  if (memcmp(ptr,ptr,8) != 0)
    ASSERT (0);
}

void test6 (const char *ptr)
{
  if (strcmp(ptr,ptr) != 0)
    ASSERT (0);
}

void test7 (const char *ptr)
{
  if (strncmp(ptr,ptr,8) != 0)
    ASSERT (0);
}


void
testTortureExecute (void)
{
  char buf[10];

  test1 (buf);
#if 0
  test2 (buf);
#endif
  test3 (buf);
#if !defined (__APPLE__)
  test4 (buf);
#endif
  test5 (buf);
  test6 (buf);
  test7 (buf);

  return;
}

