/*
   bug3564104.c
*/

#include <testfwk.h>

#include <stdint.h>

#ifdef __SDCC
#pragma disable_warning 127
#endif

uintptr_t array[4];

int i;

void g(const void *p)
{
	array[i++] = (uintptr_t)(p);
}

void f1(uintptr_t base, const char *message)
{
	for(; *message; message++)
		g((const void *)(base + *message * 8));
}

void f2(uintptr_t base, const char *message)
{
	for(; *message; message++)
	{
		volatile uintptr_t tmp = base + *message * 8;
		g((const void *)(tmp));
	}
}

void testBug(void)
{
	char a[] = {10, 20, 30, 40, 0};

	i = 0;
	f1(0, a);

	ASSERT(array[0] == 10 * 8);
	ASSERT(array[1] == 20 * 8);
	ASSERT(array[2] == 30 * 8);
	ASSERT(array[3] == 40 * 8);

	i = 0;
	f2(0, a);

	ASSERT(array[0] == 10 * 8);
	ASSERT(array[1] == 20 * 8);
	ASSERT(array[2] == 30 * 8);
	ASSERT(array[3] == 40 * 8);
}

