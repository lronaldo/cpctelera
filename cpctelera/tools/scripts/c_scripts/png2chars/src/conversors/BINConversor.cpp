#include <conversors/BINConversor.hpp>
#include <iostream>
#include <iomanip>

void 
BINConversor::convert_p(std::ostream& out) const {
   uint32_t numchars = m_width*m_height/64;

   // We assume ouput is set to be a binary

   // Convert characters to BINARY
   start_p();
   while(numchars--) {
      const auto& chardef = getNextChar_p();
      for (const auto& v : chardef)
         out.write(&v, sizeof(v));
   }
}

