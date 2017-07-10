/*
   981019-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

extern int f2(void);
extern int f3(void);
extern void f1(void);

void
ff(int fname, int part, int nparts)
{
  if (fname)  /* bb 0 */
    {
      if (nparts)  /* bb 1 */
	f1();  /* bb 2 */
    }
  else
    fname = 2; /* bb 3  */

  /* bb 4 is the branch to bb 10
     (bb 10 is physically at the end of the loop) */
  while (f3() /* bb 10 */)
    {
      if (nparts /* bb 5 */ && f2() /* bb 6 */)
	{
	  f1();  /* bb 7 ... */
	  nparts = part;
	  if (f3())  /* ... bb 7 */
	    f1();  /* bb 8 */
	  f1(); /* bb 9 */
	  break;
	}
    }

  if (nparts)  /* bb 11 */
    f1(); /* bb 12 */
  return; /* bb 13 */
}

void
testTortureExecute (void)
{
  ff(0, 1, 0);
  return;
}

int f3(void) { static int x = 0; x = !x; return x; }
void f1(void) { ASSERT(0); }
int f2(void) { ASSERT(0); }

