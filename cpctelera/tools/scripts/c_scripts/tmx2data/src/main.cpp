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
#include <main.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//
CPCT_TMX_Tilemap  g_theTilemap;  // Tilemap information obtained from the TMX file


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
      << C_LIGHT_BLUE   << "\n\n   -ba | --bitarray <bits>"
      << C_CYAN         << "\n          Generates output as an array of bits, being <bits> the amount of bits for every element (1, 2, 4 or 6)"
      << C_LIGHT_BLUE   << "\n   -ci | --c-identifier <id>"
      << C_CYAN         << "\n          Sets the C-identifier that will be used for the generated array (Default: filename)"
      << C_LIGHT_BLUE   << "\n   -gc | --generate-c"
      << C_CYAN         << "\n          Generates a C file with an array containing converted values"
      << C_LIGHT_BLUE   << "\n   -gh | --generate-h"
      << C_CYAN         << "\n          Generates a H file with the declaration of the array for C file (implies -gc)"
      << C_LIGHT_BLUE   << "\n   -h  | --help"
      << C_CYAN         << "\n          Shows this help information."
      << C_LIGHT_BLUE   << "\n   -of | --output-folder <folder>"
      << C_CYAN         << "\n          Changes the output folder for generated C/H files (Default: .)"
      << "\n\n" << C_NORMAL;

   exit(-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSES ARGUMENTS GIVEN TO THIS SCRIPT
//
void parseArguments(const TArgs& args) {
   // Check that the number of arguments is valid
   if (args.size() < 2) usage(args[0]);

   // PARSE ARGUMENTS ONE BY ONE
   for(std::size_t i=1; i < args.size(); ++i) {
      const auto& a = args[i];

      //------------------------ BITARRAY MODIFIER
      if (a == "-ba" || a == "--bitarray") {
         if (i + 1 >= args.size()) error( { "Modifier '-ba | --bitarray' needs to be followed by the amount of bits (1, 2, 4 or 6), but nothing found."} );
         std::cerr << "-ba option";
         
         ++i;

      //------------------------ SHOW HELP
      } else if (a == "-h" || a == "--help") {
         usage(args[0]);

      //------------------------ UNKNOWN MODIFIER
      } else if (a[0] == '-') {
         error( { "Unknown modifier '", a, "'" } );
      
      //------------------------ TMX FILE
      } else {
         g_theTilemap.loadMap(args[i].c_str());
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT OF THE SCRIPT
//
int main(int argc, char **argv) {
   // Get and parse commandline arguments
   try {
      TArgs args(argv, argv + argc);
      parseArguments(args);
      g_theTilemap.printSomeInfo();
      g_theTilemap.output_basic_C(std::cout);
   } catch (std::exception& e) {
      std::cerr << "ERROR: " << e.what() << "\n";
      return -1;
   }

   return 0;
}