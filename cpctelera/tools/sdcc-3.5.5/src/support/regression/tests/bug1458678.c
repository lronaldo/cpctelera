/*
    bug 1458678
*/

#include <testfwk.h>

/* no need to call this, it generates compiler error:
     error 33: Attempt to assign value to a constant variable (=)
   The keyword "const" seems to
   be attributed to the value pointed to
   instead of to the pointer itself
*/
void should_not_give_error(char * const w)
{
   w[0]='a';
}

/* (OK) */
void does_not_give_error(char * w)
{
   w[0]='a';
}

/* this correctly gives error 33, therefor cannot be regression tested
void gives_error(const char * w)
{
   w[0]='a';
}
*/

void
testDummy(void)
{
}
