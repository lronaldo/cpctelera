/*
   strlen-2.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#include <stddef.h>

/* PR tree-optimization/86532 - Wrong code due to ASSERT wrong strlen folding  */

extern size_t strlen (const char*);

static const char a[2][3] = { "1", "12" };
static const char b[2][2][5] = { { "1", "12" }, { "123", "1234" } };

volatile int v0 = 0;
volatile int v1 = 1;
volatile int v2 = 2;

void test_array_ref_2_3 (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !(defined(__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ASSERT (strlen (a[v0]) == 1);
  ASSERT (strlen (&a[v0][v0]) == 1);
  ASSERT (strlen (&a[0][v0]) == 1);
  ASSERT (strlen (&a[v0][0]) == 1);

  ASSERT (strlen (a[v1]) == 2);
  ASSERT (strlen (&a[v1][0]) == 2);
  ASSERT (strlen (&a[1][v0]) == 2);
  ASSERT (strlen (&a[v1][v0]) == 2);

  ASSERT (strlen (&a[v1][1]) == 1);
  ASSERT (strlen (&a[v1][1]) == 1);

  ASSERT (strlen (&a[v1][2]) == 0);
  ASSERT (strlen (&a[v1][v2]) == 0);

  int i0 = 0;
  int i1 = i0 + 1;
  int i2 = i1 + 1;

  ASSERT (strlen (a[v0]) == 1);
  ASSERT (strlen (&a[v0][v0]) == 1);
  ASSERT (strlen (&a[i0][v0]) == 1);
  ASSERT (strlen (&a[v0][i0]) == 1);

  ASSERT (strlen (a[v1]) == 2);
  ASSERT (strlen (&a[v1][i0]) == 2);
  ASSERT (strlen (&a[i1][v0]) == 2);
  ASSERT (strlen (&a[v1][v0]) == 2);

  ASSERT (strlen (&a[v1][i1]) == 1);
  ASSERT (strlen (&a[v1][i1]) == 1);

  ASSERT (strlen (&a[v1][i2]) == 0);
  ASSERT (strlen (&a[v1][v2]) == 0);
#endif
}

void test_array_off_2_3 (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !(defined(__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ASSERT (strlen (a[0] + 0) == 1);
  ASSERT (strlen (a[0] + v0) == 1);
  ASSERT (strlen (a[v0] + 0) == 1);
  ASSERT (strlen (a[v0] + v0) == 1);

  ASSERT (strlen (a[v1] + 0) == 2);
  ASSERT (strlen (a[1] + v0) == 2);
  ASSERT (strlen (a[v1] + 0) == 2);
  ASSERT (strlen (a[v1] + v0) == 2);

  ASSERT (strlen (a[v1] + 1) == 1);
  ASSERT (strlen (a[v1] + v1) == 1);

  ASSERT (strlen (a[v1] + 2) == 0);
  ASSERT (strlen (a[v1] + v2) == 0);

  int i0 = 0;
  int i1 = i0 + 1;
  int i2 = i1 + 1;

  ASSERT (strlen (a[i0] + i0) == 1);
  ASSERT (strlen (a[i0] + v0) == 1);
  ASSERT (strlen (a[v0] + i0) == 1);
  ASSERT (strlen (a[v0] + v0) == 1);

  ASSERT (strlen (a[v1] + i0) == 2);
  ASSERT (strlen (a[i1] + v0) == 2);
  ASSERT (strlen (a[v1] + i0) == 2);
  ASSERT (strlen (a[v1] + v0) == 2);

  ASSERT (strlen (a[v1] + i1) == 1);
  ASSERT (strlen (a[v1] + v1) == 1);

  ASSERT (strlen (a[v1] + i2) == 0);
  ASSERT (strlen (a[v1] + v2) == 0);
#endif
}

void test_array_ref_2_2_5 (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of code memory
  ASSERT (strlen (b[0][v0]) == 1);
  ASSERT (strlen (b[v0][0]) == 1);

  ASSERT (strlen (&b[0][0][v0]) == 1);
  ASSERT (strlen (&b[0][v0][0]) == 1);
  ASSERT (strlen (&b[v0][0][0]) == 1);

  ASSERT (strlen (&b[0][v0][v0]) == 1);
  ASSERT (strlen (&b[v0][0][v0]) == 1);
  ASSERT (strlen (&b[v0][v0][0]) == 1);

  ASSERT (strlen (b[0][v1]) == 2);
  ASSERT (strlen (b[v1][0]) == 3);

  ASSERT (strlen (&b[0][0][v1]) == 0);
  ASSERT (strlen (&b[0][v1][0]) == 2);
  ASSERT (strlen (&b[v0][0][0]) == 1);

  ASSERT (strlen (&b[0][v0][v0]) == 1);
  ASSERT (strlen (&b[v0][0][v0]) == 1);
  ASSERT (strlen (&b[v0][v0][0]) == 1);

  ASSERT (strlen (&b[0][v1][v1]) == 1);
  ASSERT (strlen (&b[v1][0][v1]) == 2);
  ASSERT (strlen (&b[v1][v1][0]) == 4);
  ASSERT (strlen (&b[v1][v1][1]) == 3);
  ASSERT (strlen (&b[v1][v1][2]) == 2);

  int i0 = 0;
  int i1 = i0 + 1;
  int i2 = i1 + 1;

  ASSERT (strlen (b[i0][v0]) == 1);
  ASSERT (strlen (b[v0][i0]) == 1);

  ASSERT (strlen (&b[i0][i0][v0]) == 1);
  ASSERT (strlen (&b[i0][v0][i0]) == 1);
  ASSERT (strlen (&b[v0][i0][i0]) == 1);

  ASSERT (strlen (&b[i0][v0][v0]) == 1);
  ASSERT (strlen (&b[v0][i0][v0]) == 1);
  ASSERT (strlen (&b[v0][v0][i0]) == 1);

  ASSERT (strlen (b[i0][v1]) == 2);
  ASSERT (strlen (b[v1][i0]) == 3);

  ASSERT (strlen (&b[i0][i0][v1]) == 0);
  ASSERT (strlen (&b[i0][v1][i0]) == 2);
  ASSERT (strlen (&b[v0][i0][i0]) == 1);

  ASSERT (strlen (&b[i0][v0][v0]) == 1);
  ASSERT (strlen (&b[v0][i0][v0]) == 1);
  ASSERT (strlen (&b[v0][v0][i0]) == 1);

  ASSERT (strlen (&b[i0][v1][v1]) == 1);
  ASSERT (strlen (&b[v1][i0][v1]) == 2);
  ASSERT (strlen (&b[v1][v1][i0]) == 4);
  ASSERT (strlen (&b[v1][v1][i1]) == 3);
  ASSERT (strlen (&b[v1][v1][i2]) == 2);
#endif
}

void test_array_off_2_2_5 (void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of code memory
  ASSERT (strlen (b[0][0] + v0) == 1);
  ASSERT (strlen (b[0][v0] + v0) == 1);
  ASSERT (strlen (b[v0][0] + v0) == 1);
  ASSERT (strlen (b[v0][v0] + v0) == 1);

  ASSERT (strlen (b[0][0] + v1) == 0);
  ASSERT (strlen (b[0][v1] + 0) == 2);
  ASSERT (strlen (b[v0][0] + 0) == 1);

  ASSERT (strlen (b[0][v0] + v0) == 1);
  ASSERT (strlen (b[v0][0] + v0) == 1);
  ASSERT (strlen (b[v0][v0] + 0) == 1);

  ASSERT (strlen (b[0][v1] + v1) == 1);
  ASSERT (strlen (b[v1][0] + v1) == 2);
  ASSERT (strlen (b[v1][v1] + 0) == 4);
  ASSERT (strlen (b[v1][v1] + 1) == 3);
  ASSERT (strlen (b[v1][v1] + 2) == 2);

  int i0 = 0;
  int i1 = i0 + 1;
  int i2 = i1 + 1;

  ASSERT (strlen (b[i0][i0] + v0) == 1);
  ASSERT (strlen (b[i0][v0] + v0) == 1);
  ASSERT (strlen (b[v0][i0] + v0) == 1);
  ASSERT (strlen (b[v0][v0] + v0) == 1);

  ASSERT (strlen (b[i0][i0] + v1) == 0);
  ASSERT (strlen (b[i0][v1] + i0) == 2);
  ASSERT (strlen (b[v0][i0] + i0) == 1);

  ASSERT (strlen (b[i0][v0] + v0) == 1);
  ASSERT (strlen (b[v0][i0] + v0) == 1);
  ASSERT (strlen (b[v0][v0] + i0) == 1);

  ASSERT (strlen (b[i0][v1] + v1) == 1);
  ASSERT (strlen (b[v1][i0] + v1) == 2);
  ASSERT (strlen (b[v1][v1] + i0) == 4);
  ASSERT (strlen (b[v1][v1] + i1) == 3);
  ASSERT (strlen (b[v1][v1] + i2) == 2);
#endif
}

void
testTortureExecute (void)
{
  test_array_ref_2_3 ();
  test_array_off_2_3 ();

  test_array_ref_2_2_5 ();
  test_array_off_2_2_5 ();
}
