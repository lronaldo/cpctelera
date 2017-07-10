/*
 * bug1932626.c
 */

#include <testfwk.h>
#include <string.h>

void CopyString(char acSource[], char acDestination[])
{
	unsigned char ucCurrentPosition;

	for (ucCurrentPosition = 0; ; ucCurrentPosition++)
	{
		acDestination[ucCurrentPosition]=acSource[ucCurrentPosition];
		if (0==acSource[ucCurrentPosition])
			break;
	}
}

void testBug(void)
{
	char text1[] = "spam";
	char text2[16];
	CopyString(text1, text2);
	ASSERT(!strcmp(text1, text2));
}
