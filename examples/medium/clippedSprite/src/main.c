//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <cpctelera.h>

/** Sprites includes */
#include "stones.h"
#include "chara.h"
#include "dungeon.h"
#include "wall.h"
#include "trap.h"

// Memory location definition
enum
{
	VIDEO_MEM,
	BUFFER_MEM,
	NB_BUFFERS
};

// Boolean definition
#define BOOL	u8
#define TRUE	1
#define FALSE	0

// Double buffer location
#define SCREEN_BUFF_LOC		0x8000
#define VMEM_SIZE			0x4000

// Mask table location
#define MASK_TABLE_LOC		(SCREEN_BUFF_LOC - 0x100) // 0x100 = Size of Mask Table

// New stack location
#define NEW_STACK_LOC		(MASK_TABLE_LOC - 0x100) // 0x100 = Size of Stack

// Screen size
#define SCREEN_CY	200
#define SCREEN_CX	80

// View size
#define VIEW_CX		G_STONES_W
#define VIEW_CY		G_STONES_H

// Position ground
#define POS_GROUND_X (SCREEN_CX - G_STONES_W)/2
#define POS_GROUND_Y (SCREEN_CY - G_STONES_H)/2

// Position dungeon title
#define POS_TITLE_X (SCREEN_CX - G_DUNGEON_W)/2
#define POS_TITLE_Y 20

// Position wall
#define POS_WALL_X (SCREEN_CX - G_WALL_W)/2
#define POS_WALL_Y (POS_TITLE_Y - 5)

// Position trap init
#define POS_TRAP_X	(VIEW_CX / 2 + 4)
#define POS_TRAP_Y	(VIEW_CY / 2 + 8)

// Position character
#define POS_CARA_CX	G_CHARA_00_W
#define POS_CARA_CY	G_CHARA_00_H

#define POS_CHARA_X (SCREEN_CX - POS_CARA_CX)/2
#define POS_CHARA_Y (SCREEN_CY - POS_CARA_CY)/2

// Animations character index in tileset 
#define NB_MOVE		5
#define MOVE_TOP	0
#define MOVE_DOWN	(3*NB_MOVE)
#define MOVE_LEFT	(1*NB_MOVE)
#define MOVE_RIGHT	(2*NB_MOVE)

// Move definitions
enum
{
	NO_MOVE,
	TOP,
	DOWN,
	LEFT,
	RIGHT
};

// Declare mask table
cpctm_declareMaskTable(gMaskTable);
cpctm_createTransparentMaskTable(gMaskTable, MASK_TABLE_LOC, M0, 0);

u8 gVMem;				// Current video memory
u8 gStepAnim;			// Current step sprite animation

u8 gScrollx, gScrolly;	// Ground scroll position	
i8 gCharax, gCharay;	// Character position
i8 gChestx, gChesty; 	// Chest position

/*****************************************************/
/*													 */
/*	Flip double buffer								 */
/*												     */
/*****************************************************/
void FlipBuffer()
{
	cpct_waitVSYNC();

	/** Change memory page */
	if (gVMem == BUFFER_MEM)
	{
		cpct_setVideoMemoryPage(cpct_pageC0);
		gVMem = VIDEO_MEM;
	}
	else
	{
		cpct_setVideoMemoryPage(cpct_page80);
		gVMem = BUFFER_MEM;
	}
}

/*****************************************************/
/*													 */
/*	Get back buffer video pointer					 */
/*												     */
/*****************************************************/
u8* GetBackScreenPtr(u8 xPos, u8 yPos)
{
	return cpct_getScreenPtr(gVMem != VIDEO_MEM ? (u8*)CPCT_VMEM_START : (u8*)SCREEN_BUFF_LOC, xPos, yPos);
}

/*****************************************************/
/*													 */
/*	Draw title on both buffer						 */
/*												     */
/*****************************************************/
void DrawTitle()
{
	/** Keep localy track of width of text drawn */
	static u8 titleCx = 0;
	u8* pvmem;

	/** Draw title background */
	pvmem = GetBackScreenPtr(POS_WALL_X, POS_WALL_Y);	
	cpct_drawSprite(g_wall, pvmem, G_WALL_W, G_WALL_H);
	
	/** Draw progressivly dungeon text */
	pvmem = GetBackScreenPtr(POS_TITLE_X, POS_TITLE_Y);	
	if (++titleCx < G_DUNGEON_W)
	{
		cpct_drawSpriteClippedMaskedAlignedTable(titleCx, g_dungeon, pvmem, G_DUNGEON_W, G_DUNGEON_H, gMaskTable);
	}
	/** All sprite text width is drawn then restart from 0 */
	else
	{
		cpct_drawSpriteMaskedAlignedTable(g_dungeon, pvmem, G_DUNGEON_W, G_DUNGEON_H, gMaskTable);
		titleCx = 0; 
	}
}

