#include <conversors/HSConversor.hpp>
#include <iostream>
#include <iomanip>

void 
HSConversor::convert_p(std::ostream& out) const {
   uint64_t numchars = m_width*m_height/64;
   std::string identifier  = m_identifier + ":";

   // Fix identifier
   if ( !m_asmLocalConstant ) identifier += ":";
   if (  m_idPrefix != ' ' )  identifier = m_idPrefix + identifier;

   // Ouput header information
   outputTextHeader_p(out, ";;");

   // Generate declarations
   out << "FIRSTUDG_" << m_identifier << " = " << cast8bit2print(m_firstUDG) << "\n";
   out << "NUMUDGs_"  << m_identifier << " = " << numchars << "\n";
   out << "SIZE_" << m_identifier << " = " << 8*numchars << "\n";
   out << "\n";
   out << ".globl " << identifier << "\n";
}

