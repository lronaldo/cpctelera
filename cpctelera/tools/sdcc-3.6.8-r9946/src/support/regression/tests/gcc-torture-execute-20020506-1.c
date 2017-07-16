/*
   20020506-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
/* Copyright (C) 2002  Free Software Foundation.

   Test that (A & C1) op C2 optimizations behave correctly where C1 is
   a constant power of 2, op is == or !=, and C2 is C1 or zero.

   Written by Roger Sayle, 5th May 2002.  */

#include <limits.h>

void fest1 (signed char c, int set);
void fest2 (unsigned char c, int set);
void fest3 (short s, int set);
void fest4 (unsigned short s, int set);
void fest5 (int i, int set);
void fest6 (unsigned int i, int set);
void fest7 (long long l, int set);
void fest8 (unsigned long long l, int set);

#ifndef LONG_LONG_MAX
#define LONG_LONG_MAX LLONG_MAX
#endif
#ifndef LONG_LONG_MIN
#define LONG_LONG_MIN (-LONG_LONG_MAX-1)
#endif
#ifndef ULONG_LONG_MAX
#define ULONG_LONG_MAX (LONG_LONG_MAX * 2ULL + 1)
#endif


void
fest1 (signed char c, int set)
{
  if ((c & (SCHAR_MAX+1)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((c & (SCHAR_MAX+1)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((unsigned char) (c & (SCHAR_MAX+1)) == (SCHAR_MAX+1))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((unsigned char) (c & (SCHAR_MAX+1)) != (SCHAR_MAX+1))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest2 (unsigned char c, int set)
{
  if ((c & (SCHAR_MAX+1)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((c & (SCHAR_MAX+1)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((c & (SCHAR_MAX+1)) == (SCHAR_MAX+1))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((c & (SCHAR_MAX+1)) != (SCHAR_MAX+1))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest3 (short s, int set)
{
  if ((s & (SHRT_MAX+1)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) == (SHRT_MAX+1))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) != (SHRT_MAX+1))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest4 (unsigned short s, int set)
{
  if ((s & (SHRT_MAX+1)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) == (SHRT_MAX+1))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((s & (SHRT_MAX+1)) != (SHRT_MAX+1))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest5 (int i, int set)
{
  if ((i & (INT_MAX+1U)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((i & (INT_MAX+1U)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((i & (INT_MAX+1U)) == (INT_MAX+1U))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((i & (INT_MAX+1U)) != (INT_MAX+1U))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest6 (unsigned int i, int set)
{
  if ((i & (INT_MAX+1U)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((i & (INT_MAX+1U)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((i & (INT_MAX+1U)) == (INT_MAX+1U))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((i & (INT_MAX+1U)) != (INT_MAX+1U))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}

void
fest7 (long long l, int set)
{
  if ((l & (LONG_LONG_MAX+1ULL)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) == (LONG_LONG_MAX+1ULL))
    {
      ASSERT (set);
    }
  else
    if (set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) != (LONG_LONG_MAX+1ULL))
    {
      if (set) ASSERT (0);
    }
  else
    ASSERT (set);
}

void
fest8 (unsigned long long l, int set)
{
  if ((l & (LONG_LONG_MAX+1ULL)) == 0)
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) != 0)
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) == (LONG_LONG_MAX+1ULL))
    {
      if (!set) ASSERT (0);
    }
  else
    if (set) ASSERT (0);

  if ((l & (LONG_LONG_MAX+1ULL)) != (LONG_LONG_MAX+1ULL))
    {
      if (set) ASSERT (0);
    }
  else
    if (!set) ASSERT (0);
}
#endif

void
testTortureExecute (void)
{
#if !defined(__SDCC_ds390) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16)
  fest1 (0, 0);
  fest1 (SCHAR_MAX, 0);
  fest1 (SCHAR_MIN, 1);
  fest1 (UCHAR_MAX, 1);

  fest2 (0, 0);
  fest2 (SCHAR_MAX, 0);
  fest2 (SCHAR_MIN, 1);
  fest2 (UCHAR_MAX, 1);

  fest3 (0, 0);
  fest3 (SHRT_MAX, 0);
  fest3 (SHRT_MIN, 1);
  fest3 (USHRT_MAX, 1);

  fest4 (0, 0);
  fest4 (SHRT_MAX, 0);
  fest4 (SHRT_MIN, 1);
  fest4 (USHRT_MAX, 1);

  fest5 (0, 0);
  fest5 (INT_MAX, 0);
  fest5 (INT_MIN, 1);
  fest5 (UINT_MAX, 1);

  fest6 (0, 0);
  fest6 (INT_MAX, 0);
  fest6 (INT_MIN, 1);
  fest6 (UINT_MAX, 1);

#if !defined(__SDCC_hc08) && !defined(__SDCC_s08)
  fest7 (0, 0);
  fest7 (LONG_LONG_MAX, 0);
  fest7 (LONG_LONG_MIN, 1);
  fest7 (ULONG_LONG_MAX, 1);

  fest8 (0, 0);
  fest8 (LONG_LONG_MAX, 0);
  fest8 (LONG_LONG_MIN, 1);
  fest8 (ULONG_LONG_MAX, 1);

  return;
#endif
#endif
}

