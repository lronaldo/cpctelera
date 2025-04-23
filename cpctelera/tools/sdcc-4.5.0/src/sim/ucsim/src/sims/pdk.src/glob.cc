/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
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
  { 0x0000, ~0x0000, ' ', 1, "nop" },
  { 0x003A, ~0x0000, ' ', 1, "ret" },
  { 0x0100, ~0x00FF, ' ', 1, "ret %k" },
  { 0x003B, ~0x0000, ' ', 1, "reti" },
  { 0x1700, ~0x00FF, ' ', 1, "mov a,%k" },
  { 0x0080, ~0x001F, ' ', 1, "mov %i,a" },
  { 0x00A0, ~0x001F, ' ', 1, "mov a,%i" },
  { 0x05C0, ~0x003F, ' ', 1, "mov %m,a" },
  { 0x07C0, ~0x003F, ' ', 1, "mov a,%m" },
  { 0x00C1, ~0x001E, ' ', 1, "ldt16 'M5'" },
  { 0x00C0, ~0x001E, ' ', 1, "stt16 'M5'" },
  { 0x00E1, ~0x001E, ' ', 1, "idxm a,'M5'" },
  { 0x00E0, ~0x001E, ' ', 1, "idxm 'M5',a" },
  { 0x09C0, ~0x003F, ' ', 1, "xch %m" },
  { 0x0032, ~0x0000, ' ', 1, "push af" },
  { 0x0033, ~0x0000, ' ', 1, "pop af" },

  { 0x1000, ~0x00FF, ' ', 1, "add %k" },
  { 0x0400, ~0x003F, ' ', 1, "add %m,a" },
  { 0x0600, ~0x003F, ' ', 1, "add a,%m" },
  { 0x1100, ~0x00FF, ' ', 1, "sub %k" },
  { 0x0440, ~0x003F, ' ', 1, "sub %m,a" },
  { 0x0640, ~0x003F, ' ', 1, "sub a,%m" },
  { 0x0010, ~0x0000, ' ', 1, "addc" },
  { 0x0480, ~0x003F, ' ', 1, "addc %m,a" },
  { 0x0680, ~0x003F, ' ', 1, "addc a,%m" },
  { 0x0800, ~0x003F, ' ', 1, "addc %m" },
  { 0x0011, ~0x0000, ' ', 1, "subc" },
  { 0x04C0, ~0x003F, ' ', 1, "subc %m,a" },
  { 0x06C0, ~0x003F, ' ', 1, "subc a,%m" },
  { 0x0840, ~0x007F, ' ', 1, "subc %m" },

  { 0x0900, ~0x003F, ' ', 1, "inc %m" },
  { 0x0940, ~0x003F, ' ', 1, "dec %m" },
  { 0x0980, ~0x003F, ' ', 1, "clear %m" },
  { 0x001A, ~0x0000, ' ', 1, "sr" },
  { 0x0A80, ~0x003F, ' ', 1, "sr %m" },
  { 0x001B, ~0x0000, ' ', 1, "sl" },
  { 0x0AC0, ~0x003F, ' ', 1, "sl %m" },
  { 0x001C, ~0x0000, ' ', 1, "src" },
  { 0x0B00, ~0x003F, ' ', 1, "src %m"},
  { 0x001D, ~0x0000, ' ', 1, "slc" },
  { 0x0B40, ~0x003F, ' ', 1, "slc %m" },

  { 0x1400, ~0x00FF, ' ', 1, "and %k" },
  { 0x0500, ~0x003F, ' ', 1, "and %m,a" },
  { 0x0700, ~0x003F, ' ', 1, "and a,%m" },
  { 0x1500, ~0x00FF, ' ', 1, "or %k" },
  { 0x0540, ~0x003F, ' ', 1, "or %m,a" },
  { 0x0740, ~0x003F, ' ', 1, "or a,%m" },
  { 0x1600, ~0x00FF, ' ', 1, "xor %k" },
  { 0x0580, ~0x003F, ' ', 1, "xor %m,a" },
  { 0x0780, ~0x003F, ' ', 1, "xor a,%m" },
  { 0x0060, ~0x001F, ' ', 1, "xor %i,a" },

  { 0x0018, ~0x0000, ' ', 1, "not" },
  { 0x0A00, ~0x003F, ' ', 1, "not %m" },
  { 0x0019, ~0x0000, ' ', 1, "neg" },
  { 0x0A40, ~0x003F, ' ', 1, "neg %m" },

  { 0x0E00, ~0x00FF, ' ', 1, "set0 %i,%n" },
  { 0x0300, ~0x00EF, ' ', 1, "set0 'M4','n5'" },
  { 0x0F00, ~0x00FF, ' ', 1, "set1 %i,%n" },
  { 0x0310, ~0x00EF, ' ', 1, "set1 'M4',%n" },
  { 0x0C00, ~0x00FF, ' ', 1, "t0sn %i,%n" },
  { 0x0200, ~0x00EF, ' ', 1, "t0sn '4m','5n'" },
  { 0x0D00, ~0x00FF, ' ', 1, "t1sn %i,%n" },
  { 0x0210, ~0x00EF, ' ', 1, "t1sn '4M','5n'" },

  { 0x1200, ~0x00FF, ' ', 1, "ceqsn %k" },
  { 0x0B80, ~0x003F, ' ', 1, "ceqsn %m" },
  { 0x1300, ~0x00FF, ' ', 1, "cneqsn %k" },
  { 0x0BC0, ~0x003F, ' ', 1, "cneqsn %m" },

  { 0x0012, ~0x0000, ' ', 1, "izsn" },
  { 0x0880, ~0x003F, ' ', 1, "izsn %m" },
  { 0x0013, ~0x0000, ' ', 1, "dzsn" },
  { 0x08C0, ~0x003F, ' ', 1, "dzsn %m" },

  { 0x1C00, ~0x03FF, ' ', 1, "call 'a10'" },
  { 0x1800, ~0x03FF, ' ', 1, "goto 'a10'" },

  { 0x001E, ~0x0000, ' ', 1, "swap" },
  { 0x0017, ~0x0000, ' ', 1, "pcadd" },
  { 0x0038, ~0x0000, ' ', 1, "engint" },
  { 0x0039, ~0x0000, ' ', 1, "disint" },
  { 0x0036, ~0x0000, ' ', 1, "stopsys" },
  { 0x0037, ~0x0000, ' ', 1, "stopexe" },
  { 0x0035, ~0x0000, ' ', 1, "reset" },
  { 0x0030, ~0x0000, ' ', 1, "wdreset" },
  { 0x0006, ~0x0000, ' ', 1, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 1, "ldspth" },
  { 0x003C, ~0x0000, ' ', 1, "mul" },

  { 0xFF00, ~0x0000, ' ', 1, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_pdk_14[]= {
  { 0x0000, ~0x0000, ' ', 1, "nop" },
  { 0x007A, ~0x0000, ' ', 1, "ret" },
  { 0x0200, ~0x00FF, ' ', 1, "ret %k" },
  { 0x007B, ~0x0000, ' ', 1, "reti" },
  { 0x2F00, ~0x00FF, ' ', 1, "mov a,%k" },
  { 0x0180, ~0x003F, ' ', 1, "mov %i,a" },
  { 0x01C0, ~0x003F, ' ', 1, "mov a,%i" },
  { 0x0B80, ~0x007F, ' ', 1, "mov %m,a" },
  { 0x0F80, ~0x007F, ' ', 1, "mov a,%m" },
  { 0x0301, ~0x007E, ' ', 1, "ldt16 'M7'" },
  { 0x0300, ~0x007E, ' ', 1, "stt16 'M7'" },
  { 0x0381, ~0x007E, ' ', 1, "idxm a,'M7'" },
  { 0x0380, ~0x007E, ' ', 1, "idxm 'M7',a" },
  { 0x1380, ~0x007F, ' ', 1, "xch %m" },
  { 0x0072, ~0x0000, ' ', 1, "push af" },
  { 0x0073, ~0x0000, ' ', 1, "pop af" },

  { 0x2800, ~0x00FF, ' ', 1, "add %k" },
  { 0x0800, ~0x007F, ' ', 1, "add %m,a" },
  { 0x0C00, ~0x007F, ' ', 1, "add a,%m" },
  { 0x2900, ~0x00FF, ' ', 1, "sub %k" },
  { 0x0880, ~0x007F, ' ', 1, "sub %m,a" },
  { 0x0C80, ~0x007F, ' ', 1, "sub a,%m" },
  { 0x0060, ~0x0000, ' ', 1, "addc" },
  { 0x0900, ~0x007F, ' ', 1, "addc %m,a" },
  { 0x0D00, ~0x007F, ' ', 1, "addc a,%m" },
  { 0x1000, ~0x007F, ' ', 1, "addc %m" },
  { 0x0061, ~0x0000, ' ', 1, "subc" },
  { 0x0980, ~0x007F, ' ', 1, "subc %m,a" },
  { 0x0D80, ~0x007F, ' ', 1, "subc a,%m" },
  { 0x1080, ~0x007F, ' ', 1, "subc %m" },

  { 0x1200, ~0x007F, ' ', 1, "inc %m" },
  { 0x1280, ~0x007F, ' ', 1, "dec %m" },
  { 0x1300, ~0x007F, ' ', 1, "clear %m" },
  { 0x006A, ~0x0000, ' ', 1, "sr" },
  { 0x1500, ~0x007F, ' ', 1, "sr %m" },
  { 0x006B, ~0x0000, ' ', 1, "sl" },
  { 0x1580, ~0x007F, ' ', 1, "sl %m" },
  { 0x006C, ~0x0000, ' ', 1, "src" },
  { 0x1600, ~0x007F, ' ', 1, "src %m"},
  { 0x006D, ~0x0000, ' ', 1, "slc" },
  { 0x1680, ~0x007F, ' ', 1, "slc %m" },

  { 0x2C00, ~0x00FF, ' ', 1, "and %k" },
  { 0x0A00, ~0x007F, ' ', 1, "and %m,a" },
  { 0x0E00, ~0x007F, ' ', 1, "and a,%m" },
  { 0x2D00, ~0x00FF, ' ', 1, "or %k" },
  { 0x0A80, ~0x007F, ' ', 1, "or %m,a" },
  { 0x0E80, ~0x007F, ' ', 1, "or a,%m" },
  { 0x2E00, ~0x00FF, ' ', 1, "xor %k" },
  { 0x0B00, ~0x007F, ' ', 1, "xor %m,a" },
  { 0x0F00, ~0x007F, ' ', 1, "xor a,%m" },
  { 0x00C0, ~0x003F, ' ', 1, "xor %i,a" },

  { 0x0068, ~0x0000, ' ', 1, "not" },
  { 0x1400, ~0x007F, ' ', 1, "not %m" },
  { 0x0069, ~0x0000, ' ', 1, "neg" },
  { 0x1480, ~0x007F, ' ', 1, "neg %m" },

  { 0x1C00, ~0x01FF, ' ', 1, "set0 %i,'n6'" },
  { 0x2400, ~0x01FF, ' ', 1, "set0 'm6','n6'" },
  { 0x1E00, ~0x01FF, ' ', 1, "set1 %i,'n6'" },
  { 0x2600, ~0x01FF, ' ', 1, "set1 'm6','n6'" },
  { 0x1800, ~0x01FF, ' ', 1, "t0sn %i,'n6" },
  { 0x2000, ~0x01FF, ' ', 1, "t0sn 'm6','n6'" },
  { 0x1A00, ~0x01FF, ' ', 1, "t1sn %i,'n6'" },
  { 0x2200, ~0x01FF, ' ', 1, "t1sn 'm6','n6'" },

  { 0x2A00, ~0x00FF, ' ', 1, "ceqsn %k" },
  { 0x1700, ~0x007F, ' ', 1, "ceqsn %m" },
  { 0x2B00, ~0x00FF, ' ', 1, "cneqsn %k" },
  { 0x1780, ~0x007F, ' ', 1, "cneqsn %m" },

  { 0x0062, ~0x0000, ' ', 1, "izsn" },
  { 0x1100, ~0x007F, ' ', 1, "izsn %m" },
  { 0x0063, ~0x0000, ' ', 1, "dzsn" },
  { 0x1180, ~0x007F, ' ', 1, "dzsn %m" },

  { 0x3800, ~0x07FF, ' ', 1, "call 'a11'" },
  { 0x3000, ~0x07FF, ' ', 1, "goto 'a11'" },

  { 0x0600, ~0x007F, ' ', 1, "comp a,%m" },
  { 0x6800, ~0x007F, ' ', 1, "comp %m,a" },
  { 0x0700, ~0x007F, ' ', 1, "nadd a,%m" },
  { 0x0780, ~0x007F, ' ', 1, "nadd %m,a" },

  { 0x006E, ~0x0000, ' ', 1, "swap" },
  { 0x0067, ~0x0000, ' ', 1, "pcadd" },
  { 0x0078, ~0x0000, ' ', 1, "engint" },
  { 0x0079, ~0x0000, ' ', 1, "disint" },
  { 0x0076, ~0x0000, ' ', 1, "stopsys" },
  { 0x0077, ~0x0000, ' ', 1, "stopexe" },
  { 0x0075, ~0x0000, ' ', 1, "reset" },
  { 0x0070, ~0x0000, ' ', 1, "wdreset" },
  { 0x0400, ~0x01FF, ' ', 1, "swapc %i,'n6'" },
  { 0x0006, ~0x0000, ' ', 1, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 1, "ldspth" },
  { 0x007C, ~0x0000, ' ', 1, "mul" },

  { 0xFF00, ~0x0000, ' ', 1, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_pdk_15[]= {
  { 0x0000, ~0x0000, ' ', 1, "nop" },
  { 0x007A, ~0x0000, ' ', 1, "ret" },
  { 0x0200, ~0x00FF, ' ', 1, "ret %k" },
  { 0x007B, ~0x0000, ' ', 1, "reti" },
  { 0x5700, ~0x00FF, ' ', 1, "mov a,%k" },
  { 0x0100, ~0x007F, ' ', 1, "mov %i,a" },
  { 0x0180, ~0x007F, ' ', 1, "mov a,%i" },
  { 0x1700, ~0x00FF, ' ', 1, "mov %m,a" },
  { 0x1F00, ~0x00FF, ' ', 1, "mov a,%m" },
  { 0x0601, ~0x00FE, ' ', 1, "ldt16 'M8'" },
  { 0x0600, ~0x00FE, ' ', 1, "stt16 'M8'" },
  { 0x0701, ~0x00FE, ' ', 1, "idxm a,'M8'" },
  { 0x0700, ~0x00FE, ' ', 1, "idxm 'M8',a" },
  { 0x2700, ~0x00FF, ' ', 1, "xch %m" },
  { 0x0072, ~0x0000, ' ', 1, "push af" },
  { 0x0073, ~0x0000, ' ', 1, "pop af" },

  { 0x5000, ~0x00FF, ' ', 1, "add %k" },
  { 0x1000, ~0x00FF, ' ', 1, "add %m,a" },
  { 0x1800, ~0x00FF, ' ', 1, "add a,%m" },
  { 0x5100, ~0x00FF, ' ', 1, "sub %k" },
  { 0x1100, ~0x00FF, ' ', 1, "sub %m,a" },
  { 0x1900, ~0x00FF, ' ', 1, "sub a,%m" },
  { 0x0060, ~0x0000, ' ', 1, "addc" },
  { 0x1200, ~0x00FF, ' ', 1, "addc %m,a" },
  { 0x1A00, ~0x00FF, ' ', 1, "addc a,%m" },
  { 0x2000, ~0x00FF, ' ', 1, "addc %m" },
  { 0x0061, ~0x0000, ' ', 1, "subc" },
  { 0x1300, ~0x00FF, ' ', 1, "subc %m,a" },
  { 0x1B00, ~0x00FF, ' ', 1, "subc a,%m" },
  { 0x2100, ~0x00FF, ' ', 1, "subc %m" },

  { 0x2400, ~0x00FF, ' ', 1, "inc %m" },
  { 0x2500, ~0x00FF, ' ', 1, "dec %m" },
  { 0x2600, ~0x00FF, ' ', 1, "clear %m" },
  { 0x006A, ~0x0000, ' ', 1, "sr" },
  { 0x2A00, ~0x00FF, ' ', 1, "sr %m" },
  { 0x006B, ~0x0000, ' ', 1, "sl" },
  { 0x2B00, ~0x00FF, ' ', 1, "sl %m" },
  { 0x006C, ~0x0000, ' ', 1, "src" },
  { 0x2C00, ~0x00FF, ' ', 1, "src %m"},
  { 0x006D, ~0x0000, ' ', 1, "slc" },
  { 0x2D00, ~0x00FF, ' ', 1, "slc %m" },

  { 0x5400, ~0x00FF, ' ', 1, "and %k" },
  { 0x1400, ~0x00FF, ' ', 1, "and %m,a" },
  { 0x1C00, ~0x00FF, ' ', 1, "and a,%m" },
  { 0x5500, ~0x00FF, ' ', 1, "or %k" },
  { 0x1500, ~0x00FF, ' ', 1, "or %m,a" },
  { 0x1D00, ~0x00FF, ' ', 1, "or a,%m" },
  { 0x5600, ~0x00FF, ' ', 1, "xor %k" },
  { 0x1600, ~0x00FF, ' ', 1, "xor %m,a" },
  { 0x1E00, ~0x00FF, ' ', 1, "xor a,%m" },
  { 0x0080, ~0x007F, ' ', 1, "xor %i,a" },

  { 0x0068, ~0x0000, ' ', 1, "not" },
  { 0x2800, ~0x00FF, ' ', 1, "not %m" },
  { 0x0069, ~0x0000, ' ', 1, "neg" },
  { 0x2900, ~0x00FF, ' ', 1, "neg %m" },

  { 0x3800, ~0x03FF, ' ', 1, "set0 %i,'n7'" },
  { 0x4800, ~0x03FF, ' ', 1, "set0 'm7','n7'" },
  { 0x3C00, ~0x03FF, ' ', 1, "set1 %i,'n7'" },
  { 0x4C00, ~0x03FF, ' ', 1, "set1 'm7','n7'" },
  { 0x3000, ~0x03FF, ' ', 1, "t0sn %i,'n7'" },
  { 0x4000, ~0x03FF, ' ', 1, "t0sn 'm7,'n7'" },
  { 0x3400, ~0x03FF, ' ', 1, "t1sn %i,'n7'" },
  { 0x4400, ~0x03FF, ' ', 1, "t1sn 'm7,'n7'" },

  { 0x5200, ~0x00FF, ' ', 1, "ceqsn %k" },
  { 0x2E00, ~0x00FF, ' ', 1, "ceqsn %m" },
  { 0x5300, ~0x00FF, ' ', 1, "cneqsn %k" },
  { 0x2F00, ~0x00FF, ' ', 1, "cneqsn %m" },

  { 0x0062, ~0x0000, ' ', 1, "izsn" },
  { 0x2200, ~0x00FF, ' ', 1, "izsn %m" },
  { 0x0063, ~0x0000, ' ', 1, "dzsn" },
  { 0x2300, ~0x00FF, ' ', 1, "dzsn %m" },

  { 0x7000, ~0x0FFF, ' ', 1, "call 'A12'" },
  { 0x6000, ~0x0FFF, ' ', 1, "goto 'A12'" },

  { 0x0C00, ~0x00FF, ' ', 1, "comp a,%m" },
  { 0x0D00, ~0x00FF, ' ', 1, "comp %m,a" },
  { 0x0E00, ~0x00FF, ' ', 1, "nadd a,%m" },
  { 0x0F00, ~0x00FF, ' ', 1, "nadd %m,a" },

  { 0x006E, ~0x0000, ' ', 1, "swap" },
  { 0x0067, ~0x0000, ' ', 1, "pcadd" },
  { 0x0078, ~0x0000, ' ', 1, "engint" },
  { 0x0079, ~0x0000, ' ', 1, "disint" },
  { 0x0076, ~0x0000, ' ', 1, "stopsys" },
  { 0x0077, ~0x0000, ' ', 1, "stopexe" },
  { 0x0075, ~0x0000, ' ', 1, "reset" },
  { 0x0070, ~0x0000, ' ', 1, "wdreset" },
  { 0x5C00, ~0x03FF, ' ', 1, "swapc %i,'n7'" },
  { 0x0006, ~0x0000, ' ', 1, "ldsptl" },
  { 0x0007, ~0x0000, ' ', 1, "ldspth" },
  { 0x007C, ~0x0000, ' ', 1, "mul" },

  { 0x0500, ~0x00FF, ' ', 1, "ldtabl 'M8'" },
  { 0x0501, ~0x00FF, ' ', 1, "ldtabh 'M8'" },

  { 0xFF00, ~0x0000, ' ', 1, "putchar" },
 
  { 0, 0, 0, 0, NULL }
};

struct dis_entry disass_pdk_16[]= {
  { 0x0032, 0xffff, ' ', 1, "push af" },
  { 0x0033, 0xffff, ' ', 1, "pop af" },
  { 0x0010, 0xffff, ' ', 1, "addc a" },
  { 0x0011, 0xffff, ' ', 1, "subc a" },
  { 0x001a, 0xffff, ' ', 1, "sr a" },
  { 0x001c, 0xffff, ' ', 1, "src a" },
  { 0x001b, 0xffff, ' ', 1, "sl a" },
  { 0x001d, 0xffff, ' ', 1, "slc a" },
  { 0x001e, 0xffff, ' ', 1, "swap a" },
  { 0x0018, 0xffff, ' ', 1, "not a" },
  { 0x0019, 0xffff, ' ', 1, "neg a" },
  { 0x003c, 0xffff, ' ', 1, "mul" },
  { 0x0012, 0xffff, ' ', 1, "izsn a" },
  { 0x0013, 0xffff, ' ', 1, "dzsn a" },
  { 0x001f, 0xffff, ' ', 1, "delay a" },
  { 0x003a, 0xffff, ' ', 1, "ret" },
  { 0x003b, 0xffff, ' ', 1, "reti" },
  { 0x0000, 0xffff, ' ', 1, "nop" },
  { 0x0017, 0xffff, ' ', 1, "pcadd a" },
  { 0x0038, 0xffff, ' ', 1, "engint" },
  { 0x0039, 0xffff, ' ', 1, "disgint" },
  { 0x0036, 0xffff, ' ', 1, "stopsys" },
  { 0x0037, 0xffff, ' ', 1, "stopexe" },
  { 0x0035, 0xffff, ' ', 1, "reset" },
  { 0x0030, 0xffff, ' ', 1, "wdtreset" },

  { 0x0070, 0xfff0, ' ', 1, "pushw pc'd4'" },
  { 0x0060, 0xfff0, ' ', 1, "pushw pc'd4'" },

  { 0x0040, 0xffe0, ' ', 1, "pmod 'd5'" },

  { 0x00c0, 0xffc0, ' ', 1, "mov a,%i" },
  { 0x0080, 0xffc0, ' ', 1, "mov %i,a" },
  { 0x1040, 0xffc0, ' ', 1, "xor a,%i" },
  { 0x1000, 0xffc0, ' ', 1, "xor %i,a" },

  { 0x1f00, 0xff00, ' ', 1, "mov a,%k" },
  { 0x1800, 0xff00, ' ', 1, "add a,%k" },
  { 0x1900, 0xff00, ' ', 1, "sub a,%k" },
  { 0x1c00, 0xff00, ' ', 1, "and a,%k" },
  { 0x1d00, 0xff00, ' ', 1, "or a,%k" },
  { 0x1e00, 0xff00, ' ', 1, "xor a,%k" },
  { 0x1a00, 0xff00, ' ', 1, "ceqsn a,%k" },
  { 0x1b00, 0xff00, ' ', 1, "cneqsn a,%k" },
  { 0x0e00, 0xff00, ' ', 1, "delay %k" },
  { 0x0f00, 0xff00, ' ', 1, "ret %k" },

  { 0x5c00, 0xfe00, ' ', 1, "mov %m,a" },
  { 0x5e00, 0xfe00, ' ', 1, "mov a,%m" },
  { 0x3200, 0xfe00, ' ', 1, "nmov %m,a" },
  { 0x3000, 0xfe00, ' ', 1, "nmov a,%m" },
  { 0x0400, 0xfe01, ' ', 1, "popw %M" },
  { 0x0401, 0xfe01, ' ', 1, "pushw %M" },
  { 0x0a00, 0xfe01, ' ', 1, "ltabl %M" },
  { 0x0a01, 0xfe01, ' ', 1, "ltabh %M" },
  { 0x0200, 0xfe01, ' ', 1, "stt16 %M" },
  { 0x0201, 0xfe01, ' ', 1, "ldt16 %M" },
  { 0x6e00, 0xfe00, ' ', 1, "xch %m" },
  { 0x6c00, 0xfe00, ' ', 1, "clear %m" },
  { 0x0800, 0xfe01, ' ', 1, "idxm %M,a" },
  { 0x0801, 0xfe01, ' ', 1, "idxm a,%M" },
  { 0x4200, 0xfe00, ' ', 1, "add a,%m" },
  { 0x4000, 0xfe00, ' ', 1, "add %m,a" },
  { 0x4a00, 0xfe00, ' ', 1, "addc a,%m" },
  { 0x4800, 0xfe00, ' ', 1, "addc %m,a" },
  { 0x6000, 0xfe00, ' ', 1, "addc %m" },
  { 0x3400, 0xfe00, ' ', 1, "nadd a,%m" },
  { 0x3600, 0xfe00, ' ', 1, "nadd %m,a" },
  { 0x4600, 0xfe00, ' ', 1, "sub a,%m" },
  { 0x4400, 0xfe00, ' ', 1, "sub %m,a" },
  { 0x4e00, 0xfe00, ' ', 1, "subc a,%m" },
  { 0x4c00, 0xfe00, ' ', 1, "subc %m,a" },
  { 0x6200, 0xfe00, ' ', 1, "subc %m" },
  { 0x6800, 0xfe00, ' ', 1, "inc %m" },
  { 0x6a00, 0xfe00, ' ', 1, "dec %m" },
  { 0x7400, 0xfe00, ' ', 1, "sr %m" },
  { 0x7800, 0xfe00, ' ', 1, "src %m" },
  { 0x7600, 0xfe00, ' ', 1, "sl %m" },
  { 0x7a00, 0xfe00, ' ', 1, "slc %m" },
  { 0x7c00, 0xfe00, ' ', 1, "swap %m" },
  { 0x5200, 0xfe00, ' ', 1, "and a,%m" },
  { 0x5000, 0xfe00, ' ', 1, "and %m,a" },
  { 0x5600, 0xfe00, ' ', 1, "or a,%m" },
  { 0x5400, 0xfe00, ' ', 1, "or %m,a" },
  { 0x5a00, 0xfe00, ' ', 1, "xor a,%m" },
  { 0x5800, 0xfe00, ' ', 1, "xor %m,a" },
  { 0x7000, 0xfe00, ' ', 1, "not %m" },
  { 0x7200, 0xfe00, ' ', 1, "neg %m" },
  { 0x3c00, 0xfe00, ' ', 1, "comp a,%m" },
  { 0x3e00, 0xfe00, ' ', 1, "comp %m,a" },
  { 0x3800, 0xfe00, ' ', 1, "ceqsn a,%m" },
  { 0x3a00, 0xfe00, ' ', 1, "ceqsn %m,a" },
  { 0x1600, 0xfe00, ' ', 1, "cneqsn a,%m" },
  { 0x1400, 0xfe00, ' ', 1, "cneqsn %m,a" },
  { 0x6400, 0xfe00, ' ', 1, "izsn %m" },
  { 0x6600, 0xfe00, ' ', 1, "dzsn %m" },
  { 0x2400, 0xfe00, ' ', 1, "set0 %i,'n6'" },
  { 0x2600, 0xfe00, ' ', 1, "set1 %i,'n6'" },
  { 0x2e00, 0xfe00, ' ', 1, "swapc %i,'n6'" },
  { 0x2000, 0xfe00, ' ', 1, "t0sn %i,'n6'" },
  { 0x2200, 0xfe00, ' ', 1, "t1sn %i,'n6'" },
  { 0x2a00, 0xfe00, ' ', 1, "wait0 %i,'n6'" },
  { 0x2c00, 0xfe00, ' ', 1, "wait1 %i,'n6'" },
  { 0x0600, 0xfe01, ' ', 1, "igoto %m" },
  { 0x0601, 0xfe01, ' ', 1, "icall %m" },
  { 0x7e00, 0xfe00, ' ', 1, "delay %m" },

  { 0xa000, 0xf000, ' ', 1, "set0 %m,'n9'" },
  { 0xb000, 0xf000, ' ', 1, "set1 %m,'n9'" },
  { 0x8000, 0xf000, ' ', 1, "t0sn %m,'n9'" },
  { 0x9000, 0xf000, ' ', 1, "t1sn %m,'n9'" },

  { 0xe000, 0xe000, ' ', 1, "call 'a13'" },
  { 0xc000, 0xe000, ' ', 1, "goto 'a13'" },

  { 0, 0, 0, 0, NULL }
};

struct cpu_entry cpus_pdk[]=
  {
    {"13"      , CPU_PDK13, 0			, "PDK13"		, ""},
    {"PDK13"   , CPU_PDK13, 0			, "PDK13"		, ""},
    
    {"14"      , CPU_PDK14, 0			, "PDK14"		, ""},
    {"PDK14"   , CPU_PDK14, 0			, "PDK14"		, ""},
    
    {"15"      , CPU_PDK15, 0			, "PDK15"		, ""},
    {"PDK15"   , CPU_PDK15, 0			, "PDK15"		, ""},

    {"16"      , CPU_PDK16, 0			, "PDK16"		, ""},
    {"PDK16"   , CPU_PDK16, 0			, "PDK16"		, ""},

    {NULL, CPU_NONE, 0, "", ""}
  };


/* End of pdk.src/glob.cc */
