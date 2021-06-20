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
// File: cpct_binpatch
//
//    Patches a binary file
//
// Usage:
//    <cpct_binpatch> (binfile) [OPTIONS]
//
// Description:
//    Patches the <binfile> by directly modifying bytes at given offsets in the available
// switches. For instance, switches '-pb 0x10 0xFF' will patch byte at offset 0x10 (16 in
// decimal) Overwriting it with the value 0xFF (255 in decimal). Similarly, switches
// '-ps 13000 1 2 3 4' will overwrite bytes 13000 to 13003 in the file with the values
// 1, 2, 3 and 4 (each value will overwrite next byte from offset 13000 onwards).
//
//    You may use as many switches as you want in the command line. Switches will be read
// and processed from left to write. <Binfile> is read in memory at the start of the process,
// and then each new switch processed produces modifications in memory. If anything fails
// during the process, the script ends and <binfile> is left unmodified (all modifications
// were made in memory). Modifications are only written to the file at the end, when all
// of them have been correctly processed.
//
//    If you want to test your modifications before they are made to the actual <binfile>
// you may use switch '-p'. This will make this script print out the result of the patching
// to your terminal, without modifying the <binfile>.
//
// IMPORTANT:
//    Your <binfile> will always result modified unless you use '-p' switch or
// in case that any error happens.
//
// Command line options:
//  -h  | --help                                - Shows this help
//  -p  | --print                               - Prints out the result of binary patching instead of overwritting the binfile.
//  -pb | --patch-byte <OFFSET> <8BITSVAL>      - Patches the byte at the given OFFSET with the new 8BITSVAL.
//  -pw | --patch-word <OFFSET> <16BITSVAL>     - Patches a word at the given OFFSET with 16BITSVAL, using little endian by default.
//  -ps | --patch-stream <OFFSET> <BYTESTREAM>  - Patches an BYTESTREAM of bytes starting at OFFSET.
//
