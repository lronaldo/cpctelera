/** bug-3778.c: Incorrect top byte on 24-bit function pointers
*/

#include <testfwk.h>

int j;

#if defined(__SDCC_stm8) && defined(__SDCC_MODEL_LARGE) // Fill lower 32KB of flash, so f1 or f2 is above 0x10000.
#define ARRAYSIZE 32000
long k;
void dummyfunc(void)
{
	switch(k)
	{
	case 1:
		j = (j + 1) % (j - 1);
	case 2:
		j = (j - 1) % (j + 1);
	case 3:
		j = (j + 1) % (j + 1);
	case 4:
		j = (j - 1) % (j - 1);
	case 5:
		j = (j + 2) % (j - 2);
	case 6:
		j = (j - 2) % (j + 2);
	}
}
#else
#define ARRAYSIZE 1
#endif

const char c[ARRAYSIZE];

void f1(char c, int i) __reentrant
{
	j = c + i;
}

void g1(void(*ptr)(char, int) __reentrant)
{
	(*ptr)(1, 2); // Topmost byte of ptr incorrectly assumed to be 0.
}

void h1(void)
{
	g1(&f1);
}

void f2(char c, int i) __reentrant
{
	j = c + i;
}

void g2(void(*ptr)(char, int) __reentrant)
{
	(*ptr)(2, 3); // Topmost byte of ptr incorrectly assumed to be 0.
}

void h2(void)
{
	g2(&f2);
}

void
testBug(void)
{	
	h1();
	ASSERT(j == 3);
	h2();
	ASSERT(j == 5);
}

