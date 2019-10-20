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

#include <iostream>
#include <cstdint>
#include <array>
#include <functional>
#include <helpers.h>
#include <PNGDecoder.hpp>


PNGDecoder thePNGDecoder;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USAGE FUNCTION
//
void usage(const std::string& prg) {
   std::cerr 
      << C_LIGHT_YELLOW << "USAGE" 
      << C_LIGHT_BLUE   << "\n   " << prg << " <pngfile> [OPTIONS]"
      << C_LIGHT_YELLOW << "\n\nDescription"
      << C_CYAN         << "\n   Script for generating UDG character definitions from PNG files. .\
\n\
\n   This script converts PNG files into BASIC, C or ASM files with UDG character definitions \
extracted from the pixels in the PNG image. PNG file must contain a series of 8x8 UDG character \
definitions. Therefore, PNG width and height must both be a multiple of 8. \
\n\
\n   Any pixel in the image that is not black (RGB 000000) is considered to be lit and will be \
treated as a 1. Black and white PNGs are recommended not to make mistakes when editing the \
UDG characters. \
\n\
\n   Conversion is output to stdout unless otherwise requested by options."
      << C_LIGHT_YELLOW << "\n\nOPTIONS:"
      << C_LIGHT_BLUE   << "\n\n   -alc | --asm-local-constants"
      << C_CYAN         << "\n          Make assembly generated constants local instead of global (Default: no)"
      << C_LIGHT_BLUE   << "\n   -au  | --add-underscore-s-vars"
      << C_CYAN         << "\n          Adds an underscore in front of all variable name generated into assembly files to make them C-compatible (Default: no)"
      << C_LIGHT_BLUE   << "\n   -ci  | --c-identifier <id>"
      << C_CYAN         << "\n          Sets the C/ASM-identifier that will be used for the generated array (Default: filename)"
      << C_LIGHT_BLUE   << "\n   -gb  | --generate-bin"
      << C_CYAN         << "\n          Generates a BIN file with raw data of the UDG character definitions (same values as C array) (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gbas| --generate-basic"
      << C_CYAN         << "\n          Generates a .BAS file with SYMBOL lines containing the UDG definitions (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gc  | --generate-c"
      << C_CYAN         << "\n          Generates a C file with an array all UDG definitions together, one per line (Default: yes). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gh  | --generate-h"
      << C_CYAN         << "\n          Generates a .H file with the declaration of the array for C file (Default: yes). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -ghs | --generate-h-s"
      << C_CYAN         << "\n          Generates a .H.S file with the declaration of the array for ASM file (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gs  | --generate-asm"
      << C_CYAN         << "\n          Generates a .S (ASM) file with UDG character definitions in one array (same values as C array) (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -gtrm| --generate-terminal"
      << C_CYAN         << "\n          Generates a terminal output which draws all UDG in 8x8 character matrices. This is for testing purposes. (Default: no). Defaults are set to false on using this flag."
      << C_LIGHT_BLUE   << "\n   -h   | --help"
      << C_CYAN         << "\n          Shows this help information."
      << C_LIGHT_BLUE   << "\n   -nb  | --number-base <base>"
      << C_CYAN         << "\n          Selects the output numerical base. Valid values are: { dec, hex, bin }. Default: dec (Decimal)"
      << C_LIGHT_BLUE   << "\n   -of  | --output-folder <folder>"
      << C_CYAN         << "\n          Changes the output folder for generated files (Default: .)"
      << "\n\n" << C_NORMAL;
}


void setASMConstantsLocal(bool st, const TArgs& args) {
   thePNGDecoder.setASMConstantLocal(st);
}

void setASMVariablesPrefix(char prefix, const TArgs& args){
   thePNGDecoder.setIdentifierPrefix(prefix);
}

void setCID(const TArgs& args) {
   if ( !isValidCIdentifier(args[1].c_str()) ) error({ "'", args[1], "' is not a valid C-identifier for '", args[0], "' modifier." });
   thePNGDecoder.setCArrayName(args[1]);
}

void callUsage(std::string progname, const TArgs& args){
   throw std::bad_exception();
}

