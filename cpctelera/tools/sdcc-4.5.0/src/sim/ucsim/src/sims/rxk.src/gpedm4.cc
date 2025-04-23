/*
 * Simulator of microcontrollers (gpedm4.cc)
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

#include "gpedm4.h"

struct dis_entry disass_pedm4[]=
  {
    { 0x00, 0xff, ' ', 3, "CBM %b" },
    { 0x01, 0xff, ' ', 2, "LD PW,(HTR+HL)" },
    { 0x11, 0xff, ' ', 2, "LD PX,(HTR+HL)" },
    { 0x21, 0xff, ' ', 2, "LD PY,(HTR+HL)" },
    { 0x31, 0xff, ' ', 2, "LD PZ,(HTR+HL)" },
    { 0x02, 0xff, ' ', 2, "SBOX A" },
    { 0x12, 0xff, ' ', 2, "IBOX A" },
    { 0x10, 0xff, ' ', 3, "DWJNZ %r" },
    { 0x48, 0xff, ' ', 2, "CP HL,DE" },
    { 0x4c, 0xff, ' ', 2, "TEST BC" },
    { 0xa2, 0xff, ' ', 6, "LLJP GT,%X,%w" },
    { 0xaa, 0xff, ' ', 6, "LLJP GTU,%X,%w" },
    { 0xb2, 0xff, ' ', 6, "LLJP LT,%X,%w" },
    { 0xba, 0xff, ' ', 6, "LLJP V,%X,%w" },
    { 0xc2, 0xff, ' ', 6, "LLJP NZ,%X,%w" },
    { 0xca, 0xff, ' ', 6, "LLJP Z,%X,%w" },
    { 0xd2, 0xff, ' ', 6, "LLJP NC,%X,%w" },
    { 0xda, 0xff, ' ', 6, "LLJP C,%X,%w" },
    { 0xa5, 0xff, ' ', 4, "PUSH %w" },
    { 0xa3, 0xff, ' ', 4, "JRE GT,%R" },
    { 0xb3, 0xff, ' ', 4, "JRE LT,%R" },
    { 0xab, 0xff, ' ', 4, "JRE GTU,%R" },
    { 0xbb, 0xff, ' ', 4, "JRE V,%R" },
    { 0xc3, 0xff, ' ', 4, "JRE NZ,%R" },
    { 0xcb, 0xff, ' ', 4, "JRE Z,%R" },
    { 0xd3, 0xff, ' ', 4, "JRE NC,%R" },
    { 0xdb, 0xff, ' ', 4, "JRE C,%R" },
    { 0xc4, 0xff, ' ', 2, "FLAG NZ,HL" },
    { 0xcc, 0xff, ' ', 2, "FLAG Z,HL" },
    { 0xd4, 0xff, ' ', 2, "FLAG NC,HL" },
    { 0xdc, 0xff, ' ', 2, "FLAG C,HL" },
    { 0xa4, 0xff, ' ', 2, "FLAG GT,HL" },
    { 0xb4, 0xff, ' ', 2, "FLAG LT,HL" },
    { 0xac, 0xff, ' ', 2, "FLAG GTU,HL" },
    { 0xbc, 0xff, ' ', 2, "FLAG V,HL" },
    { 0xea, 0xff, ' ', 2, "CALL (HL)" },
    { 0x8b, 0xff, ' ', 2, "LLRET" },
    { 0xd9, 0xff, ' ', 2, "EXP" },
    { 0x40, 0xff, ' ', 2, "LD HTR,A" },
    { 0x50, 0xff, ' ', 2, "LD A,HTR" },
    { 0x55, 0xff, ' ', 2, "SCALL" },
    { 0x83, 0xff, ' ', 2, "SRET" },
    { 0xb5, 0xff, ' ', 4, "SETUSRP %w" },
    { 0xb1, 0xff, ' ', 4, "SETSYSP %w" },
    { 0xfa, 0xff, ' ', 2, "LLCALL (JKHL)" },
    { 0x03, 0xff, ' ', 2, "LDL PW,(SP+%b)" },
    { 0x13, 0xff, ' ', 2, "LDL PX,(SP+%b)" },
    { 0x23, 0xff, ' ', 2, "LDL PY,(SP+%b)" },
    { 0x33, 0xff, ' ', 2, "LDL PZ,(SP+%b)" },
    { 0x04, 0xff, ' ', 2, "LD PW,(SP+%b)" },
    { 0x14, 0xff, ' ', 2, "LD PX,(SP+%b)" },
    { 0x24, 0xff, ' ', 2, "LD PY,(SP+%b)" },
    { 0x34, 0xff, ' ', 2, "LD PZ,(SP+%b)" },
    { 0x05, 0xff, ' ', 2, "LD (SP+%b),PW" },
    { 0x15, 0xff, ' ', 2, "LD (SP+%b),PX" },
    { 0x25, 0xff, ' ', 2, "LD (SP+%b),PY" },
    { 0x35, 0xff, ' ', 2, "LD (SP+%b),PZ" },
    { 0x06, 0xff, ' ', 2, "LD HL,(PW+BC)" },
    { 0x16, 0xff, ' ', 2, "LD HL,(PX+BC)" },
    { 0x26, 0xff, ' ', 2, "LD HL,(PY+BC)" },
    { 0x36, 0xff, ' ', 2, "LD HL,(PZ+BC)" },
    { 0x07, 0xff, ' ', 2, "LD (PW+BC),HL" },
    { 0x17, 0xff, ' ', 2, "LD (PX+BC),HL" },
    { 0x27, 0xff, ' ', 2, "LD (PY+BC),HL" },
    { 0x37, 0xff, ' ', 2, "LD (PZ+BC),HL" },
    { 0x08, 0xff, ' ', 5, "LDF PW,(%l)" },
    { 0x18, 0xff, ' ', 5, "LDF PX,(%l)" },
    { 0x28, 0xff, ' ', 5, "LDF PY,(%l)" },
    { 0x38, 0xff, ' ', 5, "LDF PZ,(%l)" },
    { 0x09, 0xff, ' ', 5, "LDF (%l),PW" },
    { 0x19, 0xff, ' ', 5, "LDF (%l),PX" },
    { 0x29, 0xff, ' ', 5, "LDF (%l),PY" },
    { 0x39, 0xff, ' ', 5, "LDF (%l),PZ" },
    { 0x08, 0xff, ' ', 5, "LDF BC,(%l)" },
    { 0x18, 0xff, ' ', 5, "LDF DE,(%l)" },
    { 0x28, 0xff, ' ', 5, "LDF IX,(%l)" },
    { 0x38, 0xff, ' ', 5, "LDF IY,(%l)" },
    { 0x0b, 0xff, ' ', 5, "LDF (%l),BC" },
    { 0x1b, 0xff, ' ', 5, "LDF (%l),DE" },
    { 0x2b, 0xff, ' ', 5, "LDF (%l),IX" },
    { 0x3b, 0xff, ' ', 5, "LDF (%l),IY" },
    { 0x0c, 0xff, ' ', 6, "LD PW,%x" },
    { 0x1c, 0xff, ' ', 6, "LD PX,%x" },
    { 0x2c, 0xff, ' ', 6, "LD PW,%x" },
    { 0x3c, 0xff, ' ', 6, "LD PZ,%x" },
    { 0x0d, 0xff, ' ', 4, "LDL PW,%w" },
    { 0x1d, 0xff, ' ', 4, "LDL PX,%w" },
    { 0x2d, 0xff, ' ', 4, "LDL PW,%w" },
    { 0x3d, 0xff, ' ', 4, "LDL PZ,%w" },
    { 0x0e, 0xff, ' ', 2, "CONVC PW" },
    { 0x1e, 0xff, ' ', 2, "CONVC PX" },
    { 0x2e, 0xff, ' ', 2, "CONVC PY" },
    { 0x3e, 0xff, ' ', 2, "CONVC PZ" },
    { 0x0f, 0xff, ' ', 2, "CONVD PW" },
    { 0x1f, 0xff, ' ', 2, "CONVD PX" },
    { 0x2f, 0xff, ' ', 2, "CONVD PY" },
    { 0x3f, 0xff, ' ', 2, "CONVD PZ" },
    { 0x58, 0xff, ' ', 2, "CP JKHL,BCDE" },
    { 0x74, 0xff, ' ', 2, "EX BC',HL" }, //'
    { 0x7c, 0xff, ' ', 2, "EX JK',HL" }, //'
    { 0x80, 0xff, ' ', 2, "COPY" },
    { 0x88, 0xff, ' ', 2, "COPYR" },
    { 0xc1, 0xff, ' ', 2, "POP PW" },
    { 0xd1, 0xff, ' ', 2, "POP PX" },
    { 0xe1, 0xff, ' ', 2, "POP PY" },
    { 0xf1, 0xff, ' ', 2, "POP PZ" },
    { 0xc5, 0xff, ' ', 2, "PUSH PW" },
    { 0xd5, 0xff, ' ', 2, "PUSH PX" },
    { 0xe5, 0xff, ' ', 2, "PUSH PY" },
    { 0xf5, 0xff, ' ', 2, "PUSH PZ" },
    { 0xc6, 0xff, ' ', 2, "ADD JKHL,BCDE" },
    { 0xd6, 0xff, ' ', 2, "SUB JKHL,BCDE" },
    { 0xe6, 0xff, ' ', 2, "AND JKHL,BCDE" },
    { 0xf6, 0xff, ' ', 2, "OR JKHL,BCDE" },
    { 0xee, 0xff, ' ', 2, "XOR JKHL,BCDE" },
    { 0xfe, 0xff, ' ', 2, "LD HL,(SP+HL)" },

    { 0, 0, 0, 0, 0, 0, 0 }
  };
  
/* End of rxk.src/gpedm4.cc */
