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

#include "animation.h"
#include "../entities/entities.h"
#include "../sprites/sprites.h"

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
const TAnimFrame g_allAnimFrames[16] = {
   { G_EMRright,       4, 16,  4 }, // 0// << Walk Right Frames
   { G_EMRright2,      2, 16,  4 }, // 1// |
   { G_EMRleft,        4, 16,  4 }, // 2// << Walk Left Frames
   { G_EMRleft2,       2, 16,  4 }, // 3// |
   { G_EMRjumpright1,  4,  8,  3 }, // 4// << Jump Right Frames 
   { G_EMRjumpright2,  4,  8,  4 }, // 5// |
   { G_EMRjumpright3,  4,  8,  4 }, // 6// |
   { G_EMRjumpright4,  4,  8,  3 }, // 7// |
   { G_EMRjumpleft1,   4,  8,  3 }, // 8// << Jump Left Frames 
   { G_EMRjumpleft2,   4,  8,  4 }, // 9// |
   { G_EMRjumpleft3,   4,  8,  4 }, //10// |
   { G_EMRjumpleft4,   4,  8,  3 }, //11// |
   { G_EMRhitright,    4, 16,  6 }, //12// << Hit Right Frame
   { G_EMRhitleft,     4, 16,  6 }, //13// << Hit Left Frame
   { G_EMRright3,      4, 16,  4 }, //14// << Walk 3rd steps
   { G_EMRleft3,       4, 16,  4 }  //15// |
};

// Use a define for convenience (making name of the constant shorter)
#define AF g_allAnimFrames

//
// All complete animations used in this example (NULL terminated, to know the end of the array)
//
TAnimFrame*  const g_walkLeft[5]  = { &AF[2], &AF[3], &AF[15], &AF[3], 0 };
TAnimFrame*  const g_walkRight[5] = { &AF[0], &AF[1], &AF[14], &AF[1], 0 };
TAnimFrame*  const g_jumpLeft[6]  = { &AF[8], &AF[9], &AF[10], &AF[11], &AF[3], 0 };
TAnimFrame*  const g_jumpRight[6] = { &AF[4], &AF[5], &AF[ 6], &AF[ 7], &AF[1], 0 };
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

// Macro can be undefined, as it is not required anymore
#undef AF
