/*
        bug 2051348.
*/

#include <testfwk.h>

#pragma disable_warning 85

typedef char BOOLEAN;
typedef signed char SHORTINT;

void App_PutSprite (SHORTINT x, SHORTINT y, SHORTINT spr)
{
}

static SHORTINT Labirint_objCell, Labirint_manX, Labirint_manY, Labirint_manDirX, Labirint_manDirY;
static BOOLEAN Labirint_manPresent;

static SHORTINT Labirint_GetManSprite (void)
{
	return(0);
}

static BOOLEAN Labirint_ManCanGo (void)
{
	return(0);
}

static BOOLEAN Labirint_ManGoing (SHORTINT x, SHORTINT y)
{
	return(0);
}

static void Labirint_SetCell (SHORTINT x, SHORTINT y, SHORTINT cell)
{
}

static void Labirint_TryMoveMan (void)
{
	SHORTINT x, y, manSprite;
	if (Labirint_ManCanGo()) {
		x = (Labirint_manX + Labirint_manDirX) + Labirint_manDirX;
		y = (Labirint_manY + Labirint_manDirY) + Labirint_manDirY;
		Labirint_manPresent = Labirint_ManGoing(x, y);
		if (!Labirint_manPresent) {
			return;
		}
		manSprite = Labirint_GetManSprite();
		Labirint_SetCell(x, y, manSprite);
		App_PutSprite(Labirint_manX, Labirint_manY, 0);
		// Assembly fails for the next two lines.
		Labirint_manX = Labirint_manX + Labirint_manDirX;
		Labirint_manY = Labirint_manY + Labirint_manDirY;
	} else {
		manSprite = 4;
	}
	App_PutSprite(Labirint_manX, Labirint_manY, manSprite);
}

void
testBug2051348(void)
{
        ASSERT(1);
}

