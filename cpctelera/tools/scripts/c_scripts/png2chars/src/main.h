//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
// File: cpct_png2chars
//    Script for generating UDG character definitions from PNG files. 
//
// Usage:
//    <cpct_png2chars> <file> [options]
//
// Description:
//    This script converts PNG files into BASIC, C or ASM files with UDG character definitions
// extracted from the pixels in the PNG image. PNG file must contain a series of 8x8 UDG character
// definitions. Therefore, PNG width and height must both be a multiple of 8.
//
//    Any pixel in the image that is not black (RGB 000000) is considered to be lit and will be 
// treated as a 1. Black and white PNGs are recommended not to make mistakes when editing the 
// UDG characters. 
//
//   Conversion is output to stdout unless otherwise requested by options. Folder and file options 
// enable output to files.
//
// Command line options:
//    -alc | --asm-local              - Make assembly generated labels local instead of global (Default: no)
//    -au  | --add-underscore-s-vars  - Adds an underscore in front of all variable name generated into assembly files to make them C-compatible (Default: no)
//    -ci  | --c-identifier <id>      - Sets the C/ASM-identifier that will be used for the generated array (Default: filename)
//    -gb  | --generate-bin           - Generates a BIN file with raw data of the UDG character definitions (same values as C array) (Default: no). Defaults are set to false on using this flag.
//    -gbas| --generate-basic         - Generates a .BAS file with SYMBOL lines containing the UDG definitions (Default: no). Defaults are set to false on using this flag.
//    -gc  | --generate-c             - Generates a C file with an array all UDG definitions together, one per line (Default: yes). Defaults are set to false on using this flag.
//    -gh  | --generate-h             - Generates a .H file with the declaration of the array for C file (Default: yes). Defaults are set to false on using this flag.
//    -ghs | --generate-h-s           - Generates a .H.S file with the declaration of the array for ASM file (Default: no). Defaults are set to false on using this flag.
//    -gs  | --generate-asm           - Generates a .S (ASM) file with UDG character definitions in one array (same values as C array) (Default: no). Defaults are set to false on using this flag.
//    -h   | --help                   - Shows help information on the terminal.
//    -nb  | --number-base <base>     - Selects the output numerical base. Valid values are: { dec, hex, bin }. Default: dec (Decimal)
//    -o   | --output-filename <filename> - Sets the basename for the files that will be generated. Enables file output. By default, files will be written to current working folder.
//    -of  | --output-folder <folder> - Changes the output folder for generated files and enables file output.
//

#pragma once