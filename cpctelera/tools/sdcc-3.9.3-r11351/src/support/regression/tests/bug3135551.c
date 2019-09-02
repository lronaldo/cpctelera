/*
   bug3135551.c
*/

#include <testfwk.h>

char test1[] = {3};
char test2[1] = {1 + 2};
char test3[] = {1 + 2, 1 + 2};
char test4[] = {1 + 2}; //this line failed with error 2: Initializer element is not constant
char test5[] = "a";

void testBug(void)
{
	ASSERT (1);
}
