/*
   bug1929140.c
 */

#include <testfwk.h>

// no need to call this, it generates parser error:
//   syntax error: token -> 'foo' ; column 10
extern void (* P1032E)(void);

extern void (* P1032E)(void);


void
testBug (void)
{
	ASSERT (1);
}
