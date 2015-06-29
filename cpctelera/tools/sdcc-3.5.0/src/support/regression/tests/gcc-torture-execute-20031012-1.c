/*
   20031012-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR optimization/8750
   Used to fail under Cygwin with 
   -O2 -fomit-frame-pointer        
   Testcase by David B. Trout     */

#if defined(STACK_SIZE) && STACK_SIZE < 16000
#define ARRAY_SIZE (STACK_SIZE / 2)
#define STRLEN	   (ARRAY_SIZE - 9)
#else
#define ARRAY_SIZE 15000
#define STRLEN     13371
#endif

#include <string.h>

static void foo ()
{
#if !defined (__SDCC_gbz80) && !defined (__SDCC_mcs51) && !defined (__SDCC_stm8)
    char a[ARRAY_SIZE]; /* Too big for mcs51 and gbz80 and stm8. */
    a[0]=0;
    memset( &a[0], 0xCD, STRLEN );
    a[STRLEN]=0;
    if (strlen(a) != STRLEN)
      ASSERT (0);
#endif
}

void
testTortureExecute (void)
{
    foo();
    return;
}

