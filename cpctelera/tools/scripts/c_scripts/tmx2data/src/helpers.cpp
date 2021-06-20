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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION TO THROW RUNTIME ERRORS
//
void error(const TArgs&& err) {
   std::stringstream s;
   for (auto&& e : err) s << e;
   throw std::runtime_error(s.str());
}

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
// CONVERT A 16-BIT ADDRESS (DEC OR HEX) AS STRING INTO ITS UINT16_T VALUE
//
uint16_t to16bitAddress(const std::string& str) {
   switch (str.size()) {
      case 0: error({"Received a 16-bits address of length 0."});
      case 1:
      case 2:
         // Check if they are valid digits
         if ( !std::isdigit(str[0]) ||
              (str.size() == 2 && !std::isdigit(str[1])) ) error( {"16-bits address expected, but found '", str, "' instead."} ); 
         // They are valid digits, return address
         return std::stoi(str);
         break;
      default:
         int32_t address;
         // Check for numerical prefixes
         std::string prefix = str.substr(0, 2);
         if (prefix == "0x" || prefix == "0X") {
            std::string number = str.substr(2, str.npos);
            if ( ! isValidHex(number) ) error ( {"Invalid 16-bits hexadecimal value for address '", str, "'"} ); 
            address = std::stoi(str, 0, 16);
         } else {
            if ( ! isValidDec(str) ) error ( {"Invalid 16-bits decimal value for address '", str, "'"} ); 
            address = std::stoi(str, 0, 10);
         }

         // Check that it is a valid address
         if (address < 0 || address > 65535) error( {"16-bits address '", str, "' is out of range."} ); 

         return address;
   }
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
   auto i = str.find_last_of('.');
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
