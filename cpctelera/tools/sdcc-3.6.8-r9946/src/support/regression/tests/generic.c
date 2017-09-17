/*
   C11 generic associations
*/

#include <testfwk.h>

int i;
long l;
enum {e};
typedef char t;
t c;

void testGeneric(void)
{
#ifndef PORT_HOST // Too many old host compilers out there
  ASSERT (_Generic(i, default : 0, int : 1, long : 2) == 1);
  ASSERT (_Generic(l, default : 0, int : 1, long int : 2) == 2);
  ASSERT (_Generic(l, default : 0, int : 1, char : 2) == 0);
  ASSERT (_Generic('c', default : 0, int : 1, char : 2) == 1);
#if 0 // Integer constant still have wrong type when small
  ASSERT (_Generic(7, default : 0, int : 1, char : 2) == 1);
#endif
  ASSERT (_Generic(e, default : 0, int : 1, char : 2) == 1);
  ASSERT (_Generic(c, default : 0, int : 1, char : 2) == 2);
#endif
}

