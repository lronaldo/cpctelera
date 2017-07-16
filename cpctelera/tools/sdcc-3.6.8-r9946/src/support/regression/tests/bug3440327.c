/*
   bug3440327.c
 */

#include <testfwk.h>
#include <setjmp.h>

#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__PORT_HOST)

jmp_buf b;

#define DIM 2
typedef struct
{
	char id;
	int (*f)(char c);
} fstruct;

int j;

int fnct(char c)
{
	static int i;
	i = i + c + 1;
	if (i == 4)
	{
		j = 42;
		longjmp (b, 0);
	}
	return 0;
}

fstruct tab[DIM]={{2, fnct}, {3, 0}};

int bug(void)
{
	register fstruct *dt = tab;
	char val;

	while (dt < tab+DIM)
	{
		val = 0;
		while (val < dt->id)
		{
			(dt->f)(val); /* Wrong pointer used here */
		}
		++dt;
	}

	return 0;
}

#endif

void testBug (void)
{
#if defined(__SDCC_mcs51) || defined(__SDCC_z80) || defined(__PORT_HOST)
	if (setjmp (b))
	{
		ASSERT (j == 42);
	}
	else
	{
		bug ();
		ASSERT (0);
	}
#endif
}

