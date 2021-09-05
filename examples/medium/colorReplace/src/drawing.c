
//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2018 Bouche Arnaud (@Arnaud6128)
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

#include "declarations.h"

/////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
u8 gSpriteColorized[G_BALOON_W*G_BALOON_H]; // Array for sprite to color
u8 gBackGroundColor;                        // Background color
u8 gBaloonColor;                            // Current color baloon
u8 gPosCloud;                               // Position of cloud

/////////////////////////////////////////////////////////////////////////////////
// STRUCTURES DEFINITION
typedef struct TBaloon                      // Baloon structure
{
    i16 posY;                               // Absolute Baloon PosY (can be outside screen)
    u8 posX;                                // Absolute Baloon PosX
    
    u8 drawPosY;                            // In screen Baloon PosY
    u8 drawCY;                              // In screen Baloon Height
    
    u8 speed;                               // Baloon speed
    u8 color;                               // Baloon color drawn
    
    u8 status;                              // Baloon status ACTIVE / INACTIVE
} SBaloon;

typedef struct TBaloons                     // All baloons structure
{
    u8 nb;                                  // Nb baloons in screen
    SBaloon baloons[NB_BALOONS];            // Array of baloon
} SBaloons;

SBaloons gBaloons;                          // Baloons to draw

typedef struct TStar                        // Lighting star structure
{
    u8 posX;                                // Screen PosX
    u8 posY;                                // Screen PosY

    u8 color;                               // Star color drawn
} SStar;

typedef struct TStars                       // All stars structure
{
    SStar stars[NB_STARS];                  // Array of stars
} SStars;

SStars gStars;                              // Stars to draw

///////////////////////////////////////////////////////
///    GET RANDOM FROM 0 TO MAX-1
///
u8 GetRand(u8 max)
{
    return cpct_rand()%max;
}

///////////////////////////////////////////////////////
///    CHANGE TWO COLORS OF BALOON SPRITE
///
u8* ColorSprite(u8 color)
{
    // Set pixel pattern pair for colors 1 and 2
    u16 replacePatColor1 = CPCTM_PENS2PIXELPATTERNPAIR_M0(1, color); // Just for example use cpct_pens2pixelPatternPairM0 with variables
    u16 replacePatColor2 = cpct_pens2pixelPatternPairM0(2, color + 1);
    
    // Create a copy of the original g_baloon sprite before changing it
    cpct_memcpy(gSpriteColorized, g_baloon, G_BALOON_W*G_BALOON_H);

    // Replace the two colors 1 and 2 of sprite baloon
    cpct_spriteColourizeM0(replacePatColor1, G_BALOON_W*G_BALOON_H, gSpriteColorized); // Colors are consecutives
    cpct_spriteColourizeM0(replacePatColor2, G_BALOON_W*G_BALOON_H, gSpriteColorized);
	
	return gSpriteColorized;
}

///////////////////////////////////////////////////////
///    CLEAR BALOON BACKGROUND
///
void ClearBaloon(SBaloon* baloon)
{
    // If baloon in visible part of view
    if (baloon->drawPosY < VIEW_DOWN)
    {
        u8* pvmem; 
        
        // Compute size to be filled with background color
        u8 clearCY = baloon->drawCY + BALOON_TRAIL;
        
        // Compute position to clear
        u8 posDownClearY = baloon->drawPosY + clearCY;
        if (posDownClearY > VIEW_DOWN)
            clearCY = VIEW_DOWN - baloon->drawPosY;
        
        // Draw the box with background color to clear
        pvmem = GetBackBufferPtr(baloon->posX, baloon->drawPosY);
        cpct_drawSolidBox(pvmem, gBackGroundColor, G_BALOON_W, clearCY);    
    }
}

///////////////////////////////////////////////////////
///    DELETE BALOON
///
void DeleteBaloons(SBaloon* baloons, SBaloon* baloonToDel, u8* nb)
{              
    // Get last baloon in array
    // Decrement number of remaining baloons in array
    const SBaloon* lastBaloon = &baloons[--*nb];
    
    // Replace baloon to be deleted with last baloon (if they are not the same)
    if (baloonToDel != lastBaloon)
        cpct_memcpy(baloonToDel, lastBaloon, sizeof(SBaloon));
}

