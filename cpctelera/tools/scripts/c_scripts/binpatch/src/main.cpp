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

#include <iostream>
#include <cstdint>
#include <string>
#include <main.h>
#include <helpers.h>
#include <patcher.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USAGE FUNCTION
//
void usage(const std::string& prg) {
   using namespace CPCT;
   std::cerr 
      << C_LIGHT_YELLOW << "USAGE" 
      << C_LIGHT_BLUE   << "\n   " << prg << " <binfile> [OPTIONS]"
      << C_LIGHT_YELLOW << "\n\nDescription"
      << C_CYAN         << "\n   Patches the <binfile> by directly modifying bytes at given offsets in the available "
                        << "switches. For instance, switches '-pb 0x10 0xFF' will patch byte at offset 0x10 (16 in "
                        << "decimal) Overwriting it with the value 0xFF (255 in decimal). Similarly, switches "
                        << "'-ps 13000 1 2 3 4' will overwrite bytes 13000 to 13003 in the file with the values "
                        << "1, 2, 3 and 4 (each value will overwrite next byte from offset 13000 onwards)."
                        << "\n   You may use as many switches as you want in the command line. Switches will be read "
                        << "and processed from left to write. <Binfile> is read in memory at the start of the process, "
                        << "and then each new switch processed produces modifications in memory. If anything fails "
                        << "during the process, the script ends and <binfile> is left unmodified (all modifications "
                        << "were made in memory). Modifications are only written to the file at the end, when all "
                        << "of them have been correctly processed. "
                        << "\n   If you want to test your modifications before they are made to the actual <binfile> "
                        << "you may use switch '-p'. This will make this script print out the result of the patching "
                        << "to your terminal, without modifying the <binfile>."
      << C_LIGHT_YELLOW << "\n\nIMPORTANT" 
      << C_CYAN         << "\n   Your <binfile> will always result modified unless you use '-p' switch or "
                        << "in case that any error happens."
      << C_LIGHT_YELLOW << "\n\nOPTIONS" 
      << C_LIGHT_BLUE   << "\n\n   -h  | --help"
      << C_CYAN         << "\n          Shows this help."
      << C_LIGHT_BLUE   << "\n   -p  | --print"
      << C_CYAN         << "\n          Prints out the result of binary patching instead of overwritting the binfile."
      << C_LIGHT_BLUE   << "\n   -pb | --patch-byte <OFFSET> <8BITSVAL>"
      << C_CYAN         << "\n          Patches the byte at the given OFFSET with the new 8BITSVAL."
      << C_LIGHT_BLUE   << "\n   -pw | --patch-word <OFFSET> <16BITSVAL>"
      << C_CYAN         << "\n          Patches a word at the given OFFSET with 16BITSVAL, using little endian by default."
      << C_LIGHT_BLUE   << "\n   -ps | --patch-stream <OFFSET> <BYTESTREAM>"
      << C_CYAN         << "\n          Patches an BYTESTREAM of bytes starting at OFFSET."
      << "\n\n" << C_NORMAL;

   exit(-1);
}

// Global status of the script
CPCT::Patcher thePatcher;     // Patcher object for the binfile
bool  printResult = false;    // Print result instead of overwriting binfile

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PARSES ARGUMENTS GIVEN TO THIS SCRIPT
//
void parseArguments(const CPCT::TArgs& args) {
   // PARSE ARGUMENTS ONE BY ONE
   for(std::size_t i=1; i < args.size(); ++i) {
      const std::string& a = args[i];

      //----------------------------------------------------------------------------------------------------------
      // SHOW HELP
      if (a == "-h" || a == "--help") {
         usage(args[0]);
      
      //----------------------------------------------------------------------------------------------------------
      // PRINT RESULT
      } else if (a == "-p" || a == "--print") {
         printResult = true;

      //----------------------------------------------------------------------------------------------------------
      // PATCH 1 BYTE
      } else if (a == "-pb" || a == "--patch-byte") {
         if (i + 2 >= args.size()) CPCT::error( { "Modifier '-pb' needs to be followed by <OFFSET> and <8BITSVAL>."} );

         // Get the offset and value, and patch the file
         uint16_t offset = CPCT::to16bitValue(args[i+1]);
         uint8_t   value = CPCT::to8bitValue(args[i+2]);
         thePatcher.patchByte(offset, value);

         // Advance the 2 parameters
         i += 2;
      //----------------------------------------------------------------------------------------------------------
      // PATCH 1 STREAM
      } else if (a == "-ps" || a == "--patch-stream") {
         if (i + 2 >= args.size()) CPCT::error( { "Modifier '-ps' needs to be followed by at least 2 values: <OFFSET> and at least one byte from <BYTETREAM>."} );

         // Get the offset (first argument)
         uint16_t offset = CPCT::to16bitValue(args[i+1]);

         // Get the stream of bytes
         std::string stream;
         for (i += 2; i < args.size(); ++i) {
            // Convert value
            uint8_t val = 0;
            try {
               val = CPCT::to8bitValue(args[i]);
            } catch(std::logic_error& e) {
               // Failed to convert value (it is not a number, stop here)
               --i; // Adjust to ensure that next i considered is this one
               break;
            }
            // Add converted value to stream
            stream.push_back(val);
         }
         // If no byte was pushed to the stream, then an error happened
         if ( stream.size() == 0 ) CPCT::error( { "Modifier '-ps' expects an OFFSET and a BYTESTREAM. After an offset of '", args[i], "', '", args[i+1], "' was found instead of the first byte from the BYTESTREAM." } );

         // Apply patch
         thePatcher.patchStream(offset, stream);

      //----------------------------------------------------------------------------------------------------------
      // PATCH 1 WORD
      } else if (a == "-pw" || a == "--patch-word") {
         if (i + 2 >= args.size()) CPCT::error( { "Modifier '-pw' needs to be followed by <OFFSET> and <16BITSVAL>."} );

         // Get the offset and value, and patch the file
         uint16_t offset = CPCT::to16bitValue(args[i+1]);
         uint16_t  value = CPCT::to16bitValue(args[i+2]);
         thePatcher.patchWord(offset, value);

         // Advance the 2 parameters
         i += 2;

      //----------------------------------------------------------------------------------------------------------
      // UNKNOWN SWITCH
      } else if ( a[0] == '-' ) {
         CPCT::error( {"Unknown switch '", a, "'. Check help with '-h' for valid switches."} );

      //----------------------------------------------------------------------------------------------------------
      // BINARY FILE
      } else {
         // Set the binary file if it has not being set yet
         if ( thePatcher.filename() != "" ) CPCT::error( { "At least two parameters have been given as filenames for binfile: '", thePatcher.filename(), "' and '", a, "'." } );
         thePatcher.setBinFile(a);
      }
   }

   // FINAL CHECKS
   if (args.size()==1) usage(args[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT OF THE SCRIPT
//
int main(int argc, char **argv) {
   // Get and parse commandline arguments
   try {
      CPCT::TArgs args(argv, argv + argc);
      parseArguments(args);

      // Overwrite or print the result
      if ( printResult )   thePatcher.print(std::cout);
      else                 thePatcher.saveFile();
      
   } catch (std::exception& e) {
      std::cerr << "ERROR: " << e.what() << "\n";
      return -1;
   }

   return 0;
}