/** bug-3824.c: A z80 peephole optimize rule error that resulted in a missing label in the assembler code.
 */

#include <testfwk.h>

char f(char *c);

void g(void)
{
	char c[4];
	if(f(c))
		return;
	if(f(c))
		return;
}

char f(char *c)
{
	c[0] = 0;
}

