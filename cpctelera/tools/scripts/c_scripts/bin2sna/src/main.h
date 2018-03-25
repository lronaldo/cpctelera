//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
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

//
// File: cpct_bin2sna
//
//    Creates a runnable 64K snapshot (SNA) with a binary file in its memory, ready
// to be run.
//
// Usage:
//    <cpct_bin2sna> (binfile) [OPTIONS]
//
// Description:
//    Creates a runnable 64K snapshot (SNA) file with <binfile>
// loaded in memory at the address given with option -l/--load-address. If no address
// is given, binfile is loaded at 0x0000 (memory start). The value of the PC is also
// set to 0x0000 by default and may be changed with -pc/--set-pc.
//
// Command line options:
//   -l  | --load-address <ADDRESS> (Default 0x0000)   - Sets the memory address where 
// to load the binary file <binfile>.
//
//   -pc | --set-pc <ADDRESS> (Default 0x0000)         - Sets the value of the PC register
// at execution start. This value should be set to the entry point of the application (its run address).
//
//   -h  | --help                                      - Shows this help.
//       
//