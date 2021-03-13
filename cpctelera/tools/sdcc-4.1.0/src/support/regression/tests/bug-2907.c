/* bug-2820.c
   short was sometimes considered to be the sme type as int.
 */

#include <testfwk.h>

void testBug(void)
{
	short s;
	int i;

	s = _Generic (s, short : 1, int : 2, default : 0);
	i = _Generic (i, short : 1, int : 2, default : 0);

	ASSERT (s == 1);
	ASSERT (i == 2);
}

