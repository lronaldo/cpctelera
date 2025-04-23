/*
    bug 2982135
*/

#include <testfwk.h>

#ifndef PORT_HOST
#pragma disable_warning 85  //in function f2 unreferenced function argument : 'n'
#endif

char* fp;

char* f1(int n){
	char* tmp=fp;
	fp+=n;
	return tmp;
}

char* f2(int n){
	char* tmp=fp;
	fp+=3;
	return tmp;
}

void test2982135(void)
{
	fp = (char __xdata *)42;
	ASSERT(f1(23) == (char __xdata *)42);
	fp = (char __xdata *)42;
	ASSERT(f2(23) == (char __xdata *)42);
}

