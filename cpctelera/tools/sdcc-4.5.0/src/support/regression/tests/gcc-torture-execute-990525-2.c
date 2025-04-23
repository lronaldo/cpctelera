/*
   990525-2.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc can return struct!
#if 0
typedef struct {
    int v[4];
} Test1;

Test1 func2();

int func1()
{
    Test1 test;
    test = func2();

    if (test.v[0] != 10)
      ASSERT (0);
    if (test.v[1] != 20)
      ASSERT (0);
    if (test.v[2] != 30)
      ASSERT (0);
    if (test.v[3] != 40)
      ASSERT (0);
}

Test1 func2()
{
    Test1 tmp;
    tmp.v[0] = 10;
    tmp.v[1] = 20;
    tmp.v[2] = 30;
    tmp.v[3] = 40;
    return tmp;
}
#endif

void
testTortureExecute (void)
{
#if 0
    func1();
    return;
#endif
}

