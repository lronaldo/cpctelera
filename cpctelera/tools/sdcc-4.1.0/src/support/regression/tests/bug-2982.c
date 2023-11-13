/* bug-2982.c
   A bug in the z80 peephole optimizer.
 */

#include <testfwk.h>

#pragma disable_warning 85

static int L, STEP;

void Basic_PRINT(int i)
{
  ASSERT (i == 36);
}

signed char Basic_RND(char a)
{
  return 51;
}

int f(int argc, char **argv)
{
    STEP = Basic_RND(0);
    do {
        // A load of L into register pair de got optimized out, though it was still needed for the claculation of L + 50
        Basic_PRINT((L + 50) / 100);
        L += STEP;
    } while (!(L > 3600));
}

void testBug(void)
{
	L = 3550;
	f(0, 0);
}

