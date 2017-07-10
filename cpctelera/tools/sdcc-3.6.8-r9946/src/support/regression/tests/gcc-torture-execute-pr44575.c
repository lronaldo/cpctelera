/*
   pr44575.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

/* PR target/44575 */

#include <stdarg.h>

int fails = 0;
struct S { float a[3]; };
struct S a[5];
#if 0
TODO: Enable once structures can be passed as parameters!
void
check (int z, ...)
{
  struct S arg, *p;
  va_list ap;
  int j = 0, k = 0;
  int i;
  va_start (ap, z);
  for (i = 2; i < 4; ++i)
    {
      p = 0;
      j++;
      k += 2;
      switch ((z << 4) | i)
	{
	case 0x12:
	case 0x13:
	  p = &a[2];
	  arg = va_arg (ap, struct S);
	  break;
	default:
	  ++fails;
	  break;
	}
      if (p && p->a[2] != arg.a[2])
	++fails;
      if (fails)
	break;
    }
  va_end (ap);
}
#endif
void
testTortureExecute (void)
{
#if 0
  a[2].a[2] = -49026;
  check (1, a[2], a[2]);
  if (fails)
    ASSERT (0);
  return;
#endif
}

