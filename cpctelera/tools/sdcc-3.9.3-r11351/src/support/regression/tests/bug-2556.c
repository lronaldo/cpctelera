/* bug-2569.c
   A false error on definitions of functions returning function pointers
   following the declaration.
 */

#include <testfwk.h>
#include <stdint.h>

#ifdef __SDCC
#pragma std_c99
#endif

#pragma disable_warning 85

typedef void l_fptr_1(void);

l_fptr_1 *dosub(void);

l_fptr_1 *dosub(void)
{
    return 0;
}

void (*sigset(int signo, void (*func)(int signo)))(int signo);

void (*sigset(int signo, void (*func)(int signo)))(int signo)
{
	return 0;
}

void testBug(void)
{
  dosub();
#ifndef __SDCC // Bug #2664.
  sigset(0, 0);
#endif
}

