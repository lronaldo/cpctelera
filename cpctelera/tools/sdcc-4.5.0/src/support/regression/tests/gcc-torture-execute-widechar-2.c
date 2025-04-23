/*
widechar-2.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <stddef.h>

const wchar_t ws[] = L"foo";

void
testTortureExecute (void)
{
  if (ws[0] != L'f' || ws[1] != L'o' || ws[2] != L'o' || ws[3] != L'\0')
    ASSERT(0);

  return;
}
