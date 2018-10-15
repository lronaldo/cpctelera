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
// File: cpct_tmx2data
//    Script for generating csv files with maps defined in tmx files 
// (from tiled map editor).
//
// Usage:
//    <cpct_tmx2csv> [file] [options]
//
// Known limitations:
//    * This script will *only* work with tmx files saved as CSV format. 
// Output format must be selected as CSV in tilemap properties.
//
// Description:
//    This script converts tmx files saved in CSV format to csv files
// ready to be included in C source files using #include directive.
// It also reindexes tile ids starting from 0 (as in tmx files tile ids
// start from 1).
//
//    Conversion is output to stdout unless otherwise requested by options.
//
// Command line options:
//    -alc | --asm-local-constants    - Make assembly generated constants local instead of global (Default: no)
//    -au  | --add-underscore-s-vars  - Adds an underscore in front of all variable name generated into assembly files to make them C-compatible (Default: no)
//    -ba  | --bitarray <bits>        - Generates output as an array of bits, being <bits> the amount of bits for every element (1, 2, 4 or 6)
//    -ci  | --c-identifier <id>      - Sets the C-identifier that will be used for the generated array (Default: filename)
//    -gc  | --generate-c             - Generates a C file with an array containing converted values (Default: yes). Defaults are set to false on using this flag.
//    -gh  | --generate-h             - Generates a H file with the declaration of the array for C file (Default: yes). Defaults are set to false on using this flag.
//    -ghs | --generate-h-s           - Generates a .H.S file with the declaration of the array for ASM file (Default: no). Defaults are set to false on using this flag.
//    -gb  | --generate-bin           - Generates a BIN file with a raw string containing the converted values (same values as C array) (Default: no). Defaults are set to false on using this flag.
//    -gs  | --generate-asm           - Generates a .S (ASM) file with converted values (same values as C array) (Default: no). Defaults are set to false on using this flag."
//    -h   | --help                   - Shows help information on the terminal.
//    -nb  | --number-base <base>     - Selects the output numerical base. Valid values are: { dec, hex, bin }. Default: dec (Decimal)
//    -nm  | --do-not-use-cpct-macros - Does not use CPCtelera macros when producing array values. Default: macros used
//    -of  | --output-folder <folder> - Changes the output folder for generated C/H files (Default: .)
//

#pragma once