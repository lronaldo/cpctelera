/*
   20000224-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

int loop_1 = 100;
int loop_2 = 7;
int flag = 0;

int test (void)
{
    int i;
    int counter  = 0;

    while (loop_1 > counter) {
        if (flag & 1) {
            for (i = 0; i < loop_2; i++) {
                counter++;
            }
        }
        flag++;
    }
    return 1;
}

void
testTortureExecute (void)
{
    if (test () != 1)
      ASSERT (0);
    
    return;
}

