/*
wchar_t-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <wchar.h>

/* { dg-options "-finput-charset=utf-8" } */

wchar_t x[] = L"Ä";
wchar_t y = L'Ä';

void
testTortureExecute (void)
{
  if (sizeof (x) / sizeof (wchar_t) != 2)
    ASSERT (0);
  if (x[0] != L'Ä' || x[1] != L'\0')
    ASSERT (0);
  if (y != L'Ä')
    ASSERT (0);
  return;
}
