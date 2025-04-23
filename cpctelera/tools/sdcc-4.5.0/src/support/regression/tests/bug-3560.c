/* bug-3560.c
   A bug caused by an over-eager loop optimization.
 */

#include <testfwk.h>

#pragma disable_warning 85

#include <stdint.h>

typedef enum { haut, bas, gauche, droite, entree} touche;

uint8_t key;
volatile uint8_t Control_1;
volatile uint8_t next;

void depl(uint16_t touche, uint8_t* end_piece)
{
	ASSERT (!*end_piece);
	*end_piece = 1;
	Control_1 = (key >> 1);
}

uint8_t possible (uint8_t * pce)
{
	return key;
}

void mygame()
{
	
	uint8_t end_piece;
	uint8_t end_game=0;
	
	while(! end_game) 
	{
		do
		{	
			end_piece = 0; 
			key = Control_1;

			if (!(key & 0b00000010))
				depl(bas, &end_piece);
			else if (!(key & 0b00001000))
				depl(droite, &end_piece);
			else if (!(key & 0b00000100))
				depl(gauche, &end_piece);
		}
		while(!end_piece);
		end_game= (!possible(&next));
	}
}

void
testBug(void)
{
	Control_1 = 0b00000111;
	mygame();
}

