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

// Sprites
#include "sprites\back.h"
#include "sprites\ship.h"
#include "sprites\title.h"
#include "sprites\fire.h"

// Double buffer location
#define SCREEN_BUFF				0x8000

// Mask table location
#define MASK_TABLE_LOCATION		(SCREEN_BUFF - 0x100)

// New stack location
#define NEW_STACK_LOCATION      (MASK_TABLE_LOCATION - 0x100)

// Position definition
#define VIEW_X					(80-VIEW_CX)/2
#define VIEW_Y					0
#define VIEW_CX					(200/4) // Mode 1 : 4 pixels/byte
#define VIEW_CY					60
#define POS_TEXT				(VIEW_Y + VIEW_CY + 20)
#define POS_INFO				(VIEW_Y + VIEW_CY + 70)

#define POS_TITLE_X				(VIEW_CX - G_TITLE_W) / 2

#define POS_SHIP_Y				(VIEW_CY - G_SHIP_H) / 2 + 5
#define POS_SHIP_X				(VIEW_CX - G_SHIP_W/2) / 2

// Mem location definition
#define VIDEO_MEM				0
#define BUFFER_MEM				1

// Declare mask table
cpctm_declareMaskTable(gMaskTable);
cpctm_createTransparentMaskTable(gMaskTable, MASK_TABLE_LOCATION, M1, 0);

// Variables
u8 gNbTileset; // Nb tileset of star background
u8 gPosScroll; // Pos scrolling of star backGround

u8 gBackBuffer[VIEW_CX*VIEW_CY]; // Back buffer
u8 gVMem; // Current video mem

/** Draw functions pointer */
typedef void (*TDrawFunc)(void);
TDrawFunc gDrawFunc; // Current drawing function