///////////////////////////////////////////////////////
/// UPDATE BALOONS
///
void UpdateBaloons()
{
    SBaloon* itBaloon = gBaloons.baloons;
    u8 i;
    
    // Test if we can add a new baloon
    if (gBaloons.nb < NB_BALOONS)
    {
        // Add new baloon at end of array
        // Increment number of baloons in array
        SBaloon* newBaloon = &gBaloons.baloons[gBaloons.nb++];
    
        // Get random positions X and Y
        newBaloon->posX = GetRand(SCREEN_CX - G_BALOON_W);
        newBaloon->posY = SCREEN_CY - GetRand(40);
        
        // Get random speed
        newBaloon->speed = GetRand(3) + 2;
        
        // Get circular next color 2 by 2 until 12
        gBaloonColor = (gBaloonColor + 2) % 12;
        newBaloon->color = gBaloonColor + 1;
        
        // Set baloon ACTIVE
        newBaloon->status = BALOON_ACTIVE;
    }
    
    // Update all baloons
    for (i = 0; i < gBaloons.nb; i++)
    {
        // If baloon active move and draw it
        if (itBaloon->status == BALOON_ACTIVE)
        {
            // Test if whole baloon outside view
            if (itBaloon->posY + G_BALOON_H < VIEW_TOP)
            {
                // Set baloon inactive
                itBaloon->status = BALOON_INACTIVE;
                // Clear baloon background
                ClearBaloon(itBaloon);
            }
            else
            {
                // Move baloon to up according its speed
                i16 posY = itBaloon->posY - itBaloon->speed;
                itBaloon->posY = posY;

                // Baloon outside view by top
                if (posY < VIEW_TOP)
                {
                    itBaloon->drawPosY = 0;
                    itBaloon->drawCY = G_BALOON_H + posY;
                }
                else 
                //  Baloon outside view by down
                if (posY + G_BALOON_H > VIEW_DOWN)
                {
                    itBaloon->drawPosY = posY;
                    itBaloon->drawCY = VIEW_DOWN - posY;
                }
                // Baloon all in view
                else
                {
                    itBaloon->drawPosY = posY;
                    itBaloon->drawCY = G_BALOON_H;
                }    
            }
        }
        // If inactive delete baloon
        else
        {
            // Clear baloon background
            ClearBaloon(itBaloon);
            
            // Delete baloon from list
            DeleteBaloons(gBaloons.baloons, itBaloon, &gBaloons.nb);
        }
        
        // Get next baloon pointer
        itBaloon++;
    }
}

///////////////////////////////////////////////////////
/// DRAW BALOON
///
void DrawBaloon(SBaloon* baloon, u8* spriteBaloon)
{
    i16 posY = baloon->posY;
    
    // If baloon in view
    if (posY + G_BALOON_H > VIEW_TOP && posY < VIEW_DOWN)
    {
        // Get VMem pointer of current baloon position
        u8* pvmem = GetBackBufferPtr(baloon->posX, baloon->drawPosY);
        u8* sprite = (u8*)spriteBaloon;
        
        u16 replacePatColor1 = cpct_pens2pixelPatternPairM0(1, baloon->color);
    
        // Baloon partialy outside view by top
        if (posY < VIEW_TOP)
        {
            // Compute Y position
            u8 y = -posY;
            
            // Compute sprite offset
            sprite = (u8*)spriteBaloon + G_BALOON_W * y;
        }
       
        cpct_drawSpriteMaskedAlignedColorizeM0(sprite, pvmem, G_BALOON_W, baloon->drawCY, gMaskTable, replacePatColor1);
    }
}

