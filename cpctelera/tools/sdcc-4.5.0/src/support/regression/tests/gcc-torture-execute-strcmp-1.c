/*
strcmp-1.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

/* Copyright (C) 2002  Free Software Foundation.

   Test strcmp with various combinations of pointer alignments and lengths to
   make sure any optimizations in the library are correct.

   Written by Michael Meissner, March 9, 2002.  */

#include <string.h>
#include <stddef.h>

#ifndef MAX_OFFSET
#define MAX_OFFSET (sizeof (long long))
#endif

#ifndef MAX_TEST
#define MAX_TEST (2 * sizeof (long long)) /* Was (8 * sizeof (long long)) in GCC, reduced to speed up regression testing */
#endif

#ifndef MAX_EXTRA
#define MAX_EXTRA (sizeof (long long))
#endif

#define MAX_LENGTH (MAX_OFFSET + MAX_TEST + MAX_EXTRA + 2)

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
static union {
  unsigned char buf[MAX_LENGTH];
  long long align_int;
#if 0 // TODO: Enable when SDCC suports long double!
  long double align_fp;
#endif
} u1, u2;
#endif

void
test (const unsigned char *s1, const unsigned char *s2, int expected)
{
  int value = strcmp ((char *) s1, (char *) s2);

  if (expected < 0 && value >= 0)
    ASSERT (0);
  else if (expected == 0 && value != 0)
    ASSERT (0);
  else if (expected > 0 && value <= 0)
    ASSERT (0);
}

void
testTortureExecute (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) // Lack of memory
  size_t off1, off2, len, i;
  unsigned char *buf1, *buf2;
  unsigned char *mod1, *mod2;
  unsigned char *p1, *p2;

  for (off1 = 0; off1 < MAX_OFFSET; off1++)
    for (off2 = 0; off2 < MAX_OFFSET; off2++)
      for (len = 0; len < MAX_TEST; len++)
	{
	  p1 = u1.buf;
	  for (i = 0; i < off1; i++)
	    *p1++ = '\0';

	  buf1 = p1;
	  for (i = 0; i < len; i++)
	    *p1++ = 'a';

	  mod1 = p1;
	  for (i = 0; i < MAX_EXTRA+2; i++)
	    *p1++ = 'x';

	  p2 = u2.buf;
	  for (i = 0; i < off2; i++)
	    *p2++ = '\0';

	  buf2 = p2;
	  for (i = 0; i < len; i++)
	    *p2++ = 'a';

	  mod2 = p2;
	  for (i = 0; i < MAX_EXTRA+2; i++)
	    *p2++ = 'x';

	  mod1[0] = '\0';
	  mod2[0] = '\0';
	  test (buf1, buf2, 0);

	  mod1[0] = 'a';
	  mod1[1] = '\0';
	  mod2[0] = '\0';
	  test (buf1, buf2, +1);

	  mod1[0] = '\0';
	  mod2[0] = 'a';
	  mod2[1] = '\0';
	  test (buf1, buf2, -1);

	  mod1[0] = 'b';
	  mod1[1] = '\0';
	  mod2[0] = 'c';
	  mod2[1] = '\0';
	  test (buf1, buf2, -1);

	  mod1[0] = 'c';
	  mod1[1] = '\0';
	  mod2[0] = 'b';
	  mod2[1] = '\0';
	  test (buf1, buf2, +1);

	  mod1[0] = 'b';
	  mod1[1] = '\0';
	  mod2[0] = (unsigned char)'\251';
	  mod2[1] = '\0';
	  test (buf1, buf2, -1);

	  mod1[0] = (unsigned char)'\251';
	  mod1[1] = '\0';
	  mod2[0] = 'b';
	  mod2[1] = '\0';
	  test (buf1, buf2, +1);

	  mod1[0] = (unsigned char)'\251';
	  mod1[1] = '\0';
	  mod2[0] = (unsigned char)'\252';
	  mod2[1] = '\0';
	  test (buf1, buf2, -1);

	  mod1[0] = (unsigned char)'\252';
	  mod1[1] = '\0';
	  mod2[0] = (unsigned char)'\251';
	  mod2[1] = '\0';
	  test (buf1, buf2, +1);
	}
#endif
  return;
}