/*****************************************************/
/*													 */
/*	Draw character									 */
/*												     */
/*****************************************************/
void DrawCharacter(u8 move)
{
	u8* pvmem = GetBackScreenPtr(POS_CHARA_X, POS_CHARA_Y);	
	u8* moveSprite;
	
	/** Incremeent character step animation loop */
	gStepAnim = (gStepAnim + 1) % NB_MOVE;
	
	/** get current character sprite position according to direction and current step */
	switch(move)
	{
		case TOP :		moveSprite = g_tileset[MOVE_TOP + gStepAnim];
		break;
		case DOWN :		moveSprite = g_tileset[MOVE_DOWN + gStepAnim];
		break;
		case LEFT :		moveSprite = g_tileset[MOVE_LEFT + gStepAnim];
		break;
		case RIGHT :	moveSprite = g_tileset[MOVE_RIGHT + gStepAnim];
		break;
		
		default :		moveSprite = g_tileset[MOVE_TOP];
	}
	
	cpct_drawSpriteMaskedAlignedTable(moveSprite, pvmem, POS_CARA_CX, POS_CARA_CY, gMaskTable);
}

/*****************************************************/
/*													 */
/*	Draw clipped sprite								 */
/*												     */
/*****************************************************/
void DrawClippedSprite(u8 spriteWidthToDraw, u8* sprite, u8 x, u8 y, u8 cx, u8 cy)
{
	if (cx > 0 && cy > 0 && spriteWidthToDraw > 0)
	{
		u8* pvmem = GetBackScreenPtr(x, y);	
		cpct_drawSpriteClipped(spriteWidthToDraw, sprite, pvmem, cx, cy);
	}
}

/*****************************************************/
/*													 */
/*	Draw clipped sprite	masked						 */
/*												     */
/*****************************************************/
void DrawClippedSpriteMasked(u8 spriteWidthToDraw, u8* sprite, u8 x, u8 y, u8 cx, u8 cy)
{
	if (cx > 0 && cy > 0 && spriteWidthToDraw > 0)
	{
		u8* pvmem = GetBackScreenPtr(x, y);	
		cpct_drawSpriteClippedMasked(spriteWidthToDraw, sprite, pvmem, cx, cy);
	}
}

/*****************************************************/
/*													 */
/*	Draw top right sprite 							 */
/*												     */
/*****************************************************/
void DrawTopRightSprite(u8 viewY, u8 viewX, u8 x, u8 y, u8 spriteWidth, u8 spriteHeight, u8* sprite)
{
	u8 spriteY = viewY;								// Sprite is draw in top of view
	u8 spriteCY = spriteHeight - y;					// Sprite Height = visible part
	u8* spriteToDraw = sprite + spriteWidth * y;	// Change start vertically of sprite pointer
	
	u8 spriteX = viewX + x;							// Sprite is draw at right in view 					
	u8 spriteToDrawCX = spriteWidth - x;			// Draw visible width of sprite
	
	DrawClippedSprite(spriteToDrawCX, spriteToDraw, spriteX, spriteY, spriteWidth, spriteCY);
}

/*****************************************************/
/*													 */
/*	Draw top left sprite 							 */
/*												     */
/*****************************************************/
void DrawTopLeftSprite(u8 viewY, u8 viewX, u8 x, u8 y, u8 spriteWidth, u8 spriteHeight, u8* sprite)
{
	u8 spriteY = viewY;								// Sprite is draw in top of view
	u8 spriteCY = spriteHeight - y;					// Sprite Height = visible part
	u8* spriteToDraw = sprite + spriteWidth * y;	// Change start vertically of sprite pointer
	
	u8 spriteX = viewX;								// Sprite is draw in left part of view 
	u8 spriteToDrawCX = x;							// Draw visible width of sprite
	spriteToDraw += spriteWidth - x;				// Change start horizontally of sprite pointer
	
	DrawClippedSprite(spriteToDrawCX, spriteToDraw, spriteX, spriteY, spriteWidth, spriteCY);
}

