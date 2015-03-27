/*
    bug 1579949
*/

#include <testfwk.h>

struct st_s
{
	char el;
};

// no need to call this, it generates compiler error for z80, hc08, stack-auto
//   error 9: FATAL Compiler Internal Error
char foo (int x, struct st_s *arg)
{
	x;
	return ((struct st_s *) arg) -> el;
}

void
testBug(void)
{
	ASSERT(1);
}
