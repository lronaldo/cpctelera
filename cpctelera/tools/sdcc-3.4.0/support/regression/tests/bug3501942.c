/* bug3501942, an issue with operandsNotrelated() in peepholes for z80-related ports.
 */

#include <testfwk.h>
#include <math.h>

int value;

int f(void)
{
	return 1;
}

void g(void) 
{
	if (value = f()) /* Bug was triggered here. */
		return;
}

void testBug(void)
{
	value = 0;
	g();
	ASSERT (value == 1);
}

