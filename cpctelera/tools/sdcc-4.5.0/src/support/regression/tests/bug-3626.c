/* bug-3626.c
   For optimization of jump table conditions, a computeOnly flag was ignored in common subexpression elimination,
   resulting in an optimization being triggered incorrectly.
*/

#include <testfwk.h>

#include <setjmp.h>

jmp_buf buf;

int p(int c)
{
	static int i = '0';
	ASSERT (c == i++);
	if (c == '5')
		longjmp (buf, 1);
}

void m(void)
{
    unsigned char state = 0;
    while (1) {
        p(state + '0');
        switch (state) { // Condition in this switch statement was wrongly replaced by constant 0.
            case  0: state = 1; break;
            case  1: state = 2; break;
            case  2: state = 3; break;
            case  3: state = 4; break;
            case  4: state = 5; break;
            default: state = 0; break;
        }
    }
}

void 
testBug (void)
{
	if (!setjmp (buf))
		m ();
}

