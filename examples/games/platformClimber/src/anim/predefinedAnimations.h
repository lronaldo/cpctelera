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

#ifndef _PREDEFINED_ANIMATIONS_
#define _PREDEFINED_ANIMATIONS_

#include "animation.h"
#include "../entities/entities.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////
//////  PREDEFINED ANIMATIONS
//////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// All the animation frames to be used in this example
extern const TAnimFrame g_allAnimFrames[16];

// All complete animations used in this example (NULL terminated, to know the end of the array)
extern TAnimFrame* const g_walkLeft[5];
extern TAnimFrame* const g_walkRight[5];
extern TAnimFrame* const g_jumpLeft[6];
extern TAnimFrame* const g_jumpRight[6];
extern TAnimFrame* const g_hitLeft[2];
extern TAnimFrame* const g_hitRight[2];

// Vector containing references to all animations, ordered by states and sides
extern TAnimFrame** const g_anim[es_NUMSTATUSES][s_NUMSIDES];

#endif