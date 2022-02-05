
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
u8 gSpriteColorized[G_BALLOON_W*G_BALLOON_H];   // Array for sprite to color
u8 gBackGroundColor;                            // Background color
u8 gBalloonColor;                                // Current color balloon
u8 gPosCloud;                                   // Position of cloud

/////////////////////////////////////////////////////////////////////////////////
// STRUCTURES DEFINITION
typedef struct TBalloon                     // Balloon structure
{
    i16 posY;                               // Absolute Balloon PosY (can be outside screen)
    u8 posX;                                // Absolute Balloon PosX
    
    u8 drawPosY;                            // In screen Balloon PosY
    u8 drawCY;                              // In screen Balloon Height
    
    u8 speed;                               // Balloon speed
    u8 color;                               // Balloon color drawn
    
    u8 status;                              // Balloon status ACTIVE / INACTIVE
} SBalloon;

typedef struct TBalloons                    // All balloons structure
{
    u8 nb;                                  // Nb balloons in screen
    SBalloon balloons[NB_BALLOONS];         // Array of balloon
} SBalloons;

SBalloons gBalloons;                        // Balloons to draw

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
///    CHANGE TWO COLORS OF BALLOON SPRITE
///
u8* ColorSprite(u8 color)
{
    // Set pixel pattern pair for colors 1 and 2
    u16 replacePatColor1 = CPCTM_PENS2PIXELPATTERNPAIR_M0(1, color); // Just for example use cpct_pens2pixelPatternPairM0 with variables
    u16 replacePatColor2 = cpct_pens2pixelPatternPairM0(2, color + 1);
    
    // Create a copy of the original g_balloon sprite before changing it
    cpct_memcpy(gSpriteColorized, g_balloon, G_BALLOON_W*G_BALLOON_H);

    // Replace the two colors 1 and 2 of sprite balloon
    cpct_spriteColourizeM0(replacePatColor1, G_BALLOON_W*G_BALLOON_H, gSpriteColorized); // Colors are consecutives
    cpct_spriteColourizeM0(replacePatColor2, G_BALLOON_W*G_BALLOON_H, gSpriteColorized);
	
	return gSpriteColorized;
}

///////////////////////////////////////////////////////
///    CLEAR BALLOON BACKGROUND
///
void ClearBalloon(SBalloon* balloon)
{
    // If balloon in visible part of view
    if (balloon->drawPosY < VIEW_DOWN)
    {
        u8* pvmem; 
        
        // Compute size to be filled with background color
        u8 clearCY = balloon->drawCY + BALLOON_TRAIL;
        
        // Compute position to clear
        u8 posDownClearY = balloon->drawPosY + clearCY;
        if (posDownClearY > VIEW_DOWN)
            clearCY = VIEW_DOWN - balloon->drawPosY;
        
        // Draw the box with background color to clear
        pvmem = GetBackBufferPtr(balloon->posX, balloon->drawPosY);
        cpct_drawSolidBox(pvmem, gBackGroundColor, G_BALLOON_W, clearCY);    
    }
}

///////////////////////////////////////////////////////
///    DELETE BALLOON
///
void DeleteBalloons(SBalloon* balloons, SBalloon* balloonToDel, u8* nb)
{              
    // Get last balloon in array
    // Decrement number of remaining balloons in array
    const SBalloon* lastBalloon = &balloons[--*nb];
    
    // Replace balloon to be deleted with last balloon (if they are not the same)
    if (balloonToDel != lastBalloon)
        cpct_memcpy(balloonToDel, lastBalloon, sizeof(SBalloon));
}

