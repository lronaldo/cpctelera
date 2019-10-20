#include <conversors/HConversor.hpp>
#include <iostream>
#include <iomanip>

void 
HConversor::convert_p(std::ostream& out) const {
   uint64_t numchars = m_width*m_height/64;

   // Ouput header information
   outputTextHeader_p(out, "//");

   // Generate declarations
   out << "#define FIRSTUDG_" << m_c_identifier << " " << cast8bit2print(m_firstUDG) << "\n";
   out << "#define NUMUDGs_"  << m_c_identifier << " " << numchars << "\n";
   out << "\n";
   out << "extern " << m_c_identifier << "[8*" << numchars <<"];\n";
}

