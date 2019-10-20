#include <conversors/HSConversor.hpp>
#include <iostream>
#include <iomanip>

void 
HSConversor::convert_p(std::ostream& out) const {
   uint64_t numchars = m_width*m_height/64;

   // Ouput header information
   outputTextHeader_p(out, ";;");

   // Generate declarations
   out << "FIRSTUDG_" << m_c_identifier << " = " << cast8bit2print(m_firstUDG) << "\n";
   out << "NUMUDGs_"  << m_c_identifier << " = " << numchars << "\n";
   out << "SIZE_" << m_c_identifier << " = " << 8*numchars << "\n";
   out << "\n";
   out << ".globl " << m_c_identifier << "\n";
}