///////////////////////////////////////////////////////
/// UPDATE BALLOONS
///
void UpdateBalloons()
{
    SBalloon* itBalloon = gBalloons.balloons;
    u8 i;
    
    // Test if we can add a new balloon
    if (gBalloons.nb < NB_BALLOONS)
    {
        // Add new balloon at end of array
        // Increment number of balloons in array
        SBalloon* newBalloon = &gBalloons.balloons[gBalloons.nb++];
    
        // Get random positions X and Y
        newBalloon->posX = GetRand(SCREEN_CX - G_BALLOON_W);
        newBalloon->posY = SCREEN_CY - GetRand(40);
        
        // Get random speed
        newBalloon->speed = GetRand(3) + 2;
        
        // Get circular next color 2 by 2 until 12
        gBalloonColor = (gBalloonColor + 2) % 12;
        newBalloon->color = gBalloonColor + 1;
        
        // Set balloon ACTIVE
        newBalloon->status = BALLOON_ACTIVE;
    }
    
    // Update all balloons
    for (i = 0; i < gBalloons.nb; i++)
    {
        // If balloon active move and draw it
        if (itBalloon->status == BALLOON_ACTIVE)
        {
            // Test if whole balloon outside view
            if (itBalloon->posY + G_BALLOON_H < VIEW_TOP)
            {
                // Set balloon inactive
                itBalloon->status = BALLOON_INACTIVE;
                // Clear balloon background
                ClearBalloon(itBalloon);
            }
            else
            {
                // Move balloon to up according its speed
                i16 posY = itBalloon->posY - itBalloon->speed;
                itBalloon->posY = posY;

                // Balloon outside view by top
                if (posY < VIEW_TOP)
                {
                    itBalloon->drawPosY = 0;
                    itBalloon->drawCY = G_BALLOON_H + posY;
                }
                else 
                //  Balloon outside view by down
                if (posY + G_BALLOON_H > VIEW_DOWN)
                {
                    itBalloon->drawPosY = posY;
                    itBalloon->drawCY = VIEW_DOWN - posY;
                }
                // Balloon all in view
                else
                {
                    itBalloon->drawPosY = posY;
                    itBalloon->drawCY = G_BALLOON_H;
                }    
            }
        }
        // If inactive delete balloon
        else
        {
            // Clear balloon background
            ClearBalloon(itBalloon);
            
            // Delete balloon from list
            DeleteBalloons(gBalloons.balloons, itBalloon, &gBalloons.nb);
        }
        
        // Get next balloon pointer
        itBalloon++;
    }
}

///////////////////////////////////////////////////////
/// DRAW BALLOON
///
void DrawBalloon(SBalloon* balloon, u8* spriteBalloon)
{
    i16 posY = balloon->posY;
    
    // If balloon in view
    if (posY + G_BALLOON_H > VIEW_TOP && posY < VIEW_DOWN)
    {
        // Get VMem pointer of current balloon position
        u8* pvmem = GetBackBufferPtr(balloon->posX, balloon->drawPosY);
        u8* sprite = (u8*)spriteBalloon;
        
        u16 replacePatColor1 = cpct_pens2pixelPatternPairM0(1, balloon->color);
    
        // Balloon partialy outside view by top
        if (posY < VIEW_TOP)
        {
            // Compute Y position
            u8 y = -posY;
            
            // Compute sprite offset
            sprite = (u8*)spriteBalloon + G_BALLOON_W * y;
        }
       
        cpct_drawSpriteMaskedAlignedColorizeM0(sprite, pvmem, G_BALLOON_W, balloon->drawCY, gMaskTable, replacePatColor1);
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
/// DRAW SCENE WITH ALL BALLOONS
///
void DrawSceneBalloons()
{
    // Clear background for all balloons
    SBalloon* itBalloon = gBalloons.balloons; // Get first balloon pointer
    for (u8 i = 0; i < gBalloons.nb; i++)
    {
        ClearBalloon(itBalloon);
        itBalloon++;
    }
    
    // Draw sprite cloud
    DrawCloud();
    
    // Draw all balloons
    itBalloon = gBalloons.balloons; // Get first balloon pointer
    for (u8 i = 0; i < gBalloons.nb; i++)
    {
        // Test if sprite have colors to change 
        if (itBalloon->color > 1) // Color 0 and 1 are default color balloon
        {        
            u8* sprite = ColorSprite(itBalloon->color); // Change colors of balloon
			DrawBalloon(itBalloon, sprite);              // And draw colored balloon
        }
		else
			 DrawBalloon(itBalloon, g_balloon);           // Draw default balloon sprite (blue)
    
        itBalloon++; // Get next balloon
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
    gBalloons.nb = 0;                                       // No balloon to draw at start
    DrawBackground();                                       // Set background on both buffers
}
