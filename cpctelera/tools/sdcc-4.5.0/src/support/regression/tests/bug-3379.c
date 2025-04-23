/* bug-3368.c
   A bug in stm8 codegen affecting tail-call optimization of calls to functions taing a single 8-bit parameter from functions that put exactly 1 byte of local variables on the stack.
 */

#include <testfwk.h>

#include <stdbool.h>

volatile unsigned char a;

static inline void ta(void)
{
    a |= 0x1;
}

static void sub_func(bool my_sub_flag)
{
    ASSERT (my_sub_flag);
}

static void sdcc42_bug(bool my_flag)
{
    if (my_flag)
        ta();
    else
        ta();

    sub_func(true);
}

void testBug(void)
{
	sdcc42_bug(false);
}

