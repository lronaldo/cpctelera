#include <conversors/TerminalTestDrawConversor.hpp>
#include <iostream>
#include <iomanip>

void
TerminalTestDrawConversor::drawChar8x8(std::ostream& out, const TArrayChar8x8& chardef) const {
  // Get first 8x8 character and draw it
   out << "/--------\\\n";
   for (auto row : chardef) {
      out << "|";
      for(uint8_t i=0; i < 8; ++i) {
         out << ((row & 0x80) ? '#' : ' ');
         row <<= 1;
      }
      out << "|\n";
   }
   out << "\\--------/\n";
}

void 
TerminalTestDrawConversor::convert_p(std::ostream& out) const {
   uint32_t numchars = m_width*m_height/64;

   // Convert characters to BASIC
   start_p();
   while(numchars--)
      drawChar8x8( out, getNextChar_p());
}

