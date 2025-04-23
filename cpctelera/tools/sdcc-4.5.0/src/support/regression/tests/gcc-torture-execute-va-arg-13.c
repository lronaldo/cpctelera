/*
va-arg-13.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* derived from mozilla source code */

#include <stdarg.h>

typedef struct {
  void *stream;
  va_list ap;
  int nChar;  
} ScanfState;

void dummy (va_list vap)
{
  if (va_arg (vap, int) != 1234) ASSERT(0);
  return;
}

void test (int fmt, ...)
{
  ScanfState state, *statep;

  statep = &state;

  va_start (statep->ap, fmt);
  dummy (statep->ap);
  va_end (statep->ap);
  
  va_start (state.ap, fmt);
  dummy (state.ap);
  va_end (state.ap);
  
  return;
}

void
testTortureExecute (void)
{
#ifndef __SDCC_ds390 // Bug #2779
  test (456, 1234);
  return;
#endif
}
