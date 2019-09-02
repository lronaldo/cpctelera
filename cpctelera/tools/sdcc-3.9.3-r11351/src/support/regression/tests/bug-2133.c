/*	bug-2133.c
*/

#include <testfwk.h>
#include <stdint.h>

typedef struct ABC { uint16_t aa, bb, cc; } ABC;

char __xdata buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
ABC __xdata Abc = { 0x1111, 0x2222, 0x3333 };

void testBug2133(void)
{
	ASSERT ( *(char*)&((*(ABC*) buf).cc) == 5); // no errors or warnings; this changes to __data pointer
	ASSERT ( *(char*)&((*(ABC*)&buf).cc) == 5); // sub-optimal code but okay
	ASSERT ( *(char*)&(((ABC*) buf)->cc) == 5); // uses generic pointer, otherwise okay
	ASSERT ( *(char*)&(((ABC*)&buf)->cc) == 5); // same sub-optimal code but okay
}
