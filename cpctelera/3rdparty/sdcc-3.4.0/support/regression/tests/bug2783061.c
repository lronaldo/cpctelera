/*
    bug 2783061
*/

#include <stdarg.h>
#include <testfwk.h>

#define CP	(void __code*)0x1234
#define XP	(void __xdata*)0x5678

void
varargs_fn (char k, ...)
{
  va_list arg;
  void __code * cp;
  void __xdata * xp;
  void * gp;

  va_start (arg, k);

  cp = va_arg (arg, void __code *);
  ASSERT (cp == CP);
  xp = va_arg (arg, void __xdata *);
  ASSERT (xp == XP);
  gp = va_arg (arg, void *);
  ASSERT (gp == (void *)CP);
  gp = va_arg (arg, void *);
  ASSERT (gp == (void *)XP);

  va_end (arg);
}

void
testBug (void)
{
  void __code * cp = CP;
  void __xdata * xp = XP;

  varargs_fn('k', (void __code *)cp, (void __xdata *)xp, cp, xp);
}