/*****************************************************/
/*													 */
/*	Draw bottom left sprite 						 */
/*												     */
/*****************************************************/
void DrawBottomLeftSprite(u8 viewY, u8 viewX, u8 x, u8 y, u8 spriteWidth, u8 spriteHeight, u8* sprite)
{
	u8 spriteY = viewY + spriteHeight - y;			// Sprite is draw at bottom of view
	u8 spriteCY = y;								// Sprite Height = sprite position

	u8 spriteX = viewX;								// Sprite is draw in left part of view 
	u8 spriteToDrawCX = x;							// Draw visible width of sprite
	u8* spriteToDraw = sprite + (spriteWidth - x);	// Change start horizontally of sprite pointer
	
	DrawClippedSprite(spriteToDrawCX, spriteToDraw, spriteX, spriteY, spriteWidth, spriteCY);
}

/*****************************************************/
/*													 */
/*	Draw bottom right sprite 						 */
/*												     */
/*****************************************************/
void DrawBottomRightSprite(u8 viewY, u8 viewX, u8 x, u8 y, u8 spriteWidth, u8 spriteHeight, u8* sprite)
{
	u8 spriteY = viewY + spriteHeight - y;			// Sprite is draw at bottom of view
	u8 spriteCY = y;								// Sprite Height = sprite position
	
	u8 spriteX = viewX  + x;						// Sprite is draw at right in view 	
	u8 spriteToDrawCX = spriteWidth - x;			// Draw visible width of sprite
	
	DrawClippedSprite(spriteToDrawCX, sprite, spriteX, spriteY, spriteWidth, spriteCY);
}

/*****************************************************/
/*													 */
/*	Draw four parts of sprite 						 */
/*												     */
/*****************************************************/
void DrawSpriteParts(u8* spriteInput, u8 x, u8 y, u8 cx, u8 cy)
{
	DrawTopLeftSprite(POS_GROUND_Y, POS_GROUND_X, x, y, cx, cy, spriteInput);
	DrawTopRightSprite(POS_GROUND_Y, POS_GROUND_X, x, y, cx, cy, spriteInput);
	DrawBottomLeftSprite(POS_GROUND_Y, POS_GROUND_X, x, y, cx, cy, spriteInput);
	DrawBottomRightSprite(POS_GROUND_Y, POS_GROUND_X, x, y, cx, cy, spriteInput);
}

/*****************************************************/
/*													 */
/*	Draw trap										 */
/*												     */
/*****************************************************/
void DrawChest()
{
	/** Test if trap is horizontally in view */
	i8 posx = gChestx + gCharax;
	if (posx + G_TRAP_W > 0 && posx < VIEW_CX)
	{
		/** Test if trap is vertically in view */
		i8 posy = gChesty - gCharay;
		if (posy + G_TRAP_H > 0 && posy < VIEW_CY)
		{
			u8* sprite;			
			u8 spriteCX, spriteCY;
			u8 spriteX, spriteY;
			u8 spriteToDrawCX;

			/** Sprite outside view by top */
			if (posy < 0)
			{
				u8 y = -posy;
				
				spriteY = POS_GROUND_Y;
				spriteCY = G_TRAP_H - y;
				sprite = (u8*)g_trap + G_TRAP_W * y * 2; // Sprite masked : 2 bytes = color + mask		
			}
			else 
			/** Sprite outside view by down */
			if (posy + G_TRAP_H > VIEW_CY)
			{
				spriteY = POS_GROUND_Y + posy;
				spriteCY = VIEW_CY - posy;
				sprite = (u8*)g_trap;				
			}
			/** Sprite Height all in view */
			else
			{
				spriteCY = G_TRAP_H;
				spriteY = POS_GROUND_Y + posy;
				sprite = (u8*)g_trap;
			}
			
			/** Sprite outside view by left */
			if (posx < 0)
			{
				u8 x = posx + G_TRAP_W;
				
				spriteX = POS_GROUND_X;
				spriteCX = G_TRAP_W;
				sprite += (G_TRAP_W - x) * 2; // Sprite masked : 2 bytes = color + mask	
				spriteToDrawCX = x;				
			}
			else
			/** Sprite outside view by right */
			if (posx + G_TRAP_W > VIEW_CX)
			{
				spriteX = POS_GROUND_X + posx;
				spriteCX = G_TRAP_W;
				spriteToDrawCX = VIEW_CX - posx;
			}
			/** Sprite Width all in view */
			else
			{
				spriteCX = G_TRAP_W;
				spriteX = POS_GROUND_X + posx;
				spriteToDrawCX = G_TRAP_W;
			}
						
			DrawClippedSpriteMasked(spriteToDrawCX, sprite, spriteX, spriteY, spriteCX, spriteCY);						
		}
	}
}

