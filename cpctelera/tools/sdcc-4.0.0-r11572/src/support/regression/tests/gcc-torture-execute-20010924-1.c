/*
   20010924-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 147
#pragma disable_warning 196
#endif

/* Verify that flexible arrays can be initialized from STRING_CST
   constructors. */

/* Baselines.  */
struct {
  char a1c;
  char *a1p;
} a1 = {
  '4',
  "62"
};

struct {
  char a2c;
  char a2p[2];
} a2 = {
  'v',
  "cq"
};

/* The tests.  */
struct {
  char a3c;
  char a3p[];
} a3 = {
  'o',
  "wx"
};

struct {
  char a4c;
  char a4p[];
} a4 = {
  '9',
  { 'e', 'b' }
};

void
testTortureExecute (void)
{
  if (a1.a1c != '4')
    ASSERT(0);
  if (a1.a1p[0] != '6')
    ASSERT(0);
  if (a1.a1p[1] != '2')
    ASSERT(0);
  if (a1.a1p[2] != '\0')
    ASSERT(0);

  if (a2.a2c != 'v')
    ASSERT(0);
  if (a2.a2p[0] != 'c')
    ASSERT(0);
  if (a2.a2p[1] != 'q')
    ASSERT(0);

  if (a3.a3c != 'o')
    ASSERT(0);
  if (a3.a3p[0] != 'w')
    ASSERT(0);
  if (a3.a3p[1] != 'x')
    ASSERT(0);

  if (a4.a4c != '9')
    ASSERT(0);
  if (a4.a4p[0] != 'e')
    ASSERT(0);
  if (a4.a4p[1] != 'b')
    ASSERT(0);

  return;
}

