/*
strcpy-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* Copyright (C) 2002  Free Software Foundation.

   Test strcpy with various combinations of pointer alignments and lengths to
   make sure any optimizations in the library are correct.  */

#include <string.h>

#ifndef MAX_OFFSET
#define MAX_OFFSET (sizeof (long long))
#endif

#ifndef MAX_COPY
#define MAX_COPY (3 * sizeof (long long)) /* Was (10 * sizeof (long long)) in GCC, reduced to speed up regression testing */
#endif

#ifndef MAX_EXTRA
#define MAX_EXTRA (sizeof (long long))
#endif

#define MAX_LENGTH (MAX_OFFSET + MAX_COPY + 1 + MAX_EXTRA)

/* Use a sequence length that is not divisible by two, to make it more
   likely to detect when words are mixed up.  */
#define SEQUENCE_LENGTH 31

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
static union {
  char buf[MAX_LENGTH];
  long long align_int;
#if 0 // TODO: Emable when SDCC suports long double!
  long double align_fp;
#endif
} u1, u2;
#endif

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
  int off1, off2, len, i;
  char *p, *q, c;

  for (off1 = 0; off1 < MAX_OFFSET; off1++)
    for (off2 = 0; off2 < MAX_OFFSET; off2++)
      for (len = 1; len < MAX_COPY; len++)
	{
	  for (i = 0, c = 'A'; i < MAX_LENGTH; i++, c++)
	    {
	      u1.buf[i] = 'a';
	      if (c >= 'A' + SEQUENCE_LENGTH)
		c = 'A';
	      u2.buf[i] = c;
	    }
	  u2.buf[off2 + len] = '\0';

	  p = strcpy (u1.buf + off1, u2.buf + off2);
	  if (p != u1.buf + off1)
	    ASSERT (0);

	  q = u1.buf;
	  for (i = 0; i < off1; i++, q++)
	    if (*q != 'a')
	      ASSERT (0);

	  for (i = 0, c = 'A' + off2; i < len; i++, q++, c++)
	    {
	      if (c >= 'A' + SEQUENCE_LENGTH)
		c = 'A';
	      if (*q != c)
		ASSERT (0);
	    }

	  if (*q++ != '\0')
	    ASSERT (0);
	  for (i = 0; i < MAX_EXTRA; i++, q++)
	    if (*q != 'a')
	      ASSERT (0);
	}
#endif
  return;
}
