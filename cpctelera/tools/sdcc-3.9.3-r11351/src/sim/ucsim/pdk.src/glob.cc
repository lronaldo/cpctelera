/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#include <stdio.h>

#include "stypes.h"

/* PDK14 instructions described in PFS154. */


/*
%k - immediate addressing
%m - memory addressing
%mn - memory addressing when using N-bit
%i - IO addressing 
%in - IO addressing when using N-bit
%n - N-bit addressing
*/

/*  uint  code, mask;  char  branch;  uchar length;  char  *mnemonic; */

struct dis_entry disass_pdk_13[]= {
  { 0x0000, ~0x0000, ' ', 2, "nop" },
  { 0x003A, ~0x0000, ' ', 2, "ret" },
  { 0x0100, ~0x00FF, ' ', 2, "ret %k" },
  { 0x003B, ~0x0000, ' ', 2, "reti" },
  { 0x1700, ~0x00FF, ' ', 2, "mov a, %k" },
  { 0x0080, ~0x001F, ' ', 2, "mov %i, a" },
  { 0x00A0, ~0x001F, ' ', 2, "mov a, %i" },
  { 0x05C0, ~0x003F, ' ', 2, "mov %m, a" },
  { 0x07C0, ~0x003F, ' ', 2, "mov a, %m" },
  { 0x00C1, ~0x001E, ' ', 2, "ldt16 %m" },
  { 0x00C0, ~0x001E, ' ', 2, "stt16 %m" },
  { 0x00E1, ~0x001E, ' ', 2, "idxm a, %m" },
  { 0x00E0, ~0x001E, ' ', 2, "idxm %m, a" },
  { 0x09C0, ~0x003F, ' ', 2, "xch %m" },
  { 0x0032, ~0x0000, ' ', 2, "push af" },
  { 0x0033, ~0x0000, ' ', 2, "pop af" },

  { 0x1000, ~0x00FF, ' ', 2, "add %k" },
  { 0x0400, ~0x003F, ' ', 2, "add %m, a" },
  { 0x0600, ~0x003F, ' ', 2, "add a, %m" },
  { 0x1100, ~0x00FF, ' ', 2, "sub %k" },
  { 0x0440, ~0x003F, ' ', 2, "sub %m, a" },
  { 0x0640, ~0x003F, ' ', 2, "sub a, %m" },
  { 0x0010, ~0x0000, ' ', 2, "addc" },
  { 0x0480, ~0x003F, ' ', 2, "addc %m, a" },
  { 0x0680, ~0x003F, ' ', 2, "addc a, %m" },
  { 0x0800, ~0x003F, ' ', 2, "addc %m" },
  { 0x0011, ~0x0000, ' ', 2, "subc" },
  { 0x04C0, ~0x003F, ' ', 2, "subc %m, a" },
  { 0x06C0, ~0x003F, ' ', 2, "subc a, %m" },
  { 0x0840, ~0x007F, ' ', 2, "subc %m" },

  { 0x0900, ~0x003F, ' ', 2, "inc %m" },
  { 0x0940, ~0x003F, ' ', 2, "dec %m" },
  { 0x0980, ~0x003F, ' ', 2, "clear %m" },
  { 0x001A, ~0x0000, ' ', 2, "sr" },
  { 0x0A80, ~0x003F, ' ', 2, "sr %m" },
  { 0x001B, ~0x0000, ' ', 2, "sl" },
  { 0x0AC0, ~0x003F, ' ', 2, "sl %m" },
  { 0x001C, ~0x0000, ' ', 2, "src" },
  { 0x0B00, ~0x003F, ' ', 2, "src %m"},
  { 0x001D, ~0x0000, ' ', 2, "slc" },
  { 0x0B40, ~0x003F, ' ', 2, "slc %m" },

  { 0x1400, ~0x00FF, ' ', 2, "and %k" },
  { 0x0500, ~0x003F, ' ', 2, "and %m, a" },
  { 0x0700, ~0x003F, ' ', 2, "and a, %m" },
  { 0x1500, ~0x00FF, ' ', 2, "or %k" },
  { 0x0540, ~0x003F, ' ', 2, "or %m, a" },
  { 0x0740, ~0x003F, ' ', 2, "or a, %m" },
  { 0x1600, ~0x00FF, ' ', 2, "xor %k" },
  { 0x0580, ~0x003F, ' ', 2, "xor %m, a" },
  { 0x0780, ~0x003F, ' ', 2, "xor a, %m" },
  { 0x0060, ~0x001F, ' ', 2, "xor %i, a" },

