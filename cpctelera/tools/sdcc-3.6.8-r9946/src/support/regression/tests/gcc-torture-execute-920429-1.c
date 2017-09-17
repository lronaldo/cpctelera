/*
   920429-1.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>
#pragma disable_warning 196

#ifdef __SDCC
#pragma std_c99
#endif

typedef unsigned char t;int i,j;
t*f(const t*p){t c;c=*p++;i=((c&2)?1:0);j=(c&7)+1;return p;}

void
testTortureExecute (void){const t*p0="ab",*p1;p1=f(p0);if(p0+1!=p1)ASSERT(0);return;}

