#include <conversors/ASMConversor.hpp>
#include <iostream>
#include <iomanip>

void
ASMConversor::outputCharDefs_p(
      std::ostream& out, const TArrayChar8x8& chardef
   ,  uint16_t charnum) const
{
   uint16_t size = chardef.size();

   out << " .db ";
   out << std::hex << std::setfill('0');
   for (uint8_t i=0; i < size-1; ++i) {
      out << "0x" << std::setw(2) << cast8bit2print(chardef[i]) << ",";
   }
   out << "0x" << std::setw(2) << cast8bit2print(chardef[size-1]);
   out << std::dec << std::setfill(' ');
   out << "  ;; " << std::setw(3) << std::dec << charnum << "\n";
}


void 
ASMConversor::convert_p(std::ostream& out) const {
   uint16_t    sym         = uint16_t(m_firstUDG);
   uint32_t    numchars    = m_width*m_height/64;
   std::string identifier  = m_identifier + ":";

   // Ouput header information
   outputTextHeader_p(out, ";;");

   // Fix identifier
   if ( !m_asmLocalConstant ) identifier += ":";
   if (  m_idPrefix != ' ' )  identifier = m_idPrefix + identifier;

   // Convert characters to ASM
   start_p();
   out << identifier << "\n";
   while(numchars--) {
      outputCharDefs_p( out, getNextChar_p(), sym);
      ++sym;
   }
}