  { 0x0018, ~0x0000, ' ', 2, "not" },
  { 0x0A00, ~0x003F, ' ', 2, "not %m" },
  { 0x0019, ~0x0000, ' ', 2, "neg" },
  { 0x0A40, ~0x003F, ' ', 2, "neg %m" },

  { 0x0E00, ~0x00FF, ' ', 2, "set0 %in, %n" },
  { 0x0300, ~0x00FE, ' ', 2, "set0 %mn, %n" },
  { 0x0F00, ~0x00FF, ' ', 2, "set1 %in, %n" },
  { 0x0301, ~0x00FE, ' ', 2, "set1 %mn, %n" },
  { 0x0C00, ~0x00FF, ' ', 2, "t0sn %in, %n" },
  { 0x0200, ~0x00FE, ' ', 2, "t0sn %mn, %n" },
  { 0x0D00, ~0x00FF, ' ', 2, "t1sn %in, %n" },
  { 0x0201, ~0x00FE, ' ', 2, "t1sn %mn, %n" },

  { 0x1200, ~0x00FF, ' ', 2, "ceqsn %k" },
  { 0x0B80, ~0x003F, ' ', 2, "ceqsn %m" },
  { 0x1300, ~0x00FF, ' ', 2, "cneqsn %k" },
  { 0x0BC0, ~0x003F, ' ', 2, "cneqsn %m" },

  { 0x0012, ~0x0000, ' ', 2, "izsn" },
  { 0x0880, ~0x003F, ' ', 2, "izsn %m" },
  { 0x0013, ~0x0000, ' ', 2, "dzsn" },
  { 0x08C0, ~0x003F, ' ', 2, "dzsn %m" },

  { 0x1C00, ~0x03FF, ' ', 2, "call %k" },
  { 0x1800, ~0x03FF, ' ', 2, "goto %k" },

  { 0x001E, ~0x0000, ' ', 2, "swap" },
  { 0x0017, ~0x0000, ' ', 2, "pcadd" },
  { 0x0038, ~0x0000, ' ', 2, "engint" },
  { 0x0039, ~0x0000, ' ', 2, "disint" },
  { 0x0036, ~0x0000, ' ', 2, "stopsys" },
  { 0x0037, ~0x0000, ' ', 2, "stopexe" },
  { 0x0035, ~0x0000, ' ', 2, "reset" },
  { 0x0030, ~0x0000, ' ', 2, "wdreset" },
  { 0x0006, ~0x0000, ' ', 2, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 2, "ldspth" },
  { 0x003C, ~0x0000, ' ', 2, "mul" },

