/* bug-3372.c
   A bug in integer promotion for unary +
 */

#include <testfwk.h>

char c;
int i = _Generic(+c, int: 1); //  Failed to compile

void testBug(void)
{
}

