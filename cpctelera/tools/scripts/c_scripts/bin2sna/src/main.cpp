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
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <cctype>
#include <sstream>
#include <main.h>

typedef std::vector<std::string> TArgs;

// COLORS DEFINITION
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USAGE FUNCTION
//
void usage(const std::string& prg) {
   std::cerr 
      << C_LIGHT_YELLOW << "USAGE" 
      << C_LIGHT_BLUE   << "\n   " << prg << " [OPTIONS] <binfile1> <loadaddress1> [<binfile2> <loadaddress2>...]"
      << C_LIGHT_YELLOW << "\n\nDescription"
      << C_CYAN         << "\n   Creates a runnable 64K snapshot (SNA) file with all \
binfiles loaded in memory at the addresses given. Binary files have to be mandatorily \
provided with their load address, in pairs, separated by spaces. The value of the PC is \
set to 0x0000 by default and may be changed with -pc/--set-pc. Setting the PC lets \
preparing the snapshot to start running from the desired entry point. \
\n\n   The rest of CPC's memory is filled up with standard values from a normal startup \
of an Amstrad CPC 464. These values have been captured by stopping a running 464 and \
copying memory values to this tool. \
\n\n   Currently, this tool only supports 64K snapshots including 464 memory images. "
      << C_LIGHT_YELLOW << "\n\nIMPORTANT CONSIDERATIONS: "
      << C_CYAN         << "\n   - This tool does not check your binaries and load \
addresses for collisions. Files are copied into SNA memory in the order they are provided. \
If one of them reaches the address space of a previous one, it will simply overwrite it \
silently. Similarly, your files may overwrite previous memory contents like firmware \
variables or video memory. "
                        << "\n   - If your software needs to use the firmware or other \
memory contents, take into account that this tool uses a default memory status of \
firmware variables for firmware 1.0 at the start of the system. If you want a different \
setup, you may want to provide it directly with your binaries overwritting default values."
      << C_LIGHT_YELLOW << "\n\nOPTIONS:"
      << C_LIGHT_BLUE   << "\n\n   -pc | --set-pc <ADDRESS> (Default 0x0000)"
      << C_CYAN         << "\n          Sets the value of the PC register at execution start. This value should"
      << " be set to the entry point of the application (its run address)."
      << C_LIGHT_BLUE   << "\n   -h  | --help"
      << C_CYAN         << "\n          Shows this help."
      << "\n\n" << C_NORMAL;

   exit(-1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTION TO THROW RUNTIME ERRORS
//
void error(const TArgs&& err) {
   std::stringstream s;
   for (auto&& e : err) s << e;
   throw std::runtime_error(s.str());
}

//
// Register values according to SNA3 specification
//
enum class TRegister {
   AF  = 0x11,   F   = 0x11,   A   = 0x12,
   BC  = 0x13,   C   = 0x13,   B   = 0x14,
   DE  = 0x15,   E   = 0x15,   D   = 0x16,
   HL  = 0x17,   L   = 0x17,   H   = 0x18, 
   R   = 0x19,   I   = 0x1A,  
   IX  = 0x1D,   IXL = 0x1D,   IXH = 0x1E,
   IY  = 0x1F,   IYL = 0x1F,   IYH = 0x20,
   SP  = 0x21,   SPL = 0x21,   SPH = 0x22,
   PC  = 0x23,   PCL = 0x23,   PCH = 0x24,
   AFp = 0x26,   Fp  = 0x26,   Ap  = 0x27,
   BCp = 0x28,   Cp  = 0x28,   Bp  = 0x29,
   DEp = 0x2A,   Ep  = 0x2A,   Dp  = 0x2B,
   HLp = 0x2C,   Lp  = 0x2C,   Hp  = 0x2D
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SNAPSHOT CLASS
//
//    Contains a memory snapshot
//
struct SNA {
   SNA() {
      // Copy Memory Contents
      std::memcpy(m_memory + 0x0000, gk_mem0000_004F,   80);
      std::memcpy(m_memory + 0x4000, gk_mem4000_4BEF, 3056);
      std::memcpy(m_memory + 0xA670, gk_memA670_BFFF, 6544);
      // Copy Header Contents
      std::memcpy(m_header + 0x0000, gk_sna3_header,   256);
   }

   void setReg(TRegister reg, uint8_t value) {
      uint32_t idx   = static_cast<uint32_t>(reg);
      m_header[idx]  = value;
   }
   void setReg(TRegister reg, uint16_t value) {
      uint32_t idx   = static_cast<uint32_t>(reg);
      m_header[idx]  = static_cast<uint8_t>(value & 0x00FF);
      m_header[idx+1]= static_cast<uint8_t>(value >> 8);
   }
   void loadBinary(const std::string& binfile, uint16_t loadAddress = 0) {
      std::ifstream bin(binfile, std::ios::in | std::ios::binary);
      
      // Check file constraints
      if (!bin.is_open()) error( {"Could not open binary file '", binfile, "' for reading."} );
      bin.ignore( 65537 );
      auto length = bin.gcount();
      if ( length + loadAddress > 65536 ) error ( {"File '", binfile, "' has a size of '", std::to_string(length), "' and cannot be loaded at '", std::to_string(loadAddress) ,"' because it will exceed 64K memory boundaries."} );

      // Rewind file 
      bin.clear();
      bin.seekg( 0, std::ios_base::beg );

      // Insert it into memory
      char byte;
      while(bin.get(byte)) 
         m_memory[loadAddress++] = byte;

      // Close file 
      bin.close();
   }

   void printbin(std::ostream& out) {
      out.write(reinterpret_cast<char*>(m_header), 256);
      out.write(reinterpret_cast<char*>(m_memory), 65536);
   }

private:
   uint8_t  m_memory[65536]   = {0};
   uint8_t  m_header[256]     = {0};
};

SNA theSNA;

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
// PARSES ARGUMENTS GIVEN TO THIS SCRIPT
//
void parseArguments(const TArgs& args) {
   uint16_t nbinaries = 0;
   std::string theBinary {""};
   uint16_t address;

   // PARSE ARGUMENTS ONE BY ONE
   for(std::size_t i=1; i < args.size(); ++i) {
      const std::string& a = args[i];

      // SET THE PC
      if (a == "-pc" || a == "--set-pc") {
         if (i + 1 >= args.size()) error( { "Modifier '-pc' needs to be followed by a 16-bits address, but nothing found."} );

         address = to16bitAddress(args[i+1]);
         theSNA.setReg(TRegister::PC, address);
         ++i;

      // SHOW HELP
      } else if (a == "-h" || a == "--help") {
         usage(args[0]);

      // BINARY FILE FOLLOWED BY LOADADDRESS
      } else {
         if (i + 1 >= args.size()) error( { "File '", a,"' is not followed by its load address. \nBinary files need to be given one by one, separated by spaces, and followed by each of their Loading Addresses. Example: cpct_bin2sna file1 0x0000 file2 0x0500 file3 0xA000" } );
         theBinary = a;
         address = to16bitAddress(args[i+1]);
         theSNA.loadBinary(theBinary, address);
         ++i;
         ++nbinaries;
      }
   }

   // FINAL CHECKS AND PARSING
   if (args.size()==1) usage(args[0]);
   if (nbinaries == 0) error ( {"At least one binary and its loading address is required to create the SNA file."} );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN ENTRY POINT OF THE SCRIPT
//
int main(int argc, char **argv) {
   // Get and parse commandline arguments
   try {
      TArgs args(argv, argv + argc);
      parseArguments(args);
      theSNA.printbin(std::cout);
   } catch (std::exception& e) {
      std::cerr << "ERROR: " << e.what() << "\n";
      return -1;
   }

   return 0;
}