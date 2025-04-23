/** setjmp/longjmp tests (focusing on getting the jump right).
*/
#include <testfwk.h>

#if !defined(__SDCC_pic14) // Unimplemented setjmp
#include <setjmp.h>

unsigned int global_int = 0;
unsigned int *gpInt;

#if defined(__SDCC_mcs51)
#include <8052.h>

void
T2_isr (void) __interrupt (5) //no using
{
  //do not clear flag TF2 so it keeps interrupting !
  (*gpInt)++;
}
#endif

void
try_fun (jmp_buf catch, int except)
{
  longjmp (catch, except);
}

jmp_buf buf;

void g(void)
{
	longjmp(buf, 0); // When called with an argument of 0, longjmp() makes setjmp() return 1 instead.
	g();
}

void f1(void)
{
	static int i;
	int j;
	i= 0;
	j= setjmp(buf);
	ASSERT(i == j);
	i++;
	if(!j)
		g();
}

#endif

// Get FreeBSD version to skip part of test for known broken setjmp (FreeBSD bug #255320, affecting at least FreeBSD 13.0 and FreeBSD 13.1).
#ifdef __FreeBSD__
#include <sys/param.h>
#endif

void
testJmp (void)
{
#if !defined(__SDCC_pic14) // Unimplemented setjmp
  jmp_buf catch;
  int exception;

#if defined(__SDCC_mcs51)
  gpInt = &global_int;
  //enable the interrupt and set it's flag to generate some heavy stack usage
  ET2 = 1;
  EA = 1;
  TF2 = 1;
#endif

  exception = setjmp (catch);
  if (exception == 0)
    {
      try_fun (catch, 1);
      //should not get here!
      ASSERT (0);
    }
  ASSERT (exception == 1);
  
#if !defined(__FreeBSD__) || __FreeBSD_version >= 1302000 // Known FreeBSD 13.0 and 13.1 bug #255320.
  f1();
#endif
#endif

// C99 might require setjmp to be a macro. The standard seems self-contradicting on this issue.
//#ifndef setjmp
//  ASSERT(0);
//#endif
}

