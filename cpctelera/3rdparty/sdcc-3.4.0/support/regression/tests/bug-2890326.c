/*
    bug-2890326.c
*/

#include <testfwk.h>

#include <stdint.h>

struct position
{
	uint_fast8_t track;
	uint_fast8_t flags;
};

#define MAX_VEHICLES 9

struct vehicle
{
	struct position ends[2];
	struct vehicle *next, *previous;
};

struct vehicle vehicles[MAX_VEHICLES];

// Hangs sdcc (unless using --no-peep).
void f(void)
{
	uint_fast8_t track;
	track = vehicles[1].ends[0].track;

	if(track != 4 && track != 5 && track != 6)
		return;

	for(;;);
}

void
testBug(void)
{
	ASSERT(1);
}

