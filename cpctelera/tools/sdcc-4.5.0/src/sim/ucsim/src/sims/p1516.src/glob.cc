/*
 * Simulator of microcontrollers (glob.cc)
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

#include <stdio.h>

#include "glob.h"


struct dis_entry disass_p1516[]=
  {
   { 0x08000000, 0x08000000, 'c', 1, "call  %A", true },

   { 0xb4f00000, 0xfff00000, 'M', 1, "jz    %O", false },
   { 0x94f00000, 0xfff00000, 'M', 1, "jnz   %O", false },
   { 0x74f00000, 0xfff00000, 'M', 1, "jc    %O", false },
   { 0x54f00000, 0xfff00000, 'M', 1, "jnc   %O", false },
   { 0x04f00000, 0x1ff00000, 'M', 1, "jmp   %O", false },
   { 0x04f00000, 0x0ff00000, ' ', 1, "jump  %O", false },

   { 0x00000000, 0x0f000000, ' ', 1, "nop", false },
   { 0x01000000, 0x0f000000, ' ', 1, "ld    %d:=mem[%R]", false },
   { 0x02000000, 0x0f000000, ' ', 1, "st    mem[%R]:=%d", false },
   { 0x03fe0000, 0x0fff0000, ' ', 1, "ret", false },
   { 0x03000000, 0x0f000000, ' ', 1, "mov   %d:=%a", false },
   { 0x04000000, 0x0f000000, ' ', 1, "ldl0  %d:=%0", false },
   { 0x05000000, 0x0f000000, ' ', 1, "ldl   %d:=%l", false },
   { 0x06000000, 0x0f000000, ' ', 1, "ldh   %d:=%h", false },

   { 0x07000000, 0x0f000f80, ' ', 1, "add   %d:=%a+%b", false },
   { 0x07000080, 0x0f000f80, ' ', 1, "addc  %d:=%a+%b", false },
   { 0x07000100, 0x0f000f80, ' ', 1, "sub   %d:=%a-%b", false },
   { 0x07000180, 0x0f000f80, ' ', 1, "sbb   %d:=%a-%b", false },
   { 0x07000200, 0x0f000f80, ' ', 1, "inc   %d:=%a+1", false },
   { 0x07000280, 0x0f000f80, ' ', 1, "dec   %d:=%a-1", false },
   { 0x07000300, 0x0f000f80, ' ', 1, "and   %d:=%a&%b", false },
   { 0x07000380, 0x0f000f80, ' ', 1, "or    %d:=%a|%b", false },
   { 0x07000400, 0x0f000f80, ' ', 1, "xor   %d:=%a^%b", false },
   { 0x07000480, 0x0f000f80, ' ', 1, "shl   %d:=u(%a)<<1", false },
   { 0x07000500, 0x0f000f80, ' ', 1, "shr   %d:=u(%a)>>1", false },
   { 0x07000800, 0x0f000f80, ' ', 1, "sha   %d:=s(%a)>>1", false },
   { 0x07000580, 0x0f000f80, ' ', 1, "rol   %d:=(C,%a)<<1", false },
   { 0x07000600, 0x0f000f80, ' ', 1, "ror   %d:=(%a,C)>>1", false },
   { 0x07000680, 0x0f000f80, ' ', 1, "mul   %d:=%a*%b", false },
   { 0x07000700, 0x0f000f80, ' ', 1, "div   %d:=%a/%b", false },
   { 0x07000780, 0x0f000f80, ' ', 1, "cmp   F:=%a-%b", false },
   { 0x07000880, 0x0f000f80, ' ', 1, "setc", false },
   { 0x07000900, 0x0f000f80, ' ', 1, "clrc", false },
   
   { 0, 0, 0, 0, 0, 0 }
  };

struct dis_entry disass_p2223[]=
  {
   // UNCONDITIONAL
   { 0xf4000000, 0xff000000, ' ', 1, "ces 'ar'", true },
   { 0xf5000000, 0xff000000, ' ', 1, "ces %d,'s20'", true },

   // Macro
   { 0x00000000, 0x0fffffff, ' ', 1, "nop", false },
   { 0x00000000, 0x0fffff00, ' ', 1, "'char8'", false },
   { 0x00f00e00, 0x00f00f00, ' ', 1, "ret", false },
   { 0x11f20000, 0xffff0000, ' ', 1, "jz 'j'", false },
   { 0x21f20000, 0xffff0000, ' ', 1, "jnz 'j'", false },
   { 0x31f20000, 0xffff0000, ' ', 1, "jc 'j'", false },
   { 0x41f20000, 0xffff0000, ' ', 1, "jnc 'j'", false },
   { 0x01f20000, 0x0fff0000, ' ', 1, "jmp 'j'", false },
   { 0x0d0d0000, 0x0f0f0000, ' ', 1, "push %d", false },
   { 0x0f0d0000, 0x0f0f0000, ' ', 1, "pop %d", false },
   { 0x00f00000, 0x0fff0000, ' ', 1, "jp 'jp'", false },
   { 0x01040001, 0x0f0fffff, ' ', 1, "inc %d", false },
   { 0x0104ffff, 0x0f0fffff, ' ', 1, "dec %d", false },
    
   // CALL
   { 0x04000000, 0x0f000000, ' ', 1, "call 'ar'", true },
   { 0x05000000, 0x0f000000, ' ', 1, "call %d,'s20'", true },

   // ALU 1op
   { 0x02000000, 0x0e0f0000, ' ', 1, "zeb %d", false },
   { 0x02010000, 0x0e0f0000, ' ', 1, "zew %d", false },
   { 0x02020000, 0x0e0f0000, ' ', 1, "seb %d", false },
   { 0x02030000, 0x0e0f0000, ' ', 1, "sew %d", false },
   { 0x02040000, 0x0e0f0000, ' ', 1, "not %d:=~%d", false },
   { 0x02050000, 0x0e0f0000, ' ', 1, "neg %d:=-%d", false },
   { 0x02060000, 0x0e0f0000, ' ', 1, "ror %d:=(%d,C)>>1", false },
   { 0x02070000, 0x0e0f0000, ' ', 1, "rol %d:=(C,%d)<<1", false },
   { 0x02080000, 0x0e0f0000, ' ', 1, "shl %d:=u(%d)<<1", false },
   { 0x02090000, 0x0e0f0000, ' ', 1, "shr %d:=u(%d)>>1", false },
   { 0x020a0000, 0x0e0f0000, ' ', 1, "sha %d:=s(%d)>>1", false },
   { 0x020b0000, 0x0e0f0000, ' ', 1, "sz F.SZ=%d", false },
   { 0x020c0000, 0x0e0f0000, ' ', 1, "sec F.C=1", false },
   { 0x020d0000, 0x0e0f0000, ' ', 1, "clc F.C=0", false },
   { 0x020e0000, 0x0e0f0000, ' ', 1, "getf %d:=F", false },
   { 0x020f0000, 0x0e0f0000, ' ', 1, "setf F:=%d", false },

   // ALU R
   { 0x00000000, 0x0f0f0000, ' ', 1, "mov %d:=%b", false },
   { 0x00030000, 0x0f0f0000, ' ', 1, "sed %d:=s(%b)", false },
   { 0x00040000, 0x0f0f0000, ' ', 1, "add %d:=%d+%b", false },
   { 0x00050000, 0x0f0f0000, ' ', 1, "adc %d:=%d+%b", false },
   { 0x00060000, 0x0f0f0000, ' ', 1, "sub %d:=%d-%b", false },
   { 0x00070000, 0x0f0f0000, ' ', 1, "sbb %d:=%d-%b", false },
   { 0x00080000, 0x0f0f0000, ' ', 1, "cmp F:=%d-%b", false },
   { 0x00090000, 0x0f0f0000, ' ', 1, "mul %d:=%d*%b", false },
   { 0x000a0000, 0x0f0f0000, ' ', 1, "plus %d:=%d+%b", false },
   { 0x000b0000, 0x0f0f0000, ' ', 1, "btst F:=%d&%b", false },
   { 0x000c0000, 0x0f0f0000, ' ', 1, "test F:=%d&%b", false },
   { 0x000d0000, 0x0f0f0000, ' ', 1, "or %d:=%d&%b", false },
   { 0x000e0000, 0x0f0f0000, ' ', 1, "xor %d:=%d^%b", false },
   { 0x000f0000, 0x0f0f0000, ' ', 1, "and %d:=%d&%b", false },

   // ALU #immed
   { 0x01000000, 0x0f0f0000, ' ', 1, "mvl %d:=%l", false },
   { 0x01010000, 0x0f0f0000, ' ', 1, "mvh %d:=%h", false },
   { 0x01020000, 0x0f0f0000, ' ', 1, "mvzl %d:=%0", false },
   { 0x01030000, 0x0f0f0000, ' ', 1, "mvs %d:=#'s16'", false },
   { 0x01040000, 0x0f0f0000, ' ', 1, "add %d:=%d+#'s16'", false },
   { 0x01050000, 0x0f0f0000, ' ', 1, "adc %d:=%d+#'s16'", false },
   { 0x01060000, 0x0f0f0000, ' ', 1, "sub %d:=%d-#'s16'", false },
   { 0x01070000, 0x0f0f0000, ' ', 1, "sbb %d:=%d-#'s16'", false },
   { 0x01080000, 0x0f0f0000, ' ', 1, "cmp F:=%d-#'s16'", false },
   { 0x01090000, 0x0f0f0000, ' ', 1, "mul %d:=%d*#'s16'", false },
   { 0x010a0000, 0x0f0f0000, ' ', 1, "plus %d:=%d+#'s16'", false },
   { 0x010b0000, 0x0f0f0000, ' ', 1, "btst F:=%d&#'u16'", false },
   { 0x010c0000, 0x0f0f0000, ' ', 1, "test F:=%d&#'u16'", false },
   { 0x010d0000, 0x0f0f0000, ' ', 1, "or %d:=%d|#'u16'", false },
   { 0x010e0000, 0x0f0f0000, ' ', 1, "xor %d:=%d^#'u16'", false },
   { 0x010f0000, 0x0f0f0000, ' ', 1, "and %d:=%d&#'and16'", false },

   // MEM
   { 0x08000000, 0x0f008000, ' ', 1, "st mem[%a,%b]:=%d", false },
   { 0x09000000, 0x0f00c000, ' ', 1, "st mem[%a-,%b]:=%d", false },
   { 0x09004000, 0x0f00c000, ' ', 1, "st mem[-%a,%b]:=%d", false },
   { 0x09008000, 0x0f00c000, ' ', 1, "st mem[%a+,%b]:=%d", false },
   { 0x0900c000, 0x0f00c000, ' ', 1, "st mem[+%a,%b]:=%d", false },

   { 0x0a000000, 0x0f000000, ' ', 1, "ld %d:=mem[%a,%b]", false },
   { 0x0b000000, 0x0f00c000, ' ', 1, "ld %d:=mem[%a-,%b]", false },
   { 0x0b004000, 0x0f00c000, ' ', 1, "ld %d:=mem[-%a,%b]", false },
   { 0x0b008000, 0x0f00c000, ' ', 1, "ld %d:=mem[%a+,%b]", false },
   { 0x0b00c000, 0x0f00c000, ' ', 1, "ld %d:=mem[+%a,%b]", false },

   { 0x0c000000, 0x0f000000, ' ', 1, "st mem[%a,'s16']:=%d", false },
   { 0x0d000000, 0x0f000000, ' ', 1, "st mem['*ra','s16']:=%d", false },
   { 0x0e000000, 0x0f000000, ' ', 1, "ld %d:=mem[%a,'s16']", false },
   { 0x0f000000, 0x0f000000, ' ', 1, "ld %d:=mem['*ra','s16']", false },

   // EXT
   { 0x06000000, 0x0f0f0000, ' ', 1, "st mem['u16']:=%d", false },
   { 0x07000000, 0x0f0f0000, ' ', 1, "ld %d:=mem['u16']", false },
   { 0x06014000, 0x0f0fe000, ' ', 1, "getbz %d:=%b['ri0']", false },
   { 0x0601c000, 0x0f0fe000, ' ', 1, "getbz %d:=%b['u2']", false },
   { 0x06016000, 0x0f0fe000, ' ', 1, "getbs %d:=%b['ri0']", false },
   { 0x0601e000, 0x0f0fe000, ' ', 1, "getbs %d:=%b['u2']", false },
   { 0x06010000, 0x0f0fc000, ' ', 1, "getb %d:=%b['ri0']", false },
   { 0x06018000, 0x0f0fc000, ' ', 1, "getb %d:=%b['u2']", false },
   { 0x07010000, 0x0f0f8000, ' ', 1, "putb %d:=%b['ri0']", false },
   { 0x07018000, 0x0f0f8000, ' ', 1, "putb %d:=%b['u2']", false },
   { 0x06020000, 0x0f0f0000, ' ', 1, "rds %d:='sfr'", false },
   { 0x07020000, 0x0f0f0000, ' ', 1, "wrs 'sfr':=%d", false },
   
   { 0, 0, 0, 0, 0, 0 }
  };

struct cpu_entry cpus_p1516[]=
  {
    { "P1516"		, CPU_P1516, 0, "P1516", "" },
    { "1"		, CPU_P1516, 0, "P1516", "" },
    { "5"		, CPU_P1516, 0, "P1516", "" },
    { "6"		, CPU_P1516, 0, "P1516", "" },
    { "P2223"		, CPU_P2223, 0, "P2223", "" },
    { "2"		, CPU_P2223, 0, "P2223", "" },
    { "3"		, CPU_P2223, 0, "P2223", "" },

    {NULL, CPU_NONE, 0, "", ""}
  };

/* End of p1516.src/glob.cc */
