/*
   bug1579664.c
 */

#include <testfwk.h>

// no need to call this, it generates parser error:
//   syntax error: token -> 'foo' ; column 10
typedef struct foo foo;
struct foo
{
    foo * next;
    int bar;
};

void
testBug (void)
{
	ASSERT (1);
}
