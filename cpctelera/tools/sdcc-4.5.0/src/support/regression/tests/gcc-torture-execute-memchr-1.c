/*
   memchr-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <stddef.h>

#ifdef __SDCC
#pragma disable_warning 147
#endif

/* PR tree-optimization/86711 - wrong folding of memchr

   Verify that memchr() of arrays initialized with string literals
   where the nul doesn't fit in the array doesn't find the nul.  */

extern void* memchr (const void*, int, size_t);

static const char c = '1';
static const char s1[1] = "1";
static const char s4[4] = "1234";

static const char s4_2[2][4] = { "1234", "5678" };
static const char s5_3[3][5] = { "12345", "6789", "01234" };

volatile int v0 = 0;
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 3;

void test_narrow (void)
{

  int i0 = 0;
  int i1 = i0 + 1;
  int i2 = i1 + 1;
  int i3 = i2 + 1;
  int i4 = i3 + 1;

  ASSERT (memchr ("" + 1, 0, 0) == 0);

  ASSERT (memchr (&c, 0, sizeof c) == 0);
  ASSERT (memchr (&c + 1, 0, sizeof c - 1) == 0);
  ASSERT (memchr (&c + i1, 0, sizeof c - i1) == 0);
  ASSERT (memchr (&c + v1, 0, sizeof c - v1) == 0);

  ASSERT (memchr (s1, 0, sizeof s1) == 0);
  ASSERT (memchr (s1 + 1, 0, sizeof s1 - 1) == 0);
  ASSERT (memchr (s1 + i1, 0, sizeof s1 - i1) == 0);
  ASSERT (memchr (s1 + v1, 0, sizeof s1 - v1) == 0);

  ASSERT (memchr (&s1, 0, sizeof s1) == 0);
  ASSERT (memchr (&s1 + 1, 0, sizeof s1 - 1) == 0);
  ASSERT (memchr (&s1 + i1, 0, sizeof s1 - i1) == 0);
  ASSERT (memchr (&s1 + v1, 0, sizeof s1 - v1) == 0);
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  ASSERT (memchr (&s1[0], 0, sizeof s1) == 0);
  ASSERT (memchr (&s1[0] + 1, 0, sizeof s1 - 1) == 0);
  ASSERT (memchr (&s1[0] + i1, 0, sizeof s1 - i1) == 0);
  ASSERT (memchr (&s1[0] + v1, 0, sizeof s1 - v1) == 0);

  ASSERT (memchr (&s1[i0], 0, sizeof s1) == 0);
  ASSERT (memchr (&s1[i0] + 1, 0, sizeof s1 - 1) == 0);
  ASSERT (memchr (&s1[i0] + i1, 0, sizeof s1 - i1) == 0);
  ASSERT (memchr (&s1[i0] + v1, 0, sizeof s1 - v1) == 0);

  ASSERT (memchr (&s1[v0], 0, sizeof s1) == 0);
  ASSERT (memchr (&s1[v0] + 1, 0, sizeof s1 - 1) == 0);
  ASSERT (memchr (&s1[v0] + i1, 0, sizeof s1 - i1) == 0);
  ASSERT (memchr (&s1[v0] + v1, 0, sizeof s1 - v1) == 0);


  ASSERT (memchr (s4 + i0, 0, sizeof s4 - i0) == 0);
  ASSERT (memchr (s4 + i1, 0, sizeof s4 - i1) == 0);
  ASSERT (memchr (s4 + i2, 0, sizeof s4 - i2) == 0);
  ASSERT (memchr (s4 + i3, 0, sizeof s4 - i3) == 0);
  ASSERT (memchr (s4 + i4, 0, sizeof s4 - i4) == 0);

  ASSERT (memchr (s4 + v0, 0, sizeof s4 - v0) == 0);
  ASSERT (memchr (s4 + v1, 0, sizeof s4 - v1) == 0);
  ASSERT (memchr (s4 + v2, 0, sizeof s4 - v2) == 0);
  ASSERT (memchr (s4 + v3, 0, sizeof s4 - v3) == 0);
  ASSERT (memchr (s4 + v4, 0, sizeof s4 - v4) == 0);

  ASSERT (memchr (s4_2, 0, sizeof s4_2) == 0);

  ASSERT (memchr (s4_2[0], 0, sizeof s4_2[0]) == 0);
  ASSERT (memchr (s4_2[1], 0, sizeof s4_2[1]) == 0);

  ASSERT (memchr (s4_2[0] + 1, 0, sizeof s4_2[0] - 1) == 0);
  ASSERT (memchr (s4_2[1] + 2, 0, sizeof s4_2[1] - 2) == 0);
  ASSERT (memchr (s4_2[1] + 3, 0, sizeof s4_2[1] - 3) == 0);

  ASSERT (memchr (s4_2[v0], 0, sizeof s4_2[v0]) == 0);
  ASSERT (memchr (s4_2[v0] + 1, 0, sizeof s4_2[v0] - 1) == 0);


  /* The following calls must find the nul.  */
  ASSERT (memchr ("", 0, 1) != 0);
  ASSERT (memchr (s5_3, 0, sizeof s5_3) == &s5_3[1][4]);

  ASSERT (memchr (&s5_3[0][0] + i0, 0, sizeof s5_3 - i0) == &s5_3[1][4]);
  ASSERT (memchr (&s5_3[0][0] + i1, 0, sizeof s5_3 - i1) == &s5_3[1][4]);
  ASSERT (memchr (&s5_3[0][0] + i2, 0, sizeof s5_3 - i2) == &s5_3[1][4]);
  ASSERT (memchr (&s5_3[0][0] + i4, 0, sizeof s5_3 - i4) == &s5_3[1][4]);

  ASSERT (memchr (&s5_3[1][i0], 0, sizeof s5_3[1] - i0) == &s5_3[1][4]);
#endif
}

void test_wide (void) { }

void
testTortureExecute (void)
{
  test_narrow ();
  test_wide ();
}
