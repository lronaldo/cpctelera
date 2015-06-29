/*
   string-opt-5.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 2000  Free Software Foundation.

   Ensure builtin strlen, strcmp, strchr, strrchr and strncpy
   perform correctly.

   Written by Jakub Jelinek, 11/7/2000.  */

#include <string.h>

int x = 6;
int y = 1;
const char *bar = "hi world";
char buf [24];

void
testTortureExecute (void)
{
#ifndef __SDCC_pic16
  const char *const foo = "hello world";
  char dst [64];

  if (strlen (bar) != 8)
    ASSERT (0);
  if (strlen (bar + (++x & 2)) != 6)
    ASSERT (0);
  if (x != 7)
    ASSERT (0);;
  if (strlen (foo + (x++, 6)) != 5)
    ASSERT (0);
  if (x != 8)
    ASSERT (0);
  if (strlen (foo + (++x & 1)) != 10)
    ASSERT (0);
  if (x != 9)
    ASSERT (0);
  if (strcmp (foo + (x -= 6), "lo world"))
    ASSERT (0);
  if (x != 3)
    ASSERT (0);
  if (strcmp (foo, bar) >= 0)
    ASSERT (0);
  if (strcmp (foo, bar + (x++ & 1)) >= 0)
    ASSERT (0);
  if (x != 4)
    ASSERT (0);
  if (strchr (foo + (x++ & 7), 'l') != foo + 9)
    ASSERT (0);
  if (x != 5)
    ASSERT (0);
  if (strchr (bar, 'o') != bar + 4)
    ASSERT (0);
  if (strchr (bar, '\0')  != bar + 8)
    ASSERT (0);
  if (strrchr (bar, 'x'))
    ASSERT (0);
  if (strrchr (bar, 'o') != bar + 4)
    ASSERT (0);
  if (strcmp (foo + (x++ & 1), "ello world" + (--y & 1)))
    ASSERT (0);
  if (x != 6 || y != 0)
    ASSERT (0);
  dst[5] = ' ';
  dst[6] = '\0';
  x = 5;
  y = 1;
  if (strncpy (dst + 1, foo + (x++ & 3), 4) != dst + 1
      || x != 6
      || strcmp (dst + 1, "ello "))
    ASSERT (0);
  memset (dst, ' ', sizeof dst);
  if (strncpy (dst + (++x & 1), (y++ & 3) + "foo", 10) != dst + 1
      || x != 7
      || y != 2
      || memcmp (dst, " oo\0\0\0\0\0\0\0\0 ", 12))
    ASSERT (0);
  memset (dst, ' ', sizeof dst);
  if (strncpy (dst, "hello", 8) != dst || memcmp (dst, "hello\0\0\0 ", 9))
    ASSERT (0);
  x = '!';
  memset (buf, ' ', sizeof buf);
  if (memset (buf, x++, ++y) != buf
      || x != '!' + 1
      || y != 3
      || memcmp (buf, "!!!", 3))
    ASSERT (0);
  if (memset (buf + y++, '-', 8) != buf + 3
      || y != 4
      || memcmp (buf, "!!!--------", 11))
    ASSERT (0);
  x = 10;
  if (memset (buf + ++x, 0, y++) != buf + 11
      || x != 11
      || y != 5
      || memcmp (buf + 8, "---\0\0\0", 7))
    ASSERT (0);

  if (memset (buf + (x += 4), 0, 6) != buf + 15
      || x != 15
      || memcmp (buf + 10, "-\0\0\0\0\0\0\0\0\0", 11))
    ASSERT (0);

  return;
#endif
}