void generate(uint8_t gen, const TArgs& args) {
   thePNGDecoder.setGenerate(gen);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STRUCTURE TO HOLD THE COMMAND LINE OPTION MODIFIERS AND THE FUNCTIONS THAT
// WILL BE CALLED WHEN EACH OF THIS MODIFIERS IS MET
//
struct ParseFunction_t {
   std::string shortopt;
   std::string longopt;
   uint16_t    numargs;
   std::function<void(const TArgs&)> parseFunc;
};

constexpr uint16_t knum_modifiers = 9;

using std::placeholders::_1;
std::array<ParseFunction_t, knum_modifiers> Modifiers {{
      { "-alc" , "--asm-local-constants"   , 0, std::bind(setASMConstantsLocal, true, _1) }
   ,  { "-au"  , "--add-underscore-s-vars" , 0, std::bind(setASMVariablesPrefix, '_', _1) }
   ,  { "-ci"  , "--c-identifier"          , 1, std::bind(setCID, _1) }
   ,  { "-gb"  , "--generate-binary"       , 0, std::bind(generate, PNGDecoder::_BIN, _1) }
   ,  { "-gbas", "--generate-basic"        , 0, std::bind(generate, PNGDecoder::_BAS, _1) }
   ,  { "-gc"  , "--generate-c"            , 0, std::bind(generate, PNGDecoder::_C, _1) }
   ,  { "-gs"  , "--generate-s"            , 0, std::bind(generate, PNGDecoder::_S, _1) }
   ,  { "-gtrm", "--generate-terminal"     , 0, std::bind(generate, PNGDecoder::_DRAW, _1) }
   ,  { "-h"   , "--help"                  , 0, std::bind(callUsage, "cpct_png2chars", _1) }
}};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSES ARGUMENTS GIVEN TO THIS SCRIPT
//
void parseArguments(const TArgs& args) {
   // Check that the number of arguments is valid
   if (args.size() < 2) throw std::bad_exception();

   // PARSE ARGUMENTS ONE BY ONE
   for(std::size_t i=1; i < args.size(); ++i) {
      const auto& a = args[i];

      // Is it a Modifier?
      if ( a[0] == '-' ) {
         for( auto&& m : Modifiers ) {
            if ( m.shortopt == a || m.longopt == a ) {
               // Check for required arguments
               if ( m.numargs > 0 ) {
                  if ( i + m.numargs >= args.size() ) error({ "Modifier '", a, "' requires '", std::to_string(m.numargs), "' parameter(s) to be given." });
                  // Pass required arguments
                  TArgs modargs( args.begin()+i, args.begin()+i+m.numargs+1 );
                  m.parseFunc(modargs);
               } else {
                  // Call with 0 arguments
                  m.parseFunc({});
               }
               // Do not process arguments as modifiers
               i += m.numargs;
               // Modifier has been processed, jump directly to 
               // process next arguments
               goto nextarg;
            }
         }
         // No modifier coincides with this argument
         error({ "Unrecognized modifier '", a, "' " });
      } else {
         // Not a modifier
         thePNGDecoder.readFile(a);
      }

      nextarg:
      ;
   }

/*
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

      //------------------------ SELECT OUTPUT FOLDER
      } else if (a == "-of" || a == "--output-folder") {
         if (i + 1 >= args.size()) error( { "Modifier '-of | --output-folder' needs to be followed by a folder, but nothing found."} );
         g_outputFolder = removeRepetitions(args[i + 1], '/');
         ensureOnly1CharBack(g_outputFolder, '/');
         if ( !isFolderWritable(g_outputFolder.c_str()) ) error ({ "Folder '", g_outputFolder, "' does not exist, is not a folder or is not writable." });

         ++i;
      
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
*/
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT OF THE SCRIPT
//
#include <memory>
int main(int argc, char **argv) {
   // Get and parse commandline arguments
   try {
      TArgs args(argv, argv + argc);
      parseArguments(args);

      thePNGDecoder.convert();
   } catch (const std::runtime_error& e) {
      std::cerr << "### ERROR ###\n";
      std::cerr << " - " << e.what() << "\n";
      return -1;
   } catch (const std::exception& e) {
      usage(argv[0]);
      return -2;
   }

   return 0;
}


