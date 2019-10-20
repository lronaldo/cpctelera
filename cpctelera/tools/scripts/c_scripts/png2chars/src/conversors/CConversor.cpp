#include <conversors/CConversor.hpp>
#include <iostream>
#include <iomanip>

void
CConversor::outputCharDefs_p(
      std::ostream& out, const TArrayChar8x8& chardef
   ,  uint16_t charnum, char comma) const 
{
   uint16_t size = chardef.size();

   out << " /* " << std::setw(3) << charnum << " */ " << comma;
   out << std::hex << std::setfill('0');
   for (uint8_t i=0; i < size-1; ++i) {
      out << "0x" << std::setw(2) 
                  << uint16_t(chardef[i]) << ",";
   }
   out   << "0x"  << std::setw(2)
                  << uint16_t(chardef[size-1]) << "\n";
   out << std::dec << std::setfill(' ');
}

void 
CConversor::convert_p(std::ostream& out) const {
   uint16_t sym      = uint16_t(m_firstUDG);
   uint64_t numchars = m_width*m_height/64;

   // Convert characters to C
   start_p();
   out << m_c_identifier << "[" << m_width << "*" << m_height << "] = {\n";
   outputCharDefs_p( out, getNextChar_p(), sym, ' ');
   for (uint32_t i=1; i < numchars; ++i) {
      ++sym;
      outputCharDefs_p( out, getNextChar_p(), sym, ',');
   }
   out << "};\n";
}