/*****************************************************/
/*													 */
/*	Scroll sprite Y									 */
/*												     */
/*****************************************************/
void ScrollGroundY(u8 move)
{	
	/** Wrap ground to the down */
	if (gScrolly == 0 && move == DOWN)
		gScrolly = G_STONES_H - 2; // Offset of 2 because in mode 0 : pixel size W = 2 * H
	else
	/** Wrap ground to the top */
	if (gScrolly == G_STONES_H - 2 && move == TOP)
		gScrolly = 0;
	else
	{
		/** Compute current ground position */
		if (move == TOP)
			gScrolly += 2;
		else
			gScrolly -= 2;
	}

	/** Compute character in world position */
	if (move == TOP)
		gCharay += 2;
	else
		gCharay -= 2;
	
	/** Render ground of stones */
	DrawSpriteParts(g_stones, gScrollx, gScrolly, G_STONES_W, G_STONES_H);
}

/*****************************************************/
/*													 */
/*	Scroll sprite X									 */
/*												     */
/*****************************************************/
void ScrollGroundX(u8 move)
{
	/** Wrap ground to the right */
	if (gScrollx == 0 && move == RIGHT)
		gScrollx = G_STONES_W - 1;
	else
	/** Wrap ground to the left */
	if (gScrollx == G_STONES_W - 1 && move == LEFT)
		gScrollx = 0;
	else
	{
		/** Compute current ground position */
		if (move == LEFT)
			gScrollx++;
		else
			gScrollx--;
	}
	
	/** Compute character in world position */
	if (move == LEFT)
		gCharax++;
	else
		gCharax--;
	
	/** Render ground of stones  */
	DrawSpriteParts(g_stones, gScrollx, gScrolly, G_STONES_W, G_STONES_H);
}

/*****************************************************/
/*													 */
/*	Check input keys to change draw function		 */
/*												     */
/*****************************************************/
u8 CheckUserInput() 
{
	u8 move = NO_MOVE;
	cpct_scanKeyboard_f();
	
	if (cpct_isKeyPressed(Key_CursorUp))
	{
		move = DOWN;
	}
	else  
	if (cpct_isKeyPressed(Key_CursorDown))
	{
		move = TOP;
	}
	else  
	if (cpct_isKeyPressed(Key_CursorLeft))
	{
		move = LEFT;
	}
	else  
	if (cpct_isKeyPressed(Key_CursorRight))
	{
		move = RIGHT;
	}
	
	return move;
}

/*****************************************************/
/*													 */
/*	Initialization									 */
/*												     */
/*****************************************************/
void Init()
{
	u8* pvmem;

	cpct_disableFirmware();
	cpct_setVideoMode(0);
	
	/** Set colors */
	cpct_setBorder(0x54);
	cpct_setPalette(g_palette, sizeof(g_palette));
	
	/** Clear Screens */
	cpct_memset(CPCT_VMEM_START, 0, VMEM_SIZE);
	cpct_memset((u8*)SCREEN_BUFF_LOC, 0, VMEM_SIZE);
	
	/** Draw text background */
	pvmem = GetBackScreenPtr(POS_WALL_X, POS_WALL_Y);	
	cpct_drawSprite(g_wall, pvmem, G_WALL_W, G_WALL_H);
	
	/** Draw ground at initial position with no offset */
	pvmem = GetBackScreenPtr(POS_GROUND_X, POS_GROUND_Y);
	cpct_drawSprite(g_stones, pvmem, G_STONES_W, G_STONES_H);
	
	/** initialize positions */
	gScrollx = 0;
	gScrolly = 0;
	
	gChestx = POS_TRAP_X;
	gChesty = POS_TRAP_Y;
	
	gCharax = 0;
	gCharay = 0;
	
	gStepAnim = 0;
	
	/** Draw character and trap at initial position */
	DrawCharacter(TOP);
	DrawChest();
	
	/** Render first frame */
	FlipBuffer();
}

/*****************************************************/
/*													 */
/*	Draw main routine								 */
/*												     */
/*****************************************************/
void main(void) 
{
	/** Change stack location in order to use HW double buffer */
	cpct_setStackLocation((u8*)NEW_STACK_LOC);
	Init();
	
	while (TRUE)
	{
		u8 move = CheckUserInput();

		/** Move vertically */
		if (move == TOP || move == DOWN)
			ScrollGroundY(move);
		else 
		/** Move horizontally */
		if (move == LEFT || move == RIGHT)
			ScrollGroundX(move);	
		
		/** If move then update render elements */
		if (move != NO_MOVE)
		{
			DrawChest();
			DrawCharacter(move);
			DrawTitle();
			
			FlipBuffer();			
		}
	}
}
