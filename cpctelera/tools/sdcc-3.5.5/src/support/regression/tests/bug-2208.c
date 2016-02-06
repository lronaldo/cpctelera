/*
   bug-2208.c
*/

#include <testfwk.h>

#ifndef __SDCC_mcs51
typedef
    struct SDCCBUG_Card {
        signed char suit, rank;
    } SDCCBUG_Card;

typedef
    SDCCBUG_Card SDCCBUG_Cards[37];

SDCCBUG_Cards SDCCBUG_pack;
unsigned char SDCCBUG_packN;
unsigned char SDCCBUG_handN[2];
SDCCBUG_Cards SDCCBUG_hand[2];
signed char SDCCBUG_skill = 1;

void SDCCBUG_DealCardsTo (unsigned char player)
{
    unsigned char i, j;
    SDCCBUG_Card tmp;
    if ((int)SDCCBUG_handN[player] >= 6 || (int)SDCCBUG_packN == 0) {
        return;
    }
    i = (int)SDCCBUG_handN[player] + 1;
    while ((int)i <= 6) {
        if (SDCCBUG_skill == 4) {
            j = (int)SDCCBUG_packN - 3;
            if (SDCCBUG_pack[SDCCBUG_packN].rank > SDCCBUG_pack[j].rank) {
                  tmp.suit = SDCCBUG_pack[SDCCBUG_packN].suit;
            }
        }

        SDCCBUG_hand[player][i].suit = SDCCBUG_pack[SDCCBUG_packN].suit;
        SDCCBUG_hand[player][i].rank = SDCCBUG_pack[SDCCBUG_packN].rank;
        SDCCBUG_handN[player] = i;
        SDCCBUG_packN -= 1;

        if ((int)SDCCBUG_packN == 0) {
            return;
        }
        i += 1;
    }
}
#endif

void testBug(void)
{
#ifndef __SDCC_mcs51
	SDCCBUG_packN = 1;
	SDCCBUG_skill = 1;
	SDCCBUG_pack[SDCCBUG_packN].suit = 23;
	SDCCBUG_pack[SDCCBUG_packN].rank = 42;
	SDCCBUG_handN[0] = 0;

	SDCCBUG_DealCardsTo (0);

	ASSERT(SDCCBUG_hand[0][1].suit == 23);
	ASSERT(SDCCBUG_hand[0][1].rank == 42);
	ASSERT(!SDCCBUG_packN);
#endif
}

