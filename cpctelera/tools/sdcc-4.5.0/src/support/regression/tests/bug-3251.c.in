/*
   bug-3251.c An assertion failure in stm8 code generation.
   convention: , __raisonance, __iar, __cosmic
 */
 
#include <testfwk.h>

#ifdef __SDCC_stm8
int f(int i, int j, int (*c)(int , int) {convention})
{
	return (*c)(i, j);
}

int add(int i, int j) {convention}
{
	return(i + j);
}
#endif

void
testBug(void)
{
#ifdef __SDCC_stm8
	ASSERT (f(1, 1, add) == 2);
#endif
}