/*****************************************************/
/*													 */
/*	Flip double buffer								 */
/*												     */
/*****************************************************/
void FlipBuffer()
{
	cpct_waitVSYNC();

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
/*	Get buffer video pointer						 */
/*												     */
/*****************************************************/
u8* GetScreenPtr(u8 xPos, u8 yPos)
{
	return cpct_getScreenPtr(gVMem == VIDEO_MEM ? (u8*)CPCT_VMEM_START : (u8*)SCREEN_BUFF, xPos, yPos);
}

/*****************************************************/
/*													 */
/*	Get back buffer video pointer					 */
/*												     */
/*****************************************************/
u8* GetBackScreenPtr(u8 xPos, u8 yPos)
{
	return cpct_getScreenPtr(gVMem != VIDEO_MEM ? (u8*)CPCT_VMEM_START : (u8*)SCREEN_BUFF, xPos, yPos);
}

/*****************************************************/
/*													 */
/*	Copy back buffer to screen video mem			 */
/*												     */
/*****************************************************/
void CopyToScreen()
{
	cpct_waitVSYNC();
	cpct_drawSprite(gBackBuffer, GetScreenPtr(VIEW_X, VIEW_Y), VIEW_CX, VIEW_CY);
}

/*****************************************************/
/*													 */
/*	Draw sprite directly in video memory			 */
/*												     */
/*****************************************************/
void DrawSprite()
{
	u8 i;
	
	cpct_waitVSYNC();
	
	for(i = 0; i < VIEW_CX; i++)
		cpct_drawSprite(g_tileset[(gPosScroll + i)%gNbTileset], GetScreenPtr(VIEW_X + i, VIEW_Y), G_BACK_00_W, G_BACK_00_H);
		
	cpct_drawSpriteMaskedAlignedTable(g_title, GetScreenPtr(VIEW_X + POS_TITLE_X, 0), G_TITLE_W, G_TITLE_H, gMaskTable);
	
	cpct_drawSpriteMaskedAlignedTable((gPosScroll % 2) == 0 ? g_fire_0 : g_fire_1, GetScreenPtr(VIEW_X + POS_SHIP_X - G_FIRE_0_W, VIEW_Y + POS_SHIP_Y + 2), G_FIRE_0_W, G_FIRE_0_H, gMaskTable);
	cpct_drawSpriteMasked(g_ship, GetScreenPtr(VIEW_X + POS_SHIP_X, VIEW_Y + POS_SHIP_Y), G_SHIP_W, G_SHIP_H);
}

/*****************************************************/
/*													 */
/*	Draw sprite Hardware double buffer   			 */
/*												     */
/*****************************************************/
void DrawHWDoubleBuffer()
{
	u8 i;
	for(i = 0; i < VIEW_CX; i++)
		cpct_drawSprite(g_tileset[(gPosScroll + i)%gNbTileset], GetBackScreenPtr(VIEW_X + i, VIEW_Y), G_BACK_00_W, G_BACK_00_H);
		
	cpct_drawSpriteMaskedAlignedTable(g_title, GetBackScreenPtr(VIEW_X + POS_TITLE_X, 0), G_TITLE_W, G_TITLE_H, gMaskTable);
	
	cpct_drawSpriteMaskedAlignedTable((gPosScroll % 2) == 0 ? g_fire_0 : g_fire_1, GetBackScreenPtr(VIEW_X + POS_SHIP_X - G_FIRE_0_W, VIEW_Y + POS_SHIP_Y + 2), G_FIRE_0_W, G_FIRE_0_H, gMaskTable);
	cpct_drawSpriteMasked(g_ship, GetBackScreenPtr(VIEW_X + POS_SHIP_X, VIEW_Y + POS_SHIP_Y), G_SHIP_W, G_SHIP_H);
	
	FlipBuffer();
}

/*****************************************************/
/*													 */
/*	Draw sprite Software double buffer   			 */
/*												     */
/*****************************************************/
void DrawSWDoubleBuffer()
{
	u8 i;
	for(i = 0; i < VIEW_CX; i++)
		cpct_drawToSpriteBuffer(VIEW_CX, gBackBuffer + i, G_BACK_00_H, G_BACK_00_W, g_tileset[(gPosScroll + i)%gNbTileset]);

	cpct_drawToSpriteBufferMaskedAlignedTable(VIEW_CX, gBackBuffer + POS_TITLE_X, G_TITLE_H, G_TITLE_W, g_title, gMaskTable);
	
	cpct_drawToSpriteBufferMaskedAlignedTable(VIEW_CX, cpctm_backBufferPtr(gBackBuffer, VIEW_CX, POS_SHIP_X - G_FIRE_0_W, POS_SHIP_Y + 2), G_FIRE_0_H, G_FIRE_0_W, (gPosScroll % 2) == 0 ? g_fire_0 : g_fire_1, gMaskTable);
	cpct_drawToSpriteBufferMasked(VIEW_CX, cpctm_backBufferPtr(gBackBuffer, VIEW_CX, POS_SHIP_X, POS_SHIP_Y), G_SHIP_H, G_SHIP_W, g_ship);
		
	CopyToScreen();
}

/*****************************************************/
/*													 */
/*	Draw current selection							 */
/*												     */
/*****************************************************/
void DrawSelection(u8 sel)
{
	u8 pos;
	
	switch(sel)
	{
		case 1 : pos = POS_TEXT + 15; break;
		case 2 : pos = POS_TEXT + 40; break;
		case 3 : pos = POS_TEXT + 75; break;
		default : return;
	}
	
	cpct_drawSolidBox(cpct_getScreenPtr((u8*)CPCT_VMEM_START, 0, POS_TEXT + 15), 0, 2, 80);
	cpct_drawSolidBox(cpct_getScreenPtr((u8*)SCREEN_BUFF, 0, POS_TEXT + 15), 0, 2, 80);
	
	cpct_drawStringM1(">", cpct_getScreenPtr((u8*)CPCT_VMEM_START, 0, pos), 3, 0);  
	cpct_drawStringM1(">", cpct_getScreenPtr((u8*)SCREEN_BUFF, 0, pos), 3, 0);  
}

/*****************************************************/
/*													 */
/*	Check input keys to change draw function		 */
/*												     */
/*****************************************************/
void CheckUserInput() 
{
	u8 sel = 0;
	cpct_scanKeyboard_f();
	
	if (cpct_isKeyPressed(Key_1))
	{
		sel = 1;
		gDrawFunc = DrawSprite;
	}
	else  
	if (cpct_isKeyPressed(Key_2))
	{
		sel = 2;
		gDrawFunc = DrawHWDoubleBuffer;
	}
	else  
	if (cpct_isKeyPressed(Key_3))
	{
		sel = 3;
		gDrawFunc = DrawSWDoubleBuffer;
	}
	
	DrawSelection(sel);
}

/*****************************************************/
/*													 */
/*	Draw text in both buffer						 */
/*												     */
/*****************************************************/
void DrawInfoText()
{
	u8* mem;
	
	u8 i;
	for (i = 0; i < 2; i++)
	{
		mem = (i == 0) ? (u8*)CPCT_VMEM_START : (u8*)SCREEN_BUFF;

		cpct_drawStringM1("Press:", cpct_getScreenPtr(mem, 0, POS_TEXT - 5), 3, 0);  
		
		cpct_drawStringM1("1", cpct_getScreenPtr(mem, 4, POS_TEXT + 15), 1, 0);  
		cpct_drawStringM1(": No double buffer", cpct_getScreenPtr(mem, 8, POS_TEXT + 15), 2, 0);  
		
		cpct_drawStringM1("Directly draw in video mem", cpct_getScreenPtr(mem, 8, POS_TEXT + 25), 1, 0);  
		
		cpct_drawStringM1("2", cpct_getScreenPtr(mem, 4, POS_TEXT + 40), 1, 0);  
		cpct_drawStringM1(": Hardware double buffer", cpct_getScreenPtr(mem, 8,  POS_TEXT + 40), 2, 0);  
		
		cpct_drawStringM1("Draw alternatively in two video mem  (2*16384 bytes) and flip between them", cpct_getScreenPtr(mem, 8, POS_TEXT + 50), 1, 0);  
		
		cpct_drawStringM1("3", cpct_getScreenPtr(mem, 4, POS_TEXT + 75), 1, 0);  
		cpct_drawStringM1(": Software double buffer", cpct_getScreenPtr(mem, 8, POS_TEXT + 75), 2, 0);  
		
		cpct_drawStringM1("Draw in buffer (50*60 bytes) of size view and copy whole buffer to video mem", cpct_getScreenPtr(mem, 8, POS_TEXT + 85), 1, 0);  
	}
}

/*****************************************************/
/*													 */
/*	Initialization									 */
/*												     */
/*****************************************************/
void Init()
{
	cpct_disableFirmware();
	cpct_memset((u8*)SCREEN_BUFF, 0, 0x4000);
	
	cpct_setPalette(g_palette, 5);	
	gVMem = VIDEO_MEM;
	
	gNbTileset = sizeof(g_tileset)/sizeof(u8*);
	gPosScroll = 0;
	
	gDrawFunc = DrawSprite;
	DrawSelection(1);
	
	DrawInfoText();
}

/*****************************************************/
/*													 */
/*	Draw main routine								 */
/*												     */
/*****************************************************/
void main(void) 
{
	/** Change stack location in order to use HW double buffer */
	cpct_setStackLocation((u8*)NEW_STACK_LOCATION);
	Init();
	
	while(1)
	{
		CheckUserInput();
		
		// Scroll background, draw title, draw ship
		gPosScroll++;
		gDrawFunc();
	}
}