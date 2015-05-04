//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2009 Targhan / Arkos
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
//-------------------------------------------------------------------------------
//######################################################################
//### MODULE: Audio                                                  ###
//######################################################################
//### This module contains code for music and SFX players and other  ###
//### audio routines.                                                ###
//######################################################################
//
#ifndef CPCT_AUDIO_H
#define CPCT_AUDIO_H

//
// Arkos Player: full control version (without interrupts)
// 

// Arkos Player Music Control Functions
extern void cpct_akp_musicInit  (void* songdata);
extern void cpct_akp_musicPlay  ();
extern void cpct_akp_musicStop  ();

// Arkos Player Sound FX Control Functions
extern void cpct_akp_enableSFX  (u8 enable_bitmask);
extern void cpct_akp_disableSFX (u8 disable_bitmask);
extern void cpct_akp_SFXInit    (void* sfx_song_data);
extern void cpct_akp_SFXStopAll ();
extern void cpct_akp_SFXStop    (u8 stop_bitmask);
extern void cpct_akp_SFXPlay    (u8 sfx_num, u8 volume, u8 note, u8 speed, 
                                 u16 inverted_pitch, u8 channel_num);

#endif