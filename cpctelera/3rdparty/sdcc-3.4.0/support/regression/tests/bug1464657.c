/*
   bug1464657.c
*/

#include <testfwk.h>

static void f1(void)
{
}

static short f2(void)
{
  return 0;
}

static unsigned char f3(unsigned char a)
{
  return a+5;
}

unsigned char var;

void f()
{
  switch (var) {
    case 0:
      f1();
      break;
  case 1:
      f1();
      break;
  case 2:
      var++;
      f1();
      break;
    case 4:
      f1();
      break;
    case 5:
      break;
    case 7:
      f1();
      break;
    case 6:
      f1();
      break;
    case 8:
      f1();
      break;
    case 9:
      f1();
      break;
    case 12:
      {
        unsigned char nc;
        unsigned short b = f2();
        unsigned char nb = f3(b);
        var = f3(b);
        nc = f3(nb);
      }
      break;
  }
}

void test_Peephole251(void)
{
  var = 12;
  f();
  ASSERT (var==5);
}
