/** loopp counter narrowing optimizatrion test
    type: unsigned long, signed long
*/
#include <testfwk.h>

#include <limits.h>
#include <setjmp.h>

#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
unsigned char array[300];
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
/* A loop where the counter should be narrowed to an 8-bit unsigned type. */
void loop8(unsigned char *a, {type} n)
{
	for({type} i = 0; i < n; i++)
		a[i * n] = 8;
}

/* A loop where the counter should be narrowed to a 16-bit unsigned type, but not further. */
void loop16(unsigned char *a)
{
	for ({type} i = 0; i < 300; i++)
		a[i] = 16;
}

/* A loop where the subtraction should prevent optimization. */
void loopm(unsigned char *a)
{
	for ({type} i = (1ul << 20); i < (1ul << 20) + 1; i++)
		a[i - (1ul << 20)] = 1;
}

void modify1({type} *p)
{
	*p = 17;
}

void modify2({type} *p)
{
	*p = (1ul << 30);
}

/* Loops where access to the counter via pointers should prevent optimization. */
void address(unsigned char *a)
{
	for ({type} i = (1ul << 28); i < (1ul << 30); i++)
	{
		modify1(&i);
		a[i] = 17;
		modify2(&i);
	}

	for ({type} i = (1ul << 28); i < (1ul << 30); i++)
	{
		{type} *p = &i;
		*p = 18;
		a[i] = 18;
		*p = (1ul << 30);
	}
}

void jump_func(jmp_buf *jp, {type} i)
{
	ASSERT (i == (1ul << 29));
	longjmp (*jp, 0);
}

/* A loop where the side-effects from jump_func() should prevent optimization. */
void jump(unsigned char *a)
{
	jmp_buf j;

	if (setjmp (j))
		return;

	for ({type} i = (1ul << 29); i < (1ul << 30); i++)
	{
		jump_func(&j, i);
		a[i] = 14;
	}

	a[0] = 13;
}
#endif

void testLoop(void)
{
#if !defined(__SDCC_mcs51) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
	loop8 (array, 3);
	ASSERT (array[0] == 8);
	ASSERT (array[3] == 8);
	ASSERT (array[6] == 8);

	loop16 (array);
	ASSERT (array[0] == 16);
	ASSERT (array[17] == 16);
	ASSERT (array[255] == 16);
	ASSERT (array[256] == 16);
	ASSERT (array[299] == 16);

	loopm (array);
	ASSERT (array[0] == 1);

	address (array);
	ASSERT (array[17] == 17);
	ASSERT (array[18] == 18);

	jump (array);
	ASSERT (array[0] != 13);
#endif
}

