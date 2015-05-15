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

#include <cpctelera.h>
#include "entities.h"
#include "sprites.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  PREDEFINED ANIMATIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// All the animation frames to be used in this example
//
const TAnimFrame g_allAnimFrames[14] = {
   { G_EMRright,       4, 16,  2 }, // 0// << Walk Right Frames
   { G_EMRright2,      3, 16,  2 }, // 1// |
   { G_EMRleft,        4, 16,  2 }, // 2// << Walk Left Frames
   { G_EMRleft2,       3, 16,  2 }, // 3// |
   { G_EMRjumpright1,  4,  8,  2 }, // 4// << Jump Right Frames 
   { G_EMRjumpright2,  4,  8,  2 }, // 5// |
   { G_EMRjumpright3,  4,  8,  2 }, // 6// |
   { G_EMRjumpright4,  4,  8,  2 }, // 7// |
   { G_EMRjumpleft1,   4,  8,  2 }, // 8// << Jump Left Frames 
   { G_EMRjumpleft2,   4,  8,  2 }, // 9// |
   { G_EMRjumpleft3,   4,  8,  2 }, //10// |
   { G_EMRjumpleft4,   4,  8,  2 }, //11// |
   { G_EMRhitright,    4, 16,  2 }, //12// << Hit Right Frame
   { G_EMRhitleft,     4, 16,  2 }  //13// << Hit Left Frame
};

// Use a define for convenience
#define AF g_allAnimFrames

//
// All complete animations used in this example (NULL terminated, to know the end of the array)
//
TAnimFrame*  const g_walkLeft[3]  = { &AF[3], &AF[2], 0 };
TAnimFrame*  const g_walkRight[3] = { &AF[1], &AF[0], 0 };
TAnimFrame*  const g_jumpLeft[5]  = { &AF[8], &AF[9], &AF[10], &AF[11], 0 };
TAnimFrame*  const g_jumpRight[5] = { &AF[8], &AF[9], &AF[10], &AF[11], 0 };
TAnimFrame*  const g_hitLeft[2]   = { &AF[13], 0 };
TAnimFrame*  const g_hitRight[2]  = { &AF[12], 0 };

//
// Vector containing references to all animations, ordered by states and sides
//
TAnimFrame** const g_anim[es_NUMSTATUSES][s_NUMSIDES] = {
   {  0, 0  },                   // STATE 0 = es_static: No animations
   { g_walkLeft, g_walkRight },  // STATE 1 = es_walk
   { g_jumpLeft, g_jumpRight },  // STATE 2 = es_jump
   { g_hitLeft,  g_hitRight  }   // STATE 3 = es_hit
};

#undef AF

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  PHYSICS CONSTANTS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Gravity constants (in fixed decimal, pixels/frame)
// Assuming 1 px = 1 meter
const i16 G_gy     = 9.81 * SCALE / FPS;  // Defining gravity as 9.81 px/sec^2
const i16 G_gx     = 0;                   // No gravity on x axis, at the start
const i16 G_maxVel = 100 / FPS * SCALE;   // Maximum velocity for an entity, 100 px/sec

// Size of the Screen and base pointer (in pixels)
//
const u8  g_SCR_WIDTH  =  80;         // Screen width in bytes (80 bytes = 160 pixels)
const u8  g_SCR_HEIGHT = 200;         // Screen height in bytes
u8* const g_SCR_VMEM   = (u8*)0xC000; // Pointer to the start of default video memory screen

// Define entities in the world and main character
//
#define g_MaxBlocks 10          // Maximum number of blocks at the same time
TEntity g_blocks[g_MaxBlocks];  // Vector with the values of the blocks
     u8 g_lastBlock;            // Last block value (next available)

