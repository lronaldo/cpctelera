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

#include "helpers.h"
#include "tmxtilemap.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <main.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//
CPCT_TMX_Tilemap  g_theTilemap;           // Tilemap information obtained from the TMX file
std::string       g_outputFolder;         // Output folder for output files
std::string       g_fileBasename;         // Basename for output files without folder or extension (only the name)
uint16_t          g_generate_flags = 0;   // Collection of flags indicating which output files to generate (all set to false)
const uint16_t    g_GENFLAG_C   =  1;     // Flag to indicate that a C file must be generated
const uint16_t    g_GENFLAG_H   =  2;     // Flag to indicate that a H file must be generated
const uint16_t    g_GENFLAG_HS  =  4;     // Flag to indicate that a H.S file must be generated
const uint16_t    g_GENFLAG_BIN =  8;     // Flag to indicate that a BIN file must be generated
const uint16_t    g_GENFLAG_ASM = 16;     // Flag to indicate that a ASM file must be generated

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPE DEFINITIONS
//
typedef void (CPCT_TMX_Tilemap::*TOutputMethod)(std::ostream&) const; // Method pointer to output functions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USAGE FUNCTION
//
void usage(const std::string& prg) {
   std::cerr 
      << C_LIGHT_YELLOW << "USAGE" 
      << C_LIGHT_BLUE   << "\n   " << prg << " <tmxfile> [OPTIONS]"
      << C_LIGHT_YELLOW << "\n\nDescription"
      << C_CYAN         << "\n   Script for generating csv files with maps defined in tmx files (from tiled map editor).\
\n\
\n   This script converts tmx files saved in CSV format to csv files ready to be included in C source files using #include\
directive. It also reindexes tile ids starting from 0 (as in tmx files tile ids start from 1).\
\n\
\n   Conversion is output to stdout unless otherwise requested by options."
      << C_LIGHT_YELLOW << "\n\nKNOWN LIMITATIONS:"
      << C_CYAN         << "\n    This script will only work with tmx files saved as CSV format. Output format must be \
selected as CSV in tilemap properties."
      << C_LIGHT_YELLOW << "\n\nOPTIONS:"
      << C_LIGHT_BLUE   << "\n\n   -agc | --asm-global-constants"
      << C_CYAN         << "\n          Make assembly generated constants global instead of local (Default: no)"
      << C_LIGHT_BLUE   << "\n   -ang | --asm-not-generate-globals"
      << C_CYAN         << "\n          Assembly output will not generate .globl directives for global labels in the header (Default: yes)"
      << C_LIGHT_BLUE   << "\n   -au  | --add-underscore-s-vars"
      << C_CYAN         << "\n          Adds an underscore in front of all variable name generated into assembly files to make them C-compatible (Default: no)"
      << C_LIGHT_BLUE   << "\n   -ba  | --bitarray <bits>"
      << C_CYAN         << "\n          Generates output as an array of bits, being <bits> the amount of bits for every element (1, 2, 4 or 6)"
      << C_LIGHT_BLUE   << "\n   -ci  | --c-identifier <id>"
      << C_CYAN         << "\n          Sets the C-identifier that will be used for the generated array (Default: filename)"
      << C_LIGHT_BLUE   << "\n   -gc  | --generate-c"
      << C_CYAN         << "\n          Generates a .C file with an array containing converted values (Default: yes). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gh  | --generate-h"
      << C_CYAN         << "\n          Generates a .H file with the declaration of the array for C file (Default: yes). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -ghs | --generate-h-s"
      << C_CYAN         << "\n          Generates a .H.S file with the declaration of the array for ASM file (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gb  | --generate-bin"
      << C_CYAN         << "\n          Generates a .BIN file with a raw string containing the converted values (same values as C array) (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gs  | --generate-asm"
      << C_CYAN         << "\n          Generates a .S (ASM) file with converted values (same values as C array) (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -h   | --help"
      << C_CYAN         << "\n          Shows this help information."
      << C_LIGHT_BLUE   << "\n   -nb  | --number-base <base>"
      << C_CYAN         << "\n          Selects the output numerical base. Valid values are: { dec, hex, bin }. Default: dec (Decimal)"
      << C_LIGHT_BLUE   << "\n   -nm  | --do-not-use-cpct-macros"
      << C_CYAN         << "\n          Does not use CPCtelera macros when producing array values. Default: macros used"
      << C_LIGHT_BLUE   << "\n   -of  | --output-folder <folder>"
      << C_CYAN         << "\n          Changes the output folder for generated C/H files (Default: .)"
      << "\n\n" << C_NORMAL;

   exit(-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSES ARGUMENTS GIVEN TO THIS SCRIPT
//
void parseArguments(const TArgs& args) {
   std::string filename = "";

   // Check that the number of arguments is valid
   if (args.size() < 2) usage(args[0]);

   // PARSE ARGUMENTS ONE BY ONE
   for(std::size_t i=1; i < args.size(); ++i) {
      const auto& a = args[i];

      //------------------------ MAKE ASSEMBLY CONSTANTS LOCAL
      if (a == "-agc" || a == "--asm-global-constants") {
         g_theTilemap.setASMConstantsLocal(false) ; // Sets generated constants to be global

      //------------------------ PREVENT GENERATION OF .GLOBL DIRECTIVES FOR LABELS
      } else if (a == "-ang" || a == "--asm-not-generate-globals") {
         g_theTilemap.setASMGenerateGlobal(false) ; // Prevents generation of .global directives

      //------------------------ SET UNDERSCORE PREFIX 
      } else if (a == "-au" || a == "--add-underscore-s-vars") {
         g_theTilemap.setASMVariablesPrefix('_') ; // Sets _ as prefix for asm variables

      //------------------------ BITARRAY MODIFIER
      } else if (a == "-ba" || a == "--bitarray") {
         if ( i + 1 >= args.size() ) error( { "Modifier '-ba | --bitarray' needs to be followed by the amount of bits (1, 2, 4, 6 or 8), but nothing found."} );
         std::string bits = args[i+1];
         if       ( bits == "1" ) { g_theTilemap.setBitsPerItem(1); }
         else if  ( bits == "2" ) { g_theTilemap.setBitsPerItem(2); }
         else if  ( bits == "4" ) { g_theTilemap.setBitsPerItem(4); }
         else if  ( bits == "6" ) { g_theTilemap.setBitsPerItem(6); }
         else if  ( bits == "8" ) { g_theTilemap.setBitsPerItem(8); }
         else { error( { "'", bits,"' is not a valid value for '-ba | --bitarray'. Valid values are: 1, 2, 4, 6, 8."  } ); }
         
         ++i;
      //------------------------ SELECT C IDENTIFIER
      } else if (a == "-ci" || a == "--c-identifier") {
         if (i + 1 >= args.size()) error( { "Modifier '-ci | --c-identifier' needs to be followed by a desired c-identifier, but nothing found."} );
         g_theTilemap.setCID(args[i + 1]);

         ++i;
      //------------------------ GENERATE C OUTPUT FILE
      } else if (a == "-gc" || a == "--generate-c") {
         g_generate_flags |= g_GENFLAG_C; // Set C file to be generated

      //------------------------ GENERATE H OUTPUT FILE
      } else if (a == "-gh" || a == "--generate-h") {
         g_generate_flags |= g_GENFLAG_H; // Set H file to be generated

      //------------------------ GENERATE H.S OUTPUT FILE
      } else if (a == "-ghs" || a == "--generate-h-s") {
         g_generate_flags |= g_GENFLAG_HS; // Set .H.S file to be generated

      //------------------------ GENERATE BIN OUTPUT FILE
      } else if (a == "-gb" || a == "--generate-bin") {
         g_generate_flags |= g_GENFLAG_BIN; // Set BIN file to be generated

      //------------------------ GENERATE BIN OUTPUT FILE
      } else if (a == "-gs" || a == "--generate-asm") {
         g_generate_flags |= g_GENFLAG_ASM; // Set ASM file to be generated

      //------------------------ SELECT NUMBER BASE
      } else if (a == "-nb" || a == "--number-base") {
         if (i + 1 >= args.size()) error( { "Modifier '-nb | --number-base' needs to be followed by selected numerical base (dec, bin or hex), but nothing found."} );
         std::string base = args[i+1];
         std::transform(base.begin(), base.end(), base.begin(), ::tolower);
         switch (str2int(base.c_str())) {
            case str2int("dec"): g_theTilemap.setOutputNumberFormat(TNumberFormat::decimal);     break;
            case str2int("hex"): g_theTilemap.setOutputNumberFormat(TNumberFormat::hexadecimal); break;
            case str2int("bin"): g_theTilemap.setOutputNumberFormat(TNumberFormat::binary_text); break;
            default: error( { "'", base, "' is not a valid numerical base for '-nb | --number-base'. Valid bases are: dec, bin, hex."} ); 
         }

         ++i;
      //------------------------ DO NOT USE CPCTELERA MACROS
      } else if (a == "-nm" || a == "--do-not-use-cpct-macros") {
         g_theTilemap.setUseCPCTMacros(false);

      //------------------------ SELECT OUTPUT FOLDER
      } else if (a == "-of" || a == "--output-folder") {
         if (i + 1 >= args.size()) error( { "Modifier '-of | --output-folder' needs to be followed by a folder, but nothing found."} );
         g_outputFolder = removeRepetitions(args[i + 1], '/');
         ensureOnly1CharBack(g_outputFolder, '/');
         if ( !isFolderWritable(g_outputFolder.c_str()) ) error ({ "Folder '", g_outputFolder, "' does not exist, is not a folder or is not writable." });

         ++i;
      //------------------------ SHOW HELP
      } else if (a == "-h" || a == "--help") {
         usage(args[0]);

      //------------------------ UNKNOWN MODIFIER
      } else if (a[0] == '-') {
         error( { "Unknown modifier '", a, "'" } );
      
      //------------------------ TMX FILE
      } else {
         filename = args[i];
         g_fileBasename = notdir(basename(filename), '/');
      }
   }

   // Load the TMX with selected options
   if (filename == "") error( { "No TMX file given. A TMX file is required to parse its contents." } );
   g_theTilemap.loadMap(filename.c_str());

   // If no generation flags selected, turn on default values
   if ( !g_generate_flags ) 
      g_generate_flags = g_GENFLAG_C | g_GENFLAG_H;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GENERATE AN OUTPUT FILE
//
void generateOutput(std::string ext, TOutputMethod output, std::ios_base::openmode mode = std::ios_base::out) {
   // Open the file
   std::string filename = g_outputFolder + g_fileBasename + ext;
   std::ofstream file(filename, mode);
   if ( !file ) error ( { "There was an unknown problem opening '", filename, "' for writing output data." } );
   
   // Generate contents
   (g_theTilemap.*output)(file);

   // Close the file
   file.close();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT OF THE SCRIPT
//
int main(int argc, char **argv) {
   // Get and parse commandline arguments
   try {
      TArgs args(argv, argv + argc);
      parseArguments(args);

      // Open output files and write them
      // First generate textual files
      if ( g_generate_flags & g_GENFLAG_C   ) generateOutput(".c"    , &CPCT_TMX_Tilemap::output_basic_C    );
      if ( g_generate_flags & g_GENFLAG_H   ) generateOutput(".h"    , &CPCT_TMX_Tilemap::output_basic_H    );
      if ( g_generate_flags & g_GENFLAG_HS  ) generateOutput(".h.s"  , &CPCT_TMX_Tilemap::output_basic_HS   );

      // Assembler cannot use macros
      g_theTilemap.setUseCPCTMacros(false);
      if ( g_generate_flags & g_GENFLAG_ASM ) generateOutput(".s"    , &CPCT_TMX_Tilemap::output_basic_S    );
      
      // And now binary files
      g_theTilemap.setOutputNumberFormat(TNumberFormat::binary);
      if ( g_generate_flags & g_GENFLAG_BIN ) generateOutput(".bin"  , &CPCT_TMX_Tilemap::output_basic_BIN  );

   } catch (std::exception& e) {
      std::cerr << "ERROR: " << e.what() << "\n";
      return -1;
   }

   return 0;
}
