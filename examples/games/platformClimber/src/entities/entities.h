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

#ifndef _ENTITIES_H_
#define _ENTITIES_H_

#include <types.h>
#include "../anim/animation.h"

// Scale value for fixed point maths calculations using integers
#define SCALE   256  // 2^8

// Expected Frames per Second drawing Ratio   
#define FPS      50

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  DATA STRUCTURES
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Possible statuses of an entity
//
typedef enum {
   es_static = 0, // Entity that does not move
   es_walk,       // Entity walking
   es_jump,       // Entity jumping
   es_moveFloor,  // Entity is moving a floor
   es_NUMSTATUSES // Total amount of statuses available
} TCharacterStatus;

//
// Entities can be heading both sides
//
typedef enum { s_left = 0, s_right, s_NUMSIDES } TCharacterSide;

//
// Describes physical behaviour for an object
//
struct Entity;
typedef struct {
   u16   x,  y;    // X, Y coordinates of entity in a subpixel world (in pixels*SCALE)
   i16  vx, vy;    // Velocity vector controlling entity movement (In pixels*SCALE)
   u16  bounce;    // Bounce coefficient (In pixels*SCALE. < SCALE absorves energy, > SCALE gives energy)
   struct Entity* floor; // Entity that acts as floor
} TPhysics;

//
// Information for solid objects that occupy a rectangular space in the screen
//
typedef struct {
   u8   w, h;   // Width and height in bytes
   u8 colour;   // Colour pattern use for drawing
} TBlock;

//
// Describes a game entity
//
typedef struct Entity {
   // Entities have an animation or a solid rectangular block 
   union {
      TAnimation  anim;    // Animation currently associated with this entity
      TBlock      block;   // Definition of a rectangular block in the screen
   } graph;

   u8        *pscreen;  // Pointer to Screen Video memory location where entity will be drawn
   u8       *npscreen;  // Pointer the next Screen Video memory location where entity will be drawn
   u8           x,  y;  // X, Y coordinates of entity in the screen (in bytes)
   u8          nx, ny;  // Next X, Y coordinates of entity in the screen (in bytes)
   u8          pw, ph;  // Previous Width and height of the entity (depending on animation). Used to erase it
   TPhysics      phys;  // Values for entities that have Physical components

   u8            draw;  // Flag to be set when the entity needs to be drawn again

   // Used to change animation on next timestep
   TAnimFrame  **nAnim; // Pointer to next animation frames (null when no next animation)
   TAnimStatus nStatus; // Next animation status
} TEntity;

//
// Describes a game character (the main character, for instance)
//
typedef struct Character {
   TEntity           entity;  // Entity model for this character
   TCharacterStatus  status;  // Present status of the character
   TCharacterSide    side;    // Side the character is facing
} TCharacter;

//
// Information about a collision rectangle 
//
typedef struct {
   u8    x, y;    // Screen coordinates of the top-left square of the collision
   u8    w, h;    // Width and height in bytes
} TCollision;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  PUBLIC UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    void initializeEntities();
    void performAction(TCharacter *c, TCharacterStatus move, TCharacterSide side);
      u8 updateCharacter(TCharacter *c);
    void scrollWorld();
     u16 getScore();
    void drawAll();
TCharacter* getCharacter();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY GLOBALS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Size of the Screen and base pointer (in pixels)
//
extern const u8  g_SCR_WIDTH;  // Screen width in bytes (80 bytes = 160 pixels)
extern const u8  g_SCR_HEIGHT; // Screen height in bytes
extern u8* const g_SCR_VMEM;   // Pointer to the start of default video memory screen

#endif