// Main Character
const TCharacter g_Character = {
   // Entity values
   { 
     { .anim = { g_walkRight, 0, 0, as_pause } }, // Initial animation
      (u8*)0xC000,
      0, 0, 0, 0, 
     { 0, 0, 0, 0, 0 }
   },
   es_walk,    // Walking 
   s_right     // To the right
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize entities
//   Sets up entities at their initial values
void initializeEntities() {
   TPhysics *p = ((TPhysics*)&g_Character.entity.phys); 
   g_lastBlock = 0;
   p->floor = newSolidBlock(20, 190, 40, 5, 0xFF);
   setEntityLocation(&g_Character.entity, 38, 174, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Updates an animation
//   Returns 1 when a new frame is reached, and 0 otherwise
//
i8 updateAnimation(TAnimation* anim) {
   i8 newframe = 0;

   // Update only if animation is not paused or finished
   if (anim->status != as_pause && anim->status != as_end) {

      // Update time and, If time has finished for this frame, get next
      if ( ! --anim->time ) {
         TAnimFrame* frame;

         // Next frame
         newframe = 1;
         frame = anim->frames[ ++anim->frame_id ];

         // If frame is not null, we have a new frame, else animation may have ended or may recycle
         if (frame) {
            // New frame values
            anim->time = frame->time;
         } else if ( anim->status == as_cycle ) {
            // Recycle to first frame
            anim->frame_id = 0;
            anim->time     = anim->frames[0]->time;
         } else {
            // End animation
            anim->status = as_end;
         }
      }
   }

   // Report if a new frame has started
   return newframe;
}


TCharacter* getCharacter() { return &g_Character; }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update an entity (do animation, and change screen location)
//
void updateCharacter(TCharacter *c) {
   TEntity  *e = &c->entity;
   TPhysics *p = &e->phys;
   TAnimation* anim = &e->graph.anim;

   // Previous to calculations, next position is similar to current
   e->x = e->nx;
   e->y = e->ny;

   // Update animation
   switch(c->status) {
      case es_walk: 
         if      (p->vx < 0 || c->side != s_left)  setAnimation(e, g_anim[es_walk][s_left],  as_cycle);
         else if (p->vx > 0 || c->side != s_right) setAnimation(e, g_anim[es_walk][s_right], as_cycle);
         else if (!p->vx) e->graph.anim.status = as_pause;
         break;

      default:
   }
   updateAnimation(anim);

   // Apply gravity only if the entity is not over a floor
   if ( !isOverFloor(e) ) {
      if ( p->floor ) p->floor = 0; // If the entity was over a floor, disconnect the floor from the entity

      // Apply gravity
      p->vy += G_gy;
      p->vx += G_gx;
   }
   // Crop velocity limits
   if ( p->vy >= 0 ) {
      if ( p->vy > G_maxVel ) p->vy = G_maxVel;
   } else if ( p->vy < -G_maxVel ) { 
      p->vy = -G_maxVel;
   }
   if ( p->vx >= 0 ) {
      if ( p->vx > G_maxVel ) p->vx = G_maxVel;
   } else if ( p->vx < -G_maxVel ) {
      p->vx = -G_maxVel;  
   }

   // Move character
   p->x += p->vx;
   p->y += p->vy;

   // Calculate pixel position
   e->nx = p->x / SCALE;
   e->ny = p->y / SCALE;

   // Check if character has moved to calculate new location
   if      ( e->ny != e->y ) e->pscreen  = cpct_getScreenPtr(g_SCR_VMEM, e->nx, e->ny);
   else if ( e->nx != e->x ) e->pscreen += (i8)e->nx - (i8)e->x;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if a entity is still over its defined floor
//
u8 isOverFloor(TEntity *e) {
   u8 over = 0;                  // Value to sign if we are over a floor or not
   TEntity *f = e->phys.floor;   // Get the pointer to the floor assigned to this entity

   // Check if there is a floor assigned to the entity (not null pointer)
   if ( f ) {
      TAnimFrame *e_a = e->graph.anim.frames[e->graph.anim.frame_id];
      // Check X boundaries
      if ( e->x <= (f->x + f->graph.block.w) &&    // X lower  than right border of the block
          (e->x + e_a->width) >= f->x           )  // X + width higher than left border of the block
         over = 1;

      // Note: We do not check Y boundaries, because they do not change until the user
      // jumps, but then the floor is disconnected automatically. 
   }

   // Inform if we are over a floor or not
   return over;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets up a new location for an entity in screen (and its velocity)
//
void setEntityLocation(TEntity *e, u8 x, u8 y, u8 vx, u8 vy) {
   e->pscreen   = cpct_getScreenPtr(g_SCR_VMEM, x, y);
   e->x = e->nx = x;
   e->y = e->ny = y;
   e->phys.x    = x  * SCALE;
   e->phys.y    = y  * SCALE;
   e->phys.vx   = vx * SCALE;
   e->phys.vy   = vy * SCALE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets up a new animation for a given entity
//
void setAnimation(TEntity *ent, TAnimFrame** animation, TAnimStatus status) {
   TAnimation* anim = &ent->graph.anim;
   anim->frames     = animation; // Sets the new animation to the entity
   anim->frame_id   = 0;         // First frame of the animation
   anim->time       = 0;         // Animation is at its initial timestep
   anim->status     = status;    // Set the initial status for the animation
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sets up a new animation and status for a given Character
//
void setCharacterAnim(TCharacter *ch, TCharacterStatus newstatus, TCharacterSide newside) {
   TAnimStatus  a_st = as_play;                     // Initial status for the animation (Play, by default)
   TAnimFrame** anim = g_anim[newstatus][newside];  // Animation frames for the entity
   
   // Set new status of the character and the side where it is facing
   ch->status = newstatus; // New status for the entity
   ch->side   = newside;   // New side the entity is facing
      
   // Initial status of the animation is "play" except when walking ("cycle", then)
   if (newstatus == es_walk)
      a_st = as_cycle;

   // Set the animation
   setAnimation(&ch->entity, anim, a_st);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draw a given entity on the screen
//
void drawEntity  (TEntity* ent){
   // Get the entity values from its current animation status
   TAnimation* anim  = &ent->graph.anim;
   TAnimFrame* frame = anim->frames[anim->frame_id];

   // Draw the entity
   cpct_drawSprite(frame->sprite, ent->pscreen, frame->width, frame->height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Draw all the scene 
//
void drawAll() {
   u8 i;

   drawEntity(&g_Character.entity);

   // Draw Blocks
   for (i=0; i < g_lastBlock; ++i) {
      TEntity *e = (g_blocks + i);
      cpct_drawSolidBox(e->pscreen, e->graph.block.colour, e->graph.block.w, e->graph.block.h);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Creates a new solid block and returns its entity pointer
//
TEntity* newSolidBlock(u8 x, u8 y, u8 width, u8 height, u8 colour) {
   TEntity *newEnt = 0;

   // Only can create when we are below the limit
   if (g_lastBlock < g_MaxBlocks) {
      // Create the new entity by assigning given values to the one in the entity vector
      newEnt = &g_blocks[g_lastBlock];
      newEnt->graph.block.w      = width;
      newEnt->graph.block.h      = height;
      newEnt->graph.block.colour = colour;
      newEnt->pscreen            = cpct_getScreenPtr(g_SCR_VMEM, x, y);
      newEnt->x = newEnt->nx     = x;
      newEnt->y = newEnt->ny     = y;
      newEnt->phys.bounce        = 0.85 * SCALE; // Only bounce coefficient makes sense on solid blocks,
                                                 // as they are not affected by Physics
      ++g_lastBlock;   // One more entity added to the vector
   }

   return newEnt;
}