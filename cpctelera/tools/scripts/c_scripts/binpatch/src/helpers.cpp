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
#include <fstream>
#include <cstring>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include "helpers.h"

namespace CPCT {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONCAT STRINGS
//
std::string concatStrings(const TArgs& err) {
   std::stringstream s;
   for (auto& e : err) s << e;
   return s.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS TO THROW EXCEPTIONS OF DIFFERENT TYPES
//
void error        (const TArgs&& err) { throw std::runtime_error     ( concatStrings(err) ); }
void invalid_arg  (const TArgs&& err) { throw std::invalid_argument  ( concatStrings(err) ); }
void out_of_range (const TArgs&& err) { throw std::out_of_range      ( concatStrings(err) ); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECKS IF A GIVEN STRING IS A VALID HEX NUMBER
//
bool isValidHex(const std::string& str) {
   for (auto&& c : str) {
      if ( !std::isdigit(c) ) {
         auto cu = std::toupper(c);
         if (cu < 'A' || cu > 'F')
            return false;
      }
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECKS IF A GIVEN STRING IS A VALID DECIMAL NUMBER
//
bool isValidDec(const std::string& str) {
   for (auto&& c : str) {
      if ( !std::isdigit(c) )
            return false;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECKS IF A GIVEN STRING IS A VALID BINARY NUMBER
//
bool isValidBin(const std::string& str) {
   for (auto&& c : str) {
      if ( c != '0' && c != '1' )
         return false;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STR TO VALUE
//    Converts any integer value (DEC, HEX or BIN) into a 64bit integer.
//
// THROWS: runtime_error (a number, but badly formed), invalid_arg (Could be something different than a number)
//
uint64_t str2Value(const std::string& str) {
   switch (str.size()) {
      case 0: error({"Received an integer value of length 0."});
      case 1:
      case 2:
         // Check if they are valid digits
         if ( !std::isdigit(str[0]) ||
              (str.size() == 2 && !std::isdigit(str[1])) ) invalid_arg( {"integer value expected, but found '", str, "' instead."} );
         // They are valid digits, return address
         return std::stoi(str);
         break;
      default:
         uint64_t value = 0;
         // Check for numerical prefixes
         if ( str[0]=='0' ) { 
            // HEXADECIMAL
            if (str[1]=='x' || str[1]=='X') {
               std::string number = str.substr(2, str.npos);
               if ( ! isValidHex(number) ) error ( {"Invalid hexadecimal value '", str, "'"} ); 
               value = std::stoi(number, 0, 16);
            // BINARY
            } else if (str[1]=='b' || str[1]=='B') {
               std::string number = str.substr(2, str.npos);
               if ( ! isValidBin(number) ) error ( {"Invalid binary value '", str, "'"} ); 
               value = std::stoi(number, 0, 2);
            }
         } else {
            // DECIMAL
            if ( ! isValidDec(str) ) invalid_arg ( {"Invalid decimal value '", str, "'"} ); 
            value = std::stoi(str, 0, 10);
         }

         return value;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO 8bit Value
//    Converts any integer value (DEC, HEX or BIN) into a 8bit integer.
//
// THROWS:    runtime_error (a number, but badly formed)
//          , invalid_arg (Could be something different than a number)
//          , out_of_range (Value is not in the range [0, 255])
//
uint8_t
to8bitValue (const std::string& str) {
   // Convert the value
   uint64_t val = str2Value(str);

   // Ensure it is in the range for an 8 bit value
   if ( val > 255 ) out_of_range ( { "8-bits value expected, but found '", str, "'. Value must be in range [0, 255]" } );

   return (val & 0xFF);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO 16bit Value
//    Converts any integer value (DEC, HEX or BIN) into a 16bit value.
//
// THROWS:    runtime_error (a number, but badly formed)
//          , invalid_arg (Could be something different than a number)
//          , out_of_range (Value is not in the range [0, 65535])
//
uint16_t to16bitValue(const std::string& str) {
   // Convert the value
   uint64_t val = str2Value(str);

   // Ensure it is in the range for an 8 bit value
   if ( val > 65535 ) out_of_range ( { "16-bits value expected, but found '", str, "'. Value must be in range [0, 65535]" } );

   return (val & 0xFFFF);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if a folder exists, is a folder and is readable by the user
//
bool isFolder(const char* folder) {
   struct stat sb;
   return stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Checks if a folder exists, is a folder and the user has write permission
//
bool isFolderWritable(const char* folder) {
   struct stat sb;
   return stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode) && access(folder, W_OK | X_OK) == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Removes all character repetitions from a given string
//
std::string removeRepetitions(const std::string& str, char c) {
   std::string outstr;
   if (str.size() > 0) {
      outstr.reserve(str.size());
      outstr += str[0];
      for(std::size_t i=1; i < str.size(); ++i ) {
         if (str[i] != c || str[i-1] != c)
            outstr += str[i];
      }
   }
   return outstr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ensures that a given string ends up with one repetition of a given character at the end
//
void ensureOnly1CharBack(std::string& str, char endc) {
   char c = endc;
   while(str.size() > 0 && (c = str.back()) == endc) str.pop_back();
   str += endc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove extensions from a filename
//
std::string basename(const std::string& str) {
   auto i = str.find('.');
   if (i != std::string::npos)
      return str.substr(0, i);
   return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove all directory information and leave only the name of the file
//
std::string notdir(const std::string& str, char sep) {
   auto i = str.rfind(sep);
   if (i != std::string::npos)
      return str.substr(i+1, str.length() - i);
   return str;
}

} // End Namespace CPCT