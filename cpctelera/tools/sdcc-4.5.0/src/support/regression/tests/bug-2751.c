/*
   bug-2643.c
   char was the same type as either signed char or unsigned char.
 */

#include <testfwk.h>
#include <string.h>

const char *g;

void f(void)
{
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	g = _Generic("test"[0], char: "char", unsigned char: "unsigned char", signed char: "signed char");
#endif
}

void testBug(void)
{
	f();
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	ASSERT (!strcmp(g, "char"));
#endif
}

