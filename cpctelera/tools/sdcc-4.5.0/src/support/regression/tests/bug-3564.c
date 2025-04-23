/* bug-3523.c
   An issue in handling designated initializers where the order of initalization is reversed vs. the order of members.
 */

#include <testfwk.h>

typedef struct {
    char *current;
    int x;
} BlockData1;

typedef struct {
    char *current;
    int x;
    long l;
} BlockData2;

void g1(int i, char *c)
{
	ASSERT (i == 110);
	ASSERT (c == 0);
}

void g2(int i, char *c, long l)
{
	ASSERT (i == 110);
	ASSERT (c == 0);
	ASSERT (l == 0);
}

void g3(int i, char *c, long l)
{
	ASSERT (i == 110);
	ASSERT (c == 0);
	ASSERT (l == 42);
}

void g4(int i, char *c, long l)
{
	ASSERT (i == 110);
	ASSERT (c == 0);
	ASSERT (l == 0);
}

void f1(void)
{
    BlockData1 block = { .x = 110, .current = 0 };

	g1 (block.x, block.current );
}

void f2(void)
{
    BlockData2 block = { .x = 110, .current = 0 };

	g2 (block.x, block.current, block.l );
}

void f3(void)
{
    BlockData2 block = { .x = 110, .l = 42 };

	g3 (block.x, block.current, block.l );
}

void f4(void)
{
    BlockData2 block = { .x = 110};

	g4 (block.x, block.current, block.l );
}

void
testBug (void) {
	f1 ();
	f2 ();
	f3 ();
	f4 ();
}

