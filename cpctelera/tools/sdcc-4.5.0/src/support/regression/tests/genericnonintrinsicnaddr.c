/*
   C11 generic associations and their interaction with non-intrinsic named address spaces
*/

#include <testfwk.h>

void set_a(void)
{
}

void set_b(void)
{
}

#ifndef PORT_HOST
__addressmod set_a space_a;
__addressmod set_b const space_b;

space_a int *ai;
space_b int *bi;
#endif

void testGeneric(void)
{
#ifndef PORT_HOST
	ASSERT(_Generic(ai, default : 0, space_a int *: 1, space_b int* : 2) == 1);
	ASSERT(_Generic(bi, default : 0, space_a int *: 1, space_b int* : 2) == 2);
#endif
}

