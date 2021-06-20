//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016-2018 César Nicolás González (@CNGSoft)
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
/*********************************

Record a raw or AMSDOS binary file
into a TZX/CDT file using a signal
that encodes single or double bits
into either half or full pulses.

*********************************/

#pragma once

// USEFUL VARDIADIC MACRO FOR RAISING ERRORS
#define error(err, ...) { fprintf(stderr, __VA_ARGS__); exit(err); }

// PUBLIC FUNCTION DECLARATIONS
void tiny_tape_usage();
void tiny_tape_setBitGaps(int bg);
void tiny_tape_setSkipHeader(int sk);
void tiny_tape_gen(	const char* srcfile, const char* tzxfile, int _bittype
					, int _bitsize, int _bitbyte, int _bithold);
