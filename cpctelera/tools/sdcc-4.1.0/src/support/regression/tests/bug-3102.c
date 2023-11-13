/* bug-3102.c
   A problem in the interaction of two peephole rules with rare code and an assembler quirk.
 */
 
#include <testfwk.h>

static unsigned char Flag[10];
static unsigned char Object[10];

#define CARRIED		(Flag[2])
#define WORN		(Flag[3])

static void Message(unsigned char m)
{
}

static void DropItem(void)
{
}

static void Put(unsigned char obj, unsigned char loc)
{
}

static void Wear(unsigned char obj)
{
	if (Object[obj] == WORN) {
		Message(29);
		return;
	}
	if (Object[obj] != CARRIED) {
		Message(23);
		return;
	}
	DropItem();
	Put(obj, WORN);
}

void
testBug(void)
{
	Wear(0);
}

