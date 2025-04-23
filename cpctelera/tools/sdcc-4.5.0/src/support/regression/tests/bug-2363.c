/*
    bug-2363.c, SDCC got confused by the use of uninitialized variables, resulting in assertion failures.
*/

#include <testfwk.h>

#pragma disable_warning 84

#define X(i, ub) (i)

typedef	struct Durak_Card { signed char suit, rank; } Durak_Card;

typedef Durak_Card Durak_Cards[37];

#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Not enough memory
static Durak_Cards Durak_hand[2];
static Durak_Card Durak_desk[13];
static signed char Durak_deskN, Durak_trump, Durak_badSuit;
#endif

static void Durak_CpuMoves (void)
{
#if !(defined(__SDCC_mcs51) && defined(__SDCC_MODEL_SMALL)) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_pic14) // Not enough memory
	signed char i, l;
	signed int z;
	signed char _for__14;

	if (Durak_deskN != 0) {
		if (Durak_desk[X(Durak_deskN, 13)].suit == Durak_trump && Durak_desk[X(i - 1, 13)].suit != Durak_trump) {
			Durak_badSuit = Durak_desk[X(i - 1, 13)].suit;
		}
	}
	while (l <= _for__14) {
		for (;;) {
			i = l;
			if ((Durak_hand[1][X(l, 37)].rank)) {
				z = Durak_hand[1][X(i, 37)].rank * 10 + (signed char)(Durak_hand[1][X(i, 37)].suit == Durak_trump) * 111;
			}
		}
	}
#endif
}

void testBug(void)
{
}

