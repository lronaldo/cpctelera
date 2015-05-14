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
//////  ANIMATIONS AND VALUES FOR MANAGING ENTITIES
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// All the animation frames to be used in this example
//
const TAnimFrame g_allAnimFrames[14] = {
   // Walk Right Frames
   { G_EMRright,       4, 16,  2 }, // 0// 
   { G_EMRright2,      3, 16,  2 }, // 1// 

   // Walk Left Frames
   { G_EMRleft,        4, 16,  2 }, // 2// 
   { G_EMRleft2,       3, 16,  2 }, // 3//

   // Jump Right Frames 
   { G_EMRjumpright1,  4,  8,  2 }, // 4// 
   { G_EMRjumpright2,  4,  8,  2 }, // 5//
   { G_EMRjumpright3,  4,  8,  2 }, // 6//
   { G_EMRjumpright4,  4,  8,  2 }, // 7//

   // Jump Left Frames 
   { G_EMRjumpleft1,   4,  8,  2 }, // 8// 
   { G_EMRjumpleft2,   4,  8,  2 }, // 9//
   { G_EMRjumpleft3,   4,  8,  2 }, //10//
   { G_EMRjumpleft4,   4,  8,  2 }, //11//

   // Hit Right 
   { G_EMRhitright,    4, 16,  2 }, //12// 
   
   // Hit Left
   { G_EMRhitleft,     4, 16,  2 }  //13// 
};

// Use a define for convenience
#define AF g_allAnimFrames

//
// All complete animations used in this example (NULL terminated, to know the end of the array)
//
TAnimFrame* const g_anim[es_NUMSTATUSES][s_NUMSIDES] = {
   // STATE 0 = es_static
   {  0, 0  },    // No animations for es_static status
   // STATE 1 = es_walk
   {  { &AF[3], &AF[2], 0 },  // Walk Left
      { &AF[1], &AF[0], 0 }   // Walk Right
   },
   // STATE 2 = es_jump
   {  { &AF[8], &AF[9], &AF[10], &AF[11], 0 },  // Jump Left
      { &AF[8], &AF[9], &AF[10], &AF[11], 0 }   // Jump Right
   },
   // STATE 3 = es_hit
   {  { &AF[13], 0 }, // Hit left
      { &AF[12], 0 }  // Hit Right
   }
};

#undef AF

//
// Some global constants defining our world
//
const u8 g_SCR_WIDTH  =  80;  // Screen width in bytes (80 bytes = 160 píxels)
const u8 g_SCR_HEIGHT = 200;  // Screen height in bytes

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  UTILITY FUNCTIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
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

//
// Update an entity (do animation, and change screen location)
//
void updateEntity(TEntity *ent, i8 mx, i8 my) {
   TAnimation* anim = ent->anim;
   updateAnimation(anim);

}

//
// Set a new animation and status to the entity
//
void setAnimation(TEntity *ent, TEntityStatus newstatus, TEntitySide newside) {
   TAnimation* anim = ent->anim;
   ent->status    = newstatus;                    // New status for the entity
   ent->side      = newside;                      // New side the entity is facing
   anim->frames   = g_anim[newstatus][newside];   // Animation frames for the entity
   
   // Initial status of the animation is "play" except for walking which will "cycle"
   switch (newstatus) {
      case es_walk:  anim->status = as_cycle; break;
      default:       anim->status = as_play;  break;
   }
   anim->frame_id = 0;     // First frame of the animation
   anim->time     = 0;     // Animation is at its initial timestep
}

//
// Draw a given entity on the screen
//
void drawEntity  (TEntity* ent){
   // Get the entity values from its current animation status
   TAnimation* anim  = ent->anim;
   TAnimFrame* frame = anim->frames[anim->frame_id];

   // Draw the entity
   cpct_drawSprite(frame->sprite, ent->pscreen, frame->width, frame->height);
}