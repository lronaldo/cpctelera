/*
   920908-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

// TODO: Enable when sdcc supports returning struct!
#if 0
/* REPRODUCED:RUN:SIGNAL MACHINE:mips OPTIONS: */

#include <stdarg.h>

typedef struct{int A;}T;

T f(int x,...)
{
va_list ap;
T X;
va_start(ap,x);
X=va_arg(ap,T);
if(X.A!=10)abort();
X=va_arg(ap,T);
if(X.A!=20)abort();
va_end(ap);
return X;
}
#endif

void
testTortureExecute (void)
{
#if 0
T X,Y;
int i;
X.A=10;
Y.A=20;
f(2,X,Y);
return;
#endif
}

