/*
   loop-15.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Bombed with a segfault on powerpc-linux.  doloop.c generated wrong
   loop count.  */
void
foo (unsigned long *start, unsigned long *end)
{
  unsigned long *temp = end - 1;

  while (end > start)
    *end-- = *temp--;
}

void
testTortureExecute (void)
{
#ifndef __SDCC_pdk14 // Lack of memory
  unsigned long a[5];
  int start, end, k;

  for (start = 0; start < 5; start++)
    for (end = 0; end < 5; end++)
      {
	for (k = 0; k < 5; k++)
	  a[k] = k;

	foo (a + start, a + end);

	for (k = 0; k <= start; k++)
	  if (a[k] != k)
	    ASSERT (0);

	for (k = start + 1; k <= end; k++)
	  if (a[k] != k - 1)
	    ASSERT (0);

	for (k = end + 1; k < 5; k++)
	  if (a[k] != k)
	    ASSERT (0);
      }
#endif
  return;
}
