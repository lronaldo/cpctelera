/*
 * Simulator of microcontrollers (gp0m3.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "gp0m3.h"

struct dis_entry disass_p0m3[]=
  {
    { 0x40, 0xff, ' ', 1, "LD B,B" },
    { 0x41, 0xff, ' ', 1, "LD B,C" },
    { 0x43, 0xff, ' ', 1, "LD B,E" },
    { 0x44, 0xff, ' ', 1, "LD B,H" },
    { 0x49, 0xff, ' ', 1, "LD C,C" },
    { 0x4a, 0xff, ' ', 1, "LD C,D" },
    { 0x4b, 0xff, ' ', 1, "LD C,E" },

    { 0x52, 0xff, ' ', 1, "LD D,D" },
    { 0x53, 0xff, ' ', 1, "LD D,E" },
    { 0x58, 0xff, ' ', 1, "LD E,B" },
    { 0x59, 0xff, ' ', 1, "LD E,C" },
    { 0x5a, 0xff, ' ', 1, "LD E,D" },
    { 0x5c, 0xff, ' ', 1, "LD E,H" },
    { 0x5d, 0xff, ' ', 1, "LD E,L" },

    { 0x64, 0xff, ' ', 1, "LD H,H" },
    { 0x68, 0xff, ' ', 1, "LD L,B" },
    { 0x69, 0xff, ' ', 1, "LD L,C" },
    { 0x6a, 0xff, ' ', 1, "LD L,D" },
    { 0x6b, 0xff, ' ', 1, "LD L,E" },
    { 0x6c, 0xff, ' ', 1, "LD L,H" },

    { 0x80, 0xff, ' ', 1, "ADD A,B" },
    { 0x88, 0xff, ' ', 1, "ADC A,B" },

    { 0x90, 0xff, ' ', 1, "SUB A,B" },

    { 0x42, 0xff, ' ', 1, "LD B,D" },
    { 0x45, 0xff, ' ', 1, "LD B,L" },
    { 0x48, 0xff, ' ', 1, "LD C,B" },
    { 0x4c, 0xff, ' ', 1, "LD C,H" },
    { 0x4d, 0xff, ' ', 1, "LD C,L" },

    { 0x50, 0xff, ' ', 1, "LD D,B" },
    { 0x51, 0xff, ' ', 1, "LD D,C" },
    { 0x54, 0xff, ' ', 1, "LD D,H" },
    { 0x55, 0xff, ' ', 1, "LD D,L" },

    { 0x60, 0xff, ' ', 1, "LD H,B" },
    { 0x61, 0xff, ' ', 1, "LD H,C" },
    { 0x62, 0xff, ' ', 1, "LD H,D" },
    { 0x63, 0xff, ' ', 1, "LD H,E" },
    { 0x65, 0xff, ' ', 1, "LD H,L" },
    { 0x6d, 0xff, ' ', 1, "LD L,L" },

    { 0x7f, 0xff, ' ', 1, "LD A,A" },

    { 0x81, 0xff, ' ', 1, "ADD A,C" },
    { 0x82, 0xff, ' ', 1, "ADD A,D" },
    { 0x83, 0xff, ' ', 1, "ADD A,E" },
    { 0x84, 0xff, ' ', 1, "ADD A,H" },
    { 0x85, 0xff, ' ', 1, "ADD A,L" },
    { 0x86, 0xff, ' ', 1, "ADD A,(HL)" },
    { 0x87, 0xff, ' ', 1, "ADD A,A" },
    { 0x89, 0xff, ' ', 1, "ADC A,C" },
    { 0x8a, 0xff, ' ', 1, "ADC A,D" },
    { 0x8b, 0xff, ' ', 1, "ADC A,E" },
    { 0x8c, 0xff, ' ', 1, "ADC A,H" },
    { 0x8d, 0xff, ' ', 1, "ADC A,L" },
    { 0x8e, 0xff, ' ', 1, "ADC A,(HL)" },
    { 0x8f, 0xff, ' ', 1, "ADC A,A" },
    
    { 0x91, 0xff, ' ', 1, "SUB A,C" },
    { 0x92, 0xff, ' ', 1, "SUB A,D" },
    { 0x93, 0xff, ' ', 1, "SUB A,E" },
    { 0x94, 0xff, ' ', 1, "SUB A,H" },
    { 0x95, 0xff, ' ', 1, "SUB A,L" },
    { 0x96, 0xff, ' ', 1, "SUB A,(HL)" },
    { 0x97, 0xff, ' ', 1, "SUB A,A" },
    { 0x98, 0xff, ' ', 1, "SBC A,B" },
    { 0x99, 0xff, ' ', 1, "SBC A,C" },
    { 0x9a, 0xff, ' ', 1, "SBC A,D" },
    { 0x9b, 0xff, ' ', 1, "SBC A,E" },
    { 0x9c, 0xff, ' ', 1, "SBC A,H" },
    { 0x9d, 0xff, ' ', 1, "SBC A,L" },
    { 0x9e, 0xff, ' ', 1, "SBC A,(HL)" },
    { 0x9f, 0xff, ' ', 1, "SBC A,A" },
    
    { 0xa0, 0xff, ' ', 1, "AND A,B" },
    { 0xa1, 0xff, ' ', 1, "AND A,C" },
    { 0xa2, 0xff, ' ', 1, "AND A,D" },
    { 0xa3, 0xff, ' ', 1, "AND A,E" },
    { 0xa4, 0xff, ' ', 1, "AND A,H" },
    { 0xa5, 0xff, ' ', 1, "AND A,L" },
    { 0xa6, 0xff, ' ', 1, "AND A,(HL)" },
    { 0xa7, 0xff, ' ', 1, "AND A,A" },
    { 0xa8, 0xff, ' ', 1, "XOR A,B" },
    { 0xa9, 0xff, ' ', 1, "XOR A,C" },
    { 0xaa, 0xff, ' ', 1, "XOR A,D" },
    { 0xab, 0xff, ' ', 1, "XOR A,E" },
    { 0xac, 0xff, ' ', 1, "XOR A,H" },
    { 0xad, 0xff, ' ', 1, "XOR A,L" },
    { 0xae, 0xff, ' ', 1, "XOR A,(HL)" },

    { 0xb0, 0xff, ' ', 1, "OR A,B" },
    { 0xb1, 0xff, ' ', 1, "OR A,C" },
    { 0xb2, 0xff, ' ', 1, "OR A,D" },
    { 0xb3, 0xff, ' ', 1, "OR A,E" },
    { 0xb4, 0xff, ' ', 1, "OR A,H" },
    { 0xb5, 0xff, ' ', 1, "OR A,L" },
    { 0xb6, 0xff, ' ', 1, "OR A,(HL)" },
    { 0xb8, 0xff, ' ', 1, "CP A,B" },
    { 0xb9, 0xff, ' ', 1, "CP A,C" },
    { 0xba, 0xff, ' ', 1, "CP A,D" },
    { 0xbb, 0xff, ' ', 1, "CP A,E" },
    { 0xbc, 0xff, ' ', 1, "CP A,H" },
    { 0xbd, 0xff, ' ', 1, "CP A,L" },
    { 0xbe, 0xff, ' ', 1, "CP A,(HL)" },
    { 0xbf, 0xff, ' ', 1, "CP A,A" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gp0m3.cc */
