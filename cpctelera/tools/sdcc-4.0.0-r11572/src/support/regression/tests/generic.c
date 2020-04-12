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
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  ASSERT (_Generic(i, default : 0, int : 1, long : 2) == 1);
  ASSERT (_Generic(l, default : 0, int : 1, long int : 2) == 2);
  ASSERT (_Generic(l, default : 0, int : 1, char : 2) == 0);
  ASSERT (_Generic('c', default : 0, int : 1, char : 2) == 1);
  ASSERT (_Generic(7, default : 0, int : 1, char : 2) == 1);
  ASSERT (_Generic(e, default : 0, int : 1, char : 2) == 1);
  ASSERT (_Generic(c, default : 0, int : 1, char : 2) == 2);
  ASSERT (_Generic("c"[0], char : 1, default : 0) == _Generic(c, char : 1, default : 0)); // There once was a bug where the sign of plain char different from the sign of char in string literals.
#endif
}

