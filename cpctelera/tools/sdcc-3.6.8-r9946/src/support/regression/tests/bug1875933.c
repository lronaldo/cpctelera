/*
 * bug1875933.c
 */

#include <testfwk.h>
#include <stdint.h>

char identity(char x)
{
  return x;
}

/*
 * function genAnd() and genOr() in z80/gen.c
 * were not prepared to handle the special case where ifx == 0
 */

void void_tand1(char x)
{
  char y = (identity(x) & 1) ? 42 : 43;
}

void void_tand0(char x)
{
  char y = (identity(x) & 0) ? 42 : 43;
}

void void_txor1(char x)
{
  char y = (identity(x) ^ 1) ? 42 : 43;
}

void void_txor0(char x)
{
  char y = (identity(x) ^ 0) ? 42 : 43;
}


/*
 * function genOr() in z80/gen.c
 *   assumed identity of "or a, literal" and "or a,a"
 *   thats definitly not so
 */

char tor1(char x)
{
  char y = (identity(x) | 1) ? 42 : 43;
  return y;
}

char tor0(char x)
{
  char y = (identity(x) | 0) ? 42 : 43;
  return y;
}

char tand1(char x)
{
  char y = (identity(x) & 1) ? 42 : 43;
  return y;
}

char tand0(char x)
{
  char y = (identity(x) & 0) ? 42 : 43;
  return y;
}

char txor1(char x)
{
  char y = (identity(x) ^ 1) ? 42 : 43;
  return y;
}

char txor0(char x)
{
  char y = (identity(x) ^ 0) ? 42 : 43;
  return y;
}

/*
 * mcs51 segmentation fault
 *
 * function genOr() in mcs51/gen.c
 *   was not prepeared for ifx==0
 */

void void_tor1(char x)
{
  char y = (identity(x) | 1) ? 42 : 43;
}

void void_tor0(char x)
{
  char y = (identity(x) | 0) ? 42 : 43;
}

void void_tor(char x)
{
  char y = (identity(x) | x) ? 42 : 43;
}

void
testBug(void)
{
  void_tand1(1);
  void_tand1(0);
  void_tand0(1);
  void_tand0(0);
  void_txor1(1);
  void_txor0(0);

  ASSERT(tor1(1)  == 42);
  ASSERT(tor1(0)  == 42);
  ASSERT(tor0(1)  == 42);
  ASSERT(tor0(0)  == 43);
  ASSERT(tand1(1) == 42);
  ASSERT(tand1(0) == 43);
  ASSERT(tand0(1) == 43);
  ASSERT(tand0(0) == 43);
  ASSERT(txor1(1) == 43);
  ASSERT(txor1(0) == 42);
  ASSERT(txor0(1) == 42);
  ASSERT(txor0(0) == 43);

  void_tor1(1);
  void_tor1(0);
  void_tor0(1);
  void_tor0(0);
  void_tor(0);
}
