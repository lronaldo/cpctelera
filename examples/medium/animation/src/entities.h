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
   unsigned char* sprite;        // Sprite associated to this frame
   unsigned char  width, height; // Sprite dimensions in bytes
            char  mx, my;        // Pixel movements, in bytes, to be executed at the start of this Frame
   unsigned char  time;          // Time that the sprite should be shown
} TAnimFrame;

//
// Describes an Animation as a secuence of sprites, controlled by time
//   Time is measured in main loop cycles
//
typedef struct {
   TAnimFrame**      frames;     // Vector containing all the frames of the animation
   unsigned char   frame_id;     // Index of the current frame
   unsigned char       time;     // Remaining time for this frame
   TAnimStatus       status;     // Status of the animation
} TAnimation;

//
// Possible statuses of an entity
//
typedef enum {
   es_dead,       // Entity dead
   es_stop,       // Entity stopped, not moving
   es_walk_right, // Entity walking
   es_walk_left,  // Entity walking
   es_fist,       // Entity fisting
   es_kick,       // Entity kicking
   es_hit         // Entity is being hit
} TEntityStatus;

//
// Describes a game entity (typically, a character)
//
typedef struct {
   TAnimation        *anim;   // Animation currently associated with this entity
   unsigned char *videopos;   // Video memory location where entity will be drawn
   unsigned char      x, y;   // X, Y coordinates of entity in the screen
   TEntityStatus    status;   // Present status of the entity
} TEntity;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

unsigned char *getScreenPointer(unsigned char y);
TEntity* getPersea();
void updateEntity(TEntity *ent);
void setAnimation(TEntity *ent, TEntityStatus newstatus);
char moveEntityX (TEntity* ent, char mx);
char moveEntityY (TEntity* ent, char my);
void drawEntity  (TEntity* ent);
