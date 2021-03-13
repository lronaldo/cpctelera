/*
memset-3.c from the execute part of the gcc torture tests.
*/

#include <testfwk.h>

#include <string.h>

/* Copyright (C) 2002  Free Software Foundation.

   Test memset with various combinations of constant pointer alignments and
   lengths to make sure any optimizations in the compiler are correct.

   Written by Roger Sayle, July 22, 2002.  */

#ifndef MAX_OFFSET
#define MAX_OFFSET (sizeof (long long))
#endif

#ifndef MAX_COPY
#define MAX_COPY 15
#endif

#ifndef MAX_EXTRA
#define MAX_EXTRA (sizeof (long long))
#endif

#define MAX_LENGTH (MAX_OFFSET + MAX_COPY + MAX_EXTRA)

#if 0 // TODO: Enable when SDC supports long double!
static union {
  char buf[MAX_LENGTH];
  long long align_int;
  long double align_fp;
} u;

char A = 'A';

void reset ()
{
  int i;

  for (i = 0; i < MAX_LENGTH; i++)
    u.buf[i] = 'a';
}

void check (int off, int len, int ch)
{
  char *q;
  int i;

  q = u.buf;
  for (i = 0; i < off; i++, q++)
    if (*q != 'a')
      ASSERT (0);

  for (i = 0; i < len; i++, q++)
    if (*q != ch)
      ASSERT (0);

  for (i = 0; i < MAX_EXTRA; i++, q++)
    if (*q != 'a')
      ASSERT (0);
}
#endif
void
testTortureExecute (void)
{
#if 0
  int len;
  char *p;

  /* off == 0 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf, '\0', len);
      if (p != u.buf) ASSERT (0);
      check (0, len, '\0');

      p = memset (u.buf, A, len);
      if (p != u.buf) ASSERT (0);
      check (0, len, 'A');

      p = memset (u.buf, 'B', len);
      if (p != u.buf) ASSERT (0);
      check (0, len, 'B');
    }

  /* off == 1 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+1, '\0', len);
      if (p != u.buf+1) ASSERT (0);
      check (1, len, '\0');

      p = memset (u.buf+1, A, len);
      if (p != u.buf+1) ASSERT (0);
      check (1, len, 'A');

      p = memset (u.buf+1, 'B', len);
      if (p != u.buf+1) ASSERT (0);
      check (1, len, 'B');
    }

  /* off == 2 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+2, '\0', len);
      if (p != u.buf+2) ASSERT (0);
      check (2, len, '\0');

      p = memset (u.buf+2, A, len);
      if (p != u.buf+2) ASSERT (0);
      check (2, len, 'A');

      p = memset (u.buf+2, 'B', len);
      if (p != u.buf+2) ASSERT (0);
      check (2, len, 'B');
    }

  /* off == 3 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+3, '\0', len);
      if (p != u.buf+3) ASSERT (0);
      check (3, len, '\0');

      p = memset (u.buf+3, A, len);
      if (p != u.buf+3) ASSERT (0);
      check (3, len, 'A');

      p = memset (u.buf+3, 'B', len);
      if (p != u.buf+3) ASSERT (0);
      check (3, len, 'B');
    }

  /* off == 4 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+4, '\0', len);
      if (p != u.buf+4) ASSERT (0);
      check (4, len, '\0');

      p = memset (u.buf+4, A, len);
      if (p != u.buf+4) ASSERT (0);
      check (4, len, 'A');

      p = memset (u.buf+4, 'B', len);
      if (p != u.buf+4) ASSERT (0);
      check (4, len, 'B');
    }

  /* off == 5 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+5, '\0', len);
      if (p != u.buf+5) ASSERT (0);
      check (5, len, '\0');

      p = memset (u.buf+5, A, len);
      if (p != u.buf+5) ASSERT (0);
      check (5, len, 'A');

      p = memset (u.buf+5, 'B', len);
      if (p != u.buf+5) ASSERT (0);
      check (5, len, 'B');
    }

  /* off == 6 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+6, '\0', len);
      if (p != u.buf+6) ASSERT (0);
      check (6, len, '\0');

      p = memset (u.buf+6, A, len);
      if (p != u.buf+6) ASSERT (0);
      check (6, len, 'A');

      p = memset (u.buf+6, 'B', len);
      if (p != u.buf+6) ASSERT (0);
      check (6, len, 'B');
    }

  /* off == 7 */
  for (len = 0; len < MAX_COPY; len++)
    {
      reset ();

      p = memset (u.buf+7, '\0', len);
      if (p != u.buf+7) ASSERT (0);
      check (7, len, '\0');

      p = memset (u.buf+7, A, len);
      if (p != u.buf+7) ASSERT (0);
      check (7, len, 'A');

      p = memset (u.buf+7, 'B', len);
      if (p != u.buf+7) ASSERT (0);
      check (7, len, 'B');
    }
#endif
  return;
}

