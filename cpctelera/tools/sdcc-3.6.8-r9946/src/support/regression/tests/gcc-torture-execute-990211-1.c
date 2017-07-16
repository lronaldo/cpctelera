/*
   990211-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* Copyright (C) 1999 Free Software Foundation, Inc.
  Contributed by Nathan Sidwell 20 Jan 1999 <nathan@acm.org> */

/* check range combining boolean operations work */

#define N 77

void func(int i)
{
  /* fold-const does some clever things with range tests. Make sure
     we get (some of) them right */
  
  /* these must fail, regardless of the value of i */
  if ((i < 0) && (i >= 0))
    ASSERT(0);
  if ((i > 0) && (i <= 0))
    ASSERT(0);
  if ((i >= 0) && (i < 0))
    ASSERT(0);
  if ((i <= 0) && (i > 0))
    ASSERT(0);

  if ((i < N) && (i >= N))
    ASSERT(0);
  if ((i > N) && (i <= N))
    ASSERT(0);
  if ((i >= N) && (i < N))
    ASSERT(0);
  if ((i <= N) && (i > N))
    ASSERT(0);
    
  /* these must pass, regardless of the value of i */
  if (! ((i < 0) || (i >= 0)))
    ASSERT(0);
  if (! ((i > 0) || (i <= 0)))
    ASSERT(0);
  if (! ((i >= 0) || (i < 0)))
    ASSERT(0);
  if (! ((i <= 0) || (i > 0)))
    ASSERT(0);

  if (! ((i < N) || (i >= N)))
    ASSERT(0);
  if (! ((i > N) || (i <= N)))
    ASSERT(0);
  if (! ((i >= N) || (i < N)))
    ASSERT(0);
  if (! ((i <= N) || (i > N)))
    ASSERT(0);
  
  return;
}

void
testTortureExecute (void)
{
  func(0);
  func(1);
  return;
}

