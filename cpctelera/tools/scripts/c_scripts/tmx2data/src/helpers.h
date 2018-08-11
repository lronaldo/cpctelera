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

#ifndef __HELPERS_H__
#define __HELPERS_H__

// INCLUDES
#include <vector>
#include <string>
#include <cstdint>

// TYPES
typedef std::vector<std::string> TArgs;

// CONSTANTS
// Colors definitions
const std::string C_LIGHT_RED { "\033[1;31;49m" };
const std::string C_LIGHT_GREEN { "\033[1;32;49m" };
const std::string C_LIGHT_YELLOW { "\033[1;33;49m" };
const std::string C_LIGHT_BLUE { "\033[1;34;49m" };
const std::string C_LIGHT_MAGENTA { "\033[1;35;49m" };
const std::string C_LIGHT_CYAN { "\033[1;36;49m" };
const std::string C_LIGHT_WHITE { "\033[1;37;49m" };
const std::string C_INVERTED_LIGHT_RED { "\033[1;39;41m" };
const std::string C_INVERTED_LIGHT_GREEN { "\033[1;39;42m" };
const std::string C_INVERTED_GREEN { "\033[0;39;42m" };
const std::string C_INVERTED_CYAN { "\033[0;39;46m" };
const std::string C_INVERTED_WHITE { "\033[0;39;47m" };
const std::string C_RED { "\033[0;31;49m" };
const std::string C_GREEN { "\033[0;32;49m" };
const std::string C_MAGENTA { "\033[0;35;49m" };
const std::string C_CYAN { "\033[0;36;49m" };
const std::string C_YELLOW { "\033[0;33;49m" };
const std::string C_WHITE { "\033[0;37;49m" };
const std::string C_NORMAL { "\033[0;39;49m" };

// FUNCTION PROTOTYPES
void     error          (const TArgs&& err);
bool     isValidHex     (const std::string& str);
bool     isValidDec     (const std::string& str);
uint16_t to16bitAddress (const std::string& str);


#endif