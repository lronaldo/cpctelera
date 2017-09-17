/* Float tests.

   operation: ADD, SUB, MUL, DIV, REVDIV
*/

#if 1
  // we are in the regression tests
  #include <testfwk.h>
  #define DEBUG(x)

  #define {operation} 1
#else
  // we are standalone
  #include <stdio.h>
  #define DEBUG(x) x
  #define ASSERT(x)
  #define ADD 1
  #define SUB 1
  #define MUL 1
  #define DIV 1
  #define REVDIV 1
#endif

#ifdef __SDCC_mcs51
#  define STORAGE __xdata
#  define XDATA __xdata
#elif __SDCC_pic16
#  define STORAGE __code
#  define XDATA
#else
#  define STORAGE
#  define XDATA
#endif

XDATA volatile float left, right, result;

struct {
  float left, right, add, sub, mul, div, revdiv;
} STORAGE cases[]={
  //     left  right        add        sub          mul         div   revdiv
  {      12.8,  25.6,      38.4,     -12.8,      327.68,        0.5,  2},
  {      12.8, -25.6,     -12.8,      38.4,     -327.68,       -0.5, -2},
  {     -12.8,  25.6,      12.8,     -38.4,     -327.68,       -0.5, -2},
  {     -12.8, -25.6,     -38.4,      12.8,      327.68,        0.5,  2},
  {     100.0,  10.0,     110.0,      90.0,     1000.00,       10.0,  0.1},
  {    1000.0,  10.0,    1010.0,     990.0,    10000.00,      100.0,  0.01},
  {   10000.0,  10.0,   10010.0,    9990.0,   100000.00,     1000.0,  0.001},
  {  100000.0,  10.0,  100010.0,   99990.0,  1000000.00,    10000.0,  0.0001},
  { 1000000.0,  10.0, 1000010.0,  999990.0, 10000000.00,   100000.0,  0.00001},
  {10000000.0,  10.0,10000010.0, 9999990.0,100000000.00,  1000000.0,  0.000001},
  {     0x100,  0x10,     0x110,      0xf0,      0x1000,       0x10,  0.0625},
  {    0x1000,  0x10,    0x1010,     0xff0,     0x10000,      0x100,  0.00390625},
  {   0x10000,  0x10,   0x10010,    0xfff0,    0x100000,     0x1000,  0.00024414},
  {  0x100000,  0x10,  0x100010,   0xffff0,   0x1000000,    0x10000,  0 /* ignore */},
  { 0x1000000,  0x10, 0x1000010,  0xfffff0,  0x10000000,   0x100000,  0 /* ignore */},
  {0x10000000,  0x10,0x10000010, 0xffffff0, (float)0x10000000*0x10,
                                                          0x1000000,  0 /* ignore */},
};

XDATA int tests = 0, errors = 0;

char
compare (float is, float should)
{
  float diff = should ? is / should : 0;
  tests++;
  DEBUG (printf (" %1.3f (%f %f) ", is, should, diff));

  if (should == 0)
    {
      DEBUG (printf ("IGNORED!\n"));
      return 0;
    }

  // skip the fp roundoff errors
  if (diff > 0.999999 && diff < 1.00001)
    {
      DEBUG (printf ("OK!\n"));
      ASSERT (1);
      return 0;
    }
  else
    {
      errors++;
      DEBUG (printf ("FAIL!\n"));
      ASSERT (0);
      return 1;
    }
}

void
testFloatMath (void)
{
  int i;
  int t = sizeof (cases) / sizeof (cases[0]);
  float result;

  for (i = 0; i < t; i++)
    {
      #ifdef ADD
        // add
        result = cases[i].left + cases[i].right;
        DEBUG (printf ("%1.3f + %1.3f =", cases[i].left, cases[i].right));
        compare (result, cases[i].add);
      #endif
      #ifdef SUB
        // sub
        result = cases[i].left - cases[i].right;
        DEBUG (printf ("%1.3f - %1.3f =", cases[i].left, cases[i].right));
        compare (result, cases[i].sub);
      #endif
      #ifdef MUL
        // mul
        result = cases[i].left * cases[i].right;
        DEBUG (printf ("%1.3f * %1.3f =", cases[i].left, cases[i].right));
        compare (result, cases[i].mul);
      #endif
      #ifdef DIV
        // div
        result = cases[i].left / cases[i].right;
        DEBUG (printf ("%1.3f / %1.3f =", cases[i].left, cases[i].right));
        compare (result, cases[i].div);
      #endif
      #ifdef REVDIV
        // revdiv
        result = cases[i].right / cases[i].left;
        DEBUG (printf ("%1.3f / %1.3f =", cases[i].right, cases[i].left));
        compare(result, cases[i].revdiv);
      #endif
    }
  DEBUG (printf ("%d tests, %d errors\n", tests, errors));
}

void
testFloatMulRound (void)
{
    right = 2.0 / 10.61;
    result = 10.61 * right;
    compare (result, 2.0);
}

#if 0
void
main (void)
{
  testFloatMath ();
  testFloatMulRound ();
}
#endif

