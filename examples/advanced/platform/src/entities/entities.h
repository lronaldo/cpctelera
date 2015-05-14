//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

#include <types.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  DATA STRUCTURES
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// Possible statuses of an animation
//
typedef enum {
   as_play,    // Playing till the last frame
   as_cycle,   // Playing continuosly
   as_pause,   // Paused, waiting to continue
   as_end      // Animation has ended
} TAnimStatus;

//
// Description of an animation frame
//
typedef struct {
   u8* sprite;        // Sprite associated to this frame
   u8  width, height; // Sprite dimensions in bytes
   u8  time;          // Time that the sprite should be shown
} TAnimFrame;

//
// Describes an Animation as a secuence of sprites, controlled by time
//   Time is measured in main loop cycles
//
typedef struct {
   TAnimFrame**  frames;    // Vector containing all the frames of the animation
   u8            frame_id;  // Index of the current frame
   u8            time;      // Remaining time for this frame
   TAnimStatus   status;    // Status of the animation
} TAnimation;

//
// Possible statuses of an entity
//
typedef enum {
   es_static = 0, // Entity that does not move
   es_walk,       // Entity walking
   es_jump,       // Entity jumping
   es_hit,        // Entity is being hit
   es_NUMSTATUSES // Total amount of statuses available
} TEntityStatus;

//
// Entities can be heading both sides
//
typedef enum { s_left = 0, s_right, s_NUMSIDES } TEntitySide;

//
// Describes a game entity (typically, a character)
//
typedef struct {
   TAnimation    *anim;    // Animation currently associated with this entity
   u8            *pscreen;  // Pointer to Screen Video memory location where entity will be drawn
   u8             x,  y;    // X, Y coordinates of entity in the screen
   u8            nx, ny;    // Next X, Y coordinates of entity in the screen (for next frame)
   TEntityStatus status;    // Present status of the entity
   TEntitySide   side;      // Side the entity is facing
} TEntity;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

    void updateEntity(TEntity *ent);
    void setAnimation(TEntity *ent, TEntityStatus newstatus);
      i8 moveEntityX (TEntity* ent, i8 mx);
      i8 moveEntityY (TEntity* ent, i8 my);
    void drawEntity  (TEntity* ent);
