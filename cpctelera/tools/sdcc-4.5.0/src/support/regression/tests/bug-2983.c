/* bug-2093.c
   iCode generation for a cast tried to read a non-literal operand as literal.
 */

#include <testfwk.h>

typedef struct {
    unsigned int v0;
    unsigned int v1;
} test_t;

volatile unsigned char test=0;

int f(void) {
    unsigned int *t4=&((test_t *)0)->v1;
    test=(unsigned char)t4;

    void *t6=(void *)&((test_t *)0)->v1; // Compilation failed with internal error on this line.
    test=(unsigned char)t6;

    return 0;
}

void
testBug(void)
{
	ASSERT (!f());
}

