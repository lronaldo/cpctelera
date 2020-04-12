/*
    bug-2627.c, an issue in initialization of unterminated strings that contain \" or \\ escape sequences.
*/

#include <testfwk.h>

#ifndef PORT_HOST
#pragma disable_warning 147
#endif

char array1[2] = "\"t";
char array2[2] = "\\t";
const char array3[2] = "\"t";
const char array4[2] = "\\t";

void testBug(void)
{
	ASSERT(array1[0] == '\"');
	ASSERT(array2[0] == '\\');
	ASSERT(array3[0] == '\"');
	ASSERT(array4[0] == '\\');
}