///////////////////////////////////////////////////////
/// DRAW STARS
///
void DrawStars()
{  
    // Colors of stars
    const static u8 sColorStar[NB_COLORS_STAR]  = { 2, 4, 7, 8, 10, 11, 12 };	

    // Color animation
    static u8 sColorAnim = 0;
    
    for (u8 i = 0; i < NB_STARS; i++)
    {
        // Get video pointer of each star to be drawn
        u8* pvmem = GetBackBufferPtr(SCREEN_CX / NB_STARS * i + 5, i + 175);

        // Color to replace animated
        u8 colorPaletteStar = (i + sColorAnim++) % NB_COLORS_STAR;

        // Set pixel pattern pair for color 15
        u16 replacePatColor = cpct_pens2pixelPatternPairM0(15, sColorStar[colorPaletteStar]);

        if ((i%3) == 0)
        {
            // Copy masked Sprite to temporary array
            cpct_memcpy(gSpriteColorized, g_circle_trans, G_CIRCLE_TRANS_W * G_CIRCLE_TRANS_H * 2);
            
            // Replace color in masked sprite
            cpct_spriteMaskedColourizeM0(replacePatColor, G_CIRCLE_TRANS_W * G_CIRCLE_TRANS_H, gSpriteColorized);

            // Draw masked sprite
            cpct_drawSpriteMasked(gSpriteColorized, pvmem, G_CIRCLE_TRANS_W, G_CIRCLE_TRANS_H);
        }
        else if ((i%3) == 1)
        {
            // Color and draw sprite
            cpct_drawSpriteColorizeM0(g_square, pvmem, G_SQUARE_W, G_SQUARE_H, replacePatColor);
        }
        else
        {
            // Color and draw masked sprite
            cpct_drawSpriteMaskedColorizeM0(g_star_trans, pvmem, G_STAR_TRANS_W, G_STAR_TRANS_H, replacePatColor);
        }
    }
}

///////////////////////////////////////////////////////
/// DRAW CLOUD
/// 
void DrawCloud()
{
    // Draw cloud at fixed place
    u8* pvmem = GetBackBufferPtr(0, POS_CLOUD_Y);
    cpct_drawSprite(g_cloud, pvmem, G_CLOUD_W, G_CLOUD_H);
}

///////////////////////////////////////////////////////
/// DRAW SCENE WITH ALL BALOONS
///
void DrawSceneBaloons()
{
    // Clear background for all baloons
    SBaloon* itBaloon = gBaloons.baloons; // Get first baloon pointer
    for (u8 i = 0; i < gBaloons.nb; i++)
    {
        ClearBaloon(itBaloon);
        itBaloon++;
    }
    
    // Draw sprite cloud
    DrawCloud();
    
    // Draw all baloons
    itBaloon = gBaloons.baloons; // Get first baloon pointer
    for (u8 i = 0; i < gBaloons.nb; i++)
    {
        // Test if sprite have colors to change 
        if (itBaloon->color > 1) // Color 0 and 1 are default color baloon
        {        
            u8* sprite = ColorSprite(itBaloon->color); // Change colors of baloon
			DrawBaloon(itBaloon, sprite);              // And draw colored baloon
        }
		else
			 DrawBaloon(itBaloon, g_baloon);           // Draw default baloon sprite (blue)
    
        itBaloon++; // Get next baloon
    }
}

///////////////////////////////////////////////////////
/// DRAW BACKGROUND BLUE SKY AND ROOF
/// 
void DrawBackground()
{
    u8* pvmem;
    
    // Fill video buffer with background color
    cpct_memset((u8*)SCREEN_BUFF, gBackGroundColor, VMEM_SIZE);
    
    // Draw left part of Roof
    pvmem = GetBackBufferPtr(0, SCREEN_CY - G_ROOF_H);
    cpct_drawSprite(g_roof, pvmem, G_ROOF_W, G_ROOF_H);
    
    // Draw right part of Roof
    pvmem += G_ROOF_W;
    cpct_drawSprite(g_roof, pvmem, G_ROOF_W, G_ROOF_H);
    
    // Copy Same background on both buffers 
    cpct_memcpy(CPCT_VMEM_START, (u8*)SCREEN_BUFF, VMEM_SIZE);
}

/////////////////////////////////////////////////////////////////////////////////
/// INITIALIZE DRAWING
/// 
/// Initializes global variables and general status for drawing functions
/// 
void InitializeDrawing()
{
    gBackGroundColor = cpctm_px2byteM0(14, 14);             // Get byte color of background for M0
    gBaloons.nb = 0;                                        // No baloon to draw at start
    DrawBackground();                                       // Set background on both buffers
}