  { 0xFF00, ~0x0000, ' ', 2, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_pdk_14[]= {
  { 0x0000, ~0x0000, ' ', 2, "nop" },
  { 0x007A, ~0x0000, ' ', 2, "ret" },
  { 0x0200, ~0x00FF, ' ', 2, "ret %k" },
  { 0x007B, ~0x0000, ' ', 2, "reti" },
  { 0x2F00, ~0x00FF, ' ', 2, "mov a, %k" },
  { 0x0180, ~0x003F, ' ', 2, "mov %i, a" },
  { 0x01C0, ~0x003F, ' ', 2, "mov a, %i" },
  { 0x0B80, ~0x007F, ' ', 2, "mov %m, a" },
  { 0x0F80, ~0x007F, ' ', 2, "mov a, %m" },
  { 0x0301, ~0x007E, ' ', 2, "ldt16 %m" },
  { 0x0300, ~0x007E, ' ', 2, "stt16 %m" },
  { 0x0381, ~0x007E, ' ', 2, "idxm a, %m" },
  { 0x0380, ~0x007E, ' ', 2, "idxm %m, a" },
  { 0x1380, ~0x007F, ' ', 2, "xch %m" },
  { 0x0072, ~0x0000, ' ', 2, "push af" },
  { 0x0073, ~0x0000, ' ', 2, "pop af" },

  { 0x2800, ~0x00FF, ' ', 2, "add %k" },
  { 0x0800, ~0x007F, ' ', 2, "add %m, a" },
  { 0x0C00, ~0x007F, ' ', 2, "add a, %m" },
  { 0x2900, ~0x00FF, ' ', 2, "sub %k" },
  { 0x0880, ~0x007F, ' ', 2, "sub %m, a" },
  { 0x0C80, ~0x007F, ' ', 2, "sub a, %m" },
  { 0x0060, ~0x0000, ' ', 2, "addc" },
  { 0x0900, ~0x007F, ' ', 2, "addc %m, a" },
  { 0x0D00, ~0x007F, ' ', 2, "addc a, %m" },
  { 0x1000, ~0x007F, ' ', 2, "addc %m" },
  { 0x0061, ~0x0000, ' ', 2, "subc" },
  { 0x0980, ~0x007F, ' ', 2, "subc %m, a" },
  { 0x0D80, ~0x007F, ' ', 2, "subc a, %m" },
  { 0x1080, ~0x007F, ' ', 2, "subc %m" },

  { 0x1200, ~0x007F, ' ', 2, "inc %m" },
  { 0x1280, ~0x007F, ' ', 2, "dec %m" },
  { 0x1300, ~0x007F, ' ', 2, "clear %m" },
  { 0x006A, ~0x0000, ' ', 2, "sr" },
  { 0x1500, ~0x007F, ' ', 2, "sr %m" },
  { 0x006B, ~0x0000, ' ', 2, "sl" },
  { 0x1580, ~0x007F, ' ', 2, "sl %m" },
  { 0x006C, ~0x0000, ' ', 2, "src" },
  { 0x1600, ~0x007F, ' ', 2, "src %m"},
  { 0x006D, ~0x0000, ' ', 2, "slc" },
  { 0x1680, ~0x007F, ' ', 2, "slc %m" },

  { 0x2C00, ~0x00FF, ' ', 2, "and %k" },
  { 0x0A00, ~0x007F, ' ', 2, "and %m, a" },
  { 0x0E00, ~0x007F, ' ', 2, "and a, %m" },
  { 0x2D00, ~0x00FF, ' ', 2, "or %k" },
  { 0x0A80, ~0x007F, ' ', 2, "or %m, a" },
  { 0x0E80, ~0x007F, ' ', 2, "or a, %m" },
  { 0x2E00, ~0x00FF, ' ', 2, "xor %k" },
  { 0x0B00, ~0x007F, ' ', 2, "xor %m, a" },
  { 0x0F00, ~0x007F, ' ', 2, "xor a, %m" },
  { 0x00C0, ~0x003F, ' ', 2, "xor %i, a" },

  { 0x0068, ~0x0000, ' ', 2, "not" },
  { 0x1400, ~0x007F, ' ', 2, "not %m" },
  { 0x0069, ~0x0000, ' ', 2, "neg" },
  { 0x1480, ~0x007F, ' ', 2, "neg %m" },

  { 0x1C00, ~0x01FF, ' ', 2, "set0 %in, %n" },
  { 0x2400, ~0x01FF, ' ', 2, "set0 %mn, %n" },
  { 0x1E00, ~0x01FF, ' ', 2, "set1 %in, %n" },
  { 0x2600, ~0x01FF, ' ', 2, "set1 %mn, %n" },
  { 0x1800, ~0x01FF, ' ', 2, "t0sn %in, %n" },
  { 0x2000, ~0x01FF, ' ', 2, "t0sn %mn, %n" },
  { 0x1A00, ~0x01FF, ' ', 2, "t1sn %in, %n" },
  { 0x2200, ~0x01FF, ' ', 2, "t1sn %mn, %n" },

  { 0x2A00, ~0x00FF, ' ', 2, "ceqsn %k" },
  { 0x1700, ~0x007F, ' ', 2, "ceqsn %m" },
  { 0x2B00, ~0x00FF, ' ', 2, "cneqsn %k" },
  { 0x1780, ~0x007F, ' ', 2, "cneqsn %m" },

  { 0x0062, ~0x0000, ' ', 2, "izsn" },
  { 0x1100, ~0x007F, ' ', 2, "izsn %m" },
  { 0x0063, ~0x0000, ' ', 2, "dzsn" },
  { 0x1180, ~0x007F, ' ', 2, "dzsn %m" },

  { 0x3800, ~0x07FF, ' ', 2, "call %k" },
  { 0x3000, ~0x07FF, ' ', 2, "goto %k" },

  { 0x0600, ~0x007F, ' ', 2, "comp a, %m" },
  { 0x6800, ~0x007F, ' ', 2, "comp %m, a" },
  { 0x0700, ~0x007F, ' ', 2, "nadd a, %m" },
  { 0x0780, ~0x007F, ' ', 2, "nadd %m, a" },

  { 0x006E, ~0x0000, ' ', 2, "swap" },
  { 0x0067, ~0x0000, ' ', 2, "pcadd" },
  { 0x0078, ~0x0000, ' ', 2, "engint" },
  { 0x0079, ~0x0000, ' ', 2, "disint" },
  { 0x0076, ~0x0000, ' ', 2, "stopsys" },
  { 0x0077, ~0x0000, ' ', 2, "stopexe" },
  { 0x0075, ~0x0000, ' ', 2, "reset" },
  { 0x0070, ~0x0000, ' ', 2, "wdreset" },
  { 0x0400, ~0x01FF, ' ', 2, "swapc %in, %n" },
  { 0x0006, ~0x0000, ' ', 2, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 2, "ldspth" },
  { 0x007C, ~0x0000, ' ', 2, "mul" },

  { 0xFF00, ~0x0000, ' ', 2, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_pdk_15[]= {
  { 0x0000, ~0x0000, ' ', 2, "nop" },
  { 0x007A, ~0x0000, ' ', 2, "ret" },
  { 0x0200, ~0x00FF, ' ', 2, "ret %k" },
  { 0x007B, ~0x0000, ' ', 2, "reti" },
  { 0x5700, ~0x00FF, ' ', 2, "mov a, %k" },
  { 0x0100, ~0x007F, ' ', 2, "mov %i, a" },
  { 0x0180, ~0x007F, ' ', 2, "mov a, %i" },
  { 0x1700, ~0x00FF, ' ', 2, "mov %m, a" },
  { 0x1F00, ~0x00FF, ' ', 2, "mov a, %m" },
  { 0x0601, ~0x00FE, ' ', 2, "ldt16 %m" },
  { 0x0600, ~0x00FE, ' ', 2, "stt16 %m" },
  { 0x0701, ~0x00FE, ' ', 2, "idxm a, %m" },
  { 0x0700, ~0x00FE, ' ', 2, "idxm %m, a" },
  { 0x2700, ~0x00FF, ' ', 2, "xch %m" },
  { 0x0072, ~0x0000, ' ', 2, "push af" },
  { 0x0073, ~0x0000, ' ', 2, "pop af" },

  { 0x5000, ~0x00FF, ' ', 2, "add %k" },
  { 0x1000, ~0x00FF, ' ', 2, "add %m, a" },
  { 0x1800, ~0x00FF, ' ', 2, "add a, %m" },
  { 0x5100, ~0x00FF, ' ', 2, "sub %k" },
  { 0x1100, ~0x00FF, ' ', 2, "sub %m, a" },
  { 0x1900, ~0x00FF, ' ', 2, "sub a, %m" },
  { 0x0060, ~0x0000, ' ', 2, "addc" },
  { 0x1200, ~0x00FF, ' ', 2, "addc %m, a" },
  { 0x1A00, ~0x00FF, ' ', 2, "addc a, %m" },
  { 0x2000, ~0x00FF, ' ', 2, "addc %m" },
  { 0x0061, ~0x0000, ' ', 2, "subc" },
  { 0x1300, ~0x00FF, ' ', 2, "subc %m, a" },
  { 0x1B00, ~0x00FF, ' ', 2, "subc a, %m" },
  { 0x2100, ~0x00FF, ' ', 2, "subc %m" },

  { 0x2400, ~0x00FF, ' ', 2, "inc %m" },
  { 0x2500, ~0x00FF, ' ', 2, "dec %m" },
  { 0x2600, ~0x00FF, ' ', 2, "clear %m" },
  { 0x006A, ~0x0000, ' ', 2, "sr" },
  { 0x2A00, ~0x00FF, ' ', 2, "sr %m" },
  { 0x006B, ~0x0000, ' ', 2, "sl" },
  { 0x2B00, ~0x00FF, ' ', 2, "sl %m" },
  { 0x006C, ~0x0000, ' ', 2, "src" },
  { 0x2C00, ~0x00FF, ' ', 2, "src %m"},
  { 0x006D, ~0x0000, ' ', 2, "slc" },
  { 0x2D00, ~0x00FF, ' ', 2, "slc %m" },

  { 0x5400, ~0x00FF, ' ', 2, "and %k" },
  { 0x1400, ~0x00FF, ' ', 2, "and %m, a" },
  { 0x1C00, ~0x00FF, ' ', 2, "and a, %m" },
  { 0x5500, ~0x00FF, ' ', 2, "or %k" },
  { 0x1500, ~0x00FF, ' ', 2, "or %m, a" },
  { 0x1D00, ~0x00FF, ' ', 2, "or a, %m" },
  { 0x5600, ~0x00FF, ' ', 2, "xor %k" },
  { 0x1600, ~0x00FF, ' ', 2, "xor %m, a" },
  { 0x1E00, ~0x00FF, ' ', 2, "xor a, %m" },
  { 0x0080, ~0x007F, ' ', 2, "xor %i, a" },

  { 0x0068, ~0x0000, ' ', 2, "not" },
  { 0x2800, ~0x00FF, ' ', 2, "not %m" },
  { 0x0069, ~0x0000, ' ', 2, "neg" },
  { 0x2900, ~0x00FF, ' ', 2, "neg %m" },

  { 0x3800, ~0x03FF, ' ', 2, "set0 %in, %n" },
  { 0x4800, ~0x03FF, ' ', 2, "set0 %mn, %n" },
  { 0x3C00, ~0x03FF, ' ', 2, "set1 %in, %n" },
  { 0x4C00, ~0x03FF, ' ', 2, "set1 %mn, %n" },
  { 0x3000, ~0x03FF, ' ', 2, "t0sn %in, %n" },
  { 0x4000, ~0x03FF, ' ', 2, "t0sn %mn, %n" },
  { 0x3400, ~0x03FF, ' ', 2, "t1sn %in, %n" },
  { 0x4400, ~0x03FF, ' ', 2, "t1sn %mn, %n" },

  { 0x5200, ~0x00FF, ' ', 2, "ceqsn %k" },
  { 0x2E00, ~0x00FF, ' ', 2, "ceqsn %m" },
  { 0x5300, ~0x00FF, ' ', 2, "cneqsn %k" },
  { 0x2F00, ~0x00FF, ' ', 2, "cneqsn %m" },

  { 0x0062, ~0x0000, ' ', 2, "izsn" },
  { 0x2200, ~0x00FF, ' ', 2, "izsn %m" },
  { 0x0063, ~0x0000, ' ', 2, "dzsn" },
  { 0x2300, ~0x00FF, ' ', 2, "dzsn %m" },

  { 0x7000, ~0x0FFF, ' ', 2, "call %k" },
  { 0x6000, ~0x0FFF, ' ', 2, "goto %k" },

  { 0x0C00, ~0x00FF, ' ', 2, "comp a, %m" },
  { 0x0D00, ~0x00FF, ' ', 2, "comp %m, a" },
  { 0x0E00, ~0x00FF, ' ', 2, "nadd a, %m" },
  { 0x0F00, ~0x00FF, ' ', 2, "nadd %m, a" },

  { 0x006E, ~0x0000, ' ', 2, "swap" },
  { 0x0067, ~0x0000, ' ', 2, "pcadd" },
  { 0x0078, ~0x0000, ' ', 2, "engint" },
  { 0x0079, ~0x0000, ' ', 2, "disint" },
  { 0x0076, ~0x0000, ' ', 2, "stopsys" },
  { 0x0077, ~0x0000, ' ', 2, "stopexe" },
  { 0x0075, ~0x0000, ' ', 2, "reset" },
  { 0x0070, ~0x0000, ' ', 2, "wdreset" },
  { 0x5C00, ~0x03FF, ' ', 2, "swapc %in, %n" },
  { 0x0006, ~0x0000, ' ', 2, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 2, "ldspth" },
  { 0x007C, ~0x0000, ' ', 2, "mul" },

  { 0x0500, ~0x00FF, ' ', 2, "ldtabl %m" },
  { 0x0501, ~0x00FF, ' ', 2, "ldtabh %m" },

  { 0xFF00, ~0x0000, ' ', 2, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};


/* glob.cc */
