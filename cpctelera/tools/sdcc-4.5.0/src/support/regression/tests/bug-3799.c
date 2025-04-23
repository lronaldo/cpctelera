/** bug-3779.c: code generation for z80 affecting the passing of struct members of the same global as subsequent paramters
*/

#include <testfwk.h>

struct s
{
	unsigned char c[4];
};

struct s a[3];

void f(struct s s0, struct s s1, struct s s2)
{
	ASSERT(s0.c[0] == 0xa5);
	ASSERT(s0.c[1] == 0x00);
	ASSERT(s1.c[0] == 0x5a);
	ASSERT(s1.c[1] == 0x00);
	ASSERT(s2.c[0] == 0xaa);
	ASSERT(s2.c[1] == 0x55);
}

void testBug(void)
{
	a[0].c[0] = 0xa5;
	a[1].c[0] = 0x5a;
	a[2].c[0] = 0xaa;
	a[2].c[1] = 0x55;

	f(a[0], a[1], a[2]); // When passing the second, the cached value for the hl register was wrong, resulting in the passed data being read at an off-by-one-byte location from a[1].
}

