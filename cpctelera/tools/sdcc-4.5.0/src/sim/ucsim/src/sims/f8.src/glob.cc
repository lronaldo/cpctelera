/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "f8cl.h"

#include "glob.h"

instruction_wrapper_fn itab[256];

u8_t ptab[256]= {
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

// code mask branch len mn call tick
struct dis_entry disass_f8[]=
  {
    // move
    { 0x80, 0xff, ' ', 2, "ld %a,#'i8'" },
    { 0x81, 0xff, ' ', 3, "ld %a,'a16_8'" },
    { 0x82, 0xff, ' ', 2, "ld %a,('nsp_8')" },
    { 0x83, 0xff, ' ', 3, "ld %a,('nnz_8')" },
    { 0x84, 0xff, ' ', 1, "ld %a,('y_8')" },
    { 0x85, 0xff, ' ', 2, "ld %a,('ny_8')" },
    { 0x86, 0xff, ' ', 1, "ld %a,xh" },
    { 0x87, 0xff, ' ', 1, "ld %a,yl" },
    { 0x88, 0xff, ' ', 1, "ld %a,yh" },
    { 0x89, 0xff, ' ', 1, "ld %a,zl" },
    { 0x8a, 0xff, ' ', 1, "ld %a,zh" },
    { 0x8b, 0xff, ' ', 3, "ld 'a16_8',%a" },
    { 0x8c, 0xff, ' ', 2, "ld ('nsp_8'),%a" },
    { 0x8d, 0xff, ' ', 3, "ld ('nnz_8'),%a" },
    { 0x8e, 0xff, ' ', 1, "ld ('y_8'),%a" },
    { 0x8f, 0xff, ' ', 2, "ld ('ny_8'),%a" },
    { 0x94, 0xff, ' ', 2, "ld yh,#'i8'" },
    
    { 0xc0, 0xff, ' ', 3, "ldw %A,#'i16'" },
    { 0xc1, 0xff, ' ', 3, "ldw %A,'a16_16'" },
    { 0xc2, 0xff, ' ', 2, "ldw %A,('nsp_16')" },
    { 0xc3, 0xff, ' ', 3, "ldw %A,('nnz_16')" },
    { 0xc4, 0xff, ' ', 2, "ldw %A,('ny_16')" },
    { 0xc5, 0xff, ' ', 1, "ldw %A,(%A)" },
    { 0xc6, 0xff, ' ', 1, "ldw %A,x" },
    { 0xc7, 0xff, ' ', 2, "ldw %A,#%d" },
    { 0xc8, 0xff, ' ', 3, "ldw 'a16_16',%A" },
    { 0xc9, 0xff, ' ', 2, "ldw ('nsp_16'),%A" },
    { 0xca, 0xff, ' ', 3, "ldw ('nnz_16'),%A" },
    { 0xcb, 0xff, ' ', 1, "ldw x,%A" },
    { 0xcc, 0xff, ' ', 1, "ldw z,%A" },
    { 0xcd, 0xff, ' ', 1, "ldw (%A),%R" },
    { 0xce, 0xff, ' ', 2, "ldw ('nA_16'),%R" },
    { 0x70, 0xff, ' ', 1, "ldw %A, sp" },
    { 0x74, 0xff, ' ', 1, "ldw ('dsp_16'),%A" },
    { 0xdc, 0xff, ' ', 2, "ldw %A,z" },
    { 0xde, 0xff, ' ', 2, "ldw %R,(%A)" },

    { 0xed, 0xff, ' ', 2, "ldi ('ny_8'),(z)" },
    { 0xcf, 0xff, ' ', 2, "ldwi ('ny16'),(z)" },

    { 0x60, 0xff, ' ', 3, "push 'a16_8'" },
    { 0x61, 0xff, ' ', 2, "push ('nsp_8')" },
    { 0x62, 0xff, ' ', 1, "push %a" },
    { 0x63, 0xff, ' ', 1, "push ('ny_8')" },
    { 0x90, 0xff, ' ', 2, "push #'i8'" },

    { 0xb0, 0xff, ' ', 3, "pushw 'a16_16'" },
    { 0xb1, 0xff, ' ', 2, "pushw ('nsp_16')" },
    { 0xb2, 0xff, ' ', 3, "pushw ('nnz_16')" },
    { 0xb3, 0xff, ' ', 1, "pushw %A" },
    { 0xe8, 0xff, ' ', 3, "pushw #'i16'" },

    { 0x99, 0xff, ' ', 1, "pop %a" },
    { 0xe9, 0xff, ' ', 1, "popw %A" },

    { 0x91, 0xff, ' ', 2, "xch %a,('nsp_8')" },
    { 0x92, 0xff, ' ', 1, "xch %a,('y_8')" },
    { 0x93, 0xff, ' ', 1, "xch %L,%H" },
    { 0xf4, 0xff, ' ', 1, "xchw %R,('%A')" },
    { 0xf5, 0xff, ' ', 1, "xchw %A,('nsp_16')" },

    { 0x9b, 0xff, ' ', 1, "cax ('y_8'),xh,yl" },
    { 0xf9, 0xff, ' ', 1, "caxw ('y_16'),z,x" },

    { 0x58, 0xff, ' ', 3, "clr 'a16_8'" },
    { 0x59, 0xff, ' ', 2, "clr ('nsp_8')" },
    { 0x5a, 0xff, ' ', 1, "clr %a" },
    { 0x5b, 0xff, ' ', 1, "clr zh" },
    { 0xa0, 0xff, ' ', 3, "clrw 'a16_16'" },
    { 0xa1, 0xff, ' ', 2, "clrw ('nsp_16')" },
    { 0xa2, 0xff, ' ', 3, "clrw ('nnz_16')" },
    { 0xa3, 0xff, ' ', 1, "clrw %A" },

    { 0x68, 0xff, ' ', 3, "xchb %a,'a16_b0',#0" },
    { 0x69, 0xff, ' ', 3, "xchb %a,'a16_b1',#1" },
    { 0x6a, 0xff, ' ', 3, "xchb %a,'a16_b2',#2" },
    { 0x6b, 0xff, ' ', 3, "xchb %a,'a16_b3',#3" },
    { 0x6c, 0xff, ' ', 3, "xchb %a,'a16_b4',#4" },
    { 0x6d, 0xff, ' ', 3, "xchb %a,'a16_b5',#5" },
    { 0x6e, 0xff, ' ', 3, "xchb %a,'a16_b6',#6" },
    { 0x6f, 0xff, ' ', 3, "xchb %a,'a16_b7',#7" },

    // alu
    { 0x10, 0xff, ' ', 2, "add %a,#'i8'" },
    { 0x11, 0xff, ' ', 3, "add %a,'a16_8'" },
    { 0x12, 0xff, ' ', 2, "add %a,('nsp_8')" },
    { 0x13, 0xff, ' ', 3, "add %a,('nnz_8')" },
    { 0x14, 0xff, ' ', 1, "add %a,zl" },
    { 0x15, 0xff, ' ', 1, "add %a,xh" },
    { 0x16, 0xff, ' ', 1, "add %a,yl" },
    { 0x17, 0xff, ' ', 1, "add %a,yh" },
    { 0x18, 0xff, ' ', 2, "adc %a,#'i8'" },
    { 0x19, 0xff, ' ', 3, "adc %a,'a16_8'" },
    { 0x1a, 0xff, ' ', 2, "adc %a,('nsp_8')" },
    { 0x1b, 0xff, ' ', 3, "adc %a,('nnz_8')" },
    { 0x1c, 0xff, ' ', 1, "adc %a,zl" },
    { 0x1d, 0xff, ' ', 1, "adc %a,xh" },
    { 0x1e, 0xff, ' ', 1, "adc %a,yl" },
    { 0x1f, 0xff, ' ', 1, "adc %a,yh" },

    { 0x01, 0xff, ' ', 3, "sub %a,'a16_8'" },
    { 0x02, 0xff, ' ', 2, "sub %a,('nsp_8')" },
    { 0x03, 0xff, ' ', 3, "sub %a,('nnz_8')" },
    { 0x04, 0xff, ' ', 1, "sub %a,zl" },
    { 0x05, 0xff, ' ', 1, "sub %a,xh" },
    { 0x06, 0xff, ' ', 1, "sub %a,yl" },
    { 0x07, 0xff, ' ', 1, "sub %a,yh" },
    { 0x09, 0xff, ' ', 3, "sbc %a,'a16_8'" },
    { 0x0a, 0xff, ' ', 2, "sbc %a,('nsp_8')" },
    { 0x0b, 0xff, ' ', 3, "sbc %a,('nnz_8')" },
    { 0x0c, 0xff, ' ', 1, "sbc %a,zl" },
    { 0x0d, 0xff, ' ', 1, "sbc %a,xh" },
    { 0x0e, 0xff, ' ', 1, "sbc %a,yl" },
    { 0x0f, 0xff, ' ', 1, "sbc %a,yh" },

    { 0x20, 0xff, ' ', 2, "cp %a,#'i8'" },
    { 0x21, 0xff, ' ', 3, "cp %a,'a16_8'" },
    { 0x22, 0xff, ' ', 2, "cp %a,('nsp_8')" },
    { 0x23, 0xff, ' ', 3, "cp %a,('nnz_8')" },
    { 0x24, 0xff, ' ', 1, "cp %a,zl" },
    { 0x25, 0xff, ' ', 1, "cp %a,xh" },
    { 0x26, 0xff, ' ', 1, "cp %a,yl" },
    { 0x27, 0xff, ' ', 1, "cp %a,yh" },

    { 0x28, 0xff, ' ', 2, "or %a,#'i8'" },
    { 0x29, 0xff, ' ', 3, "or %a,'a16_8'" },
    { 0x2a, 0xff, ' ', 2, "or %a,('nsp_8')" },
    { 0x2b, 0xff, ' ', 3, "or %a,('nnz_8')" },
    { 0x2c, 0xff, ' ', 1, "or %a,zl" },
    { 0x2d, 0xff, ' ', 1, "or %a,xh" },
    { 0x2e, 0xff, ' ', 1, "or %a,yl" },
    { 0x2f, 0xff, ' ', 1, "or %a,yh" },

    { 0x30, 0xff, ' ', 2, "and %a,#'i8'" },
    { 0x31, 0xff, ' ', 3, "and %a,'a16_8'" },
    { 0x32, 0xff, ' ', 2, "and %a,('nsp_8')" },
    { 0x33, 0xff, ' ', 3, "and %a,('nnz_8')" },
    { 0x34, 0xff, ' ', 1, "and %a,zl" },
    { 0x35, 0xff, ' ', 1, "and %a,xh" },
    { 0x36, 0xff, ' ', 1, "and %a,yl" },
    { 0x37, 0xff, ' ', 1, "and %a,yh" },

    { 0x38, 0xff, ' ', 2, "xor %a,#'i8'" },
    { 0x39, 0xff, ' ', 3, "xor %a,'a16_8'" },
    { 0x3a, 0xff, ' ', 2, "xor %a,('nsp_8')" },
    { 0x3b, 0xff, ' ', 3, "xor %a,('nnz_8')" },
    { 0x3c, 0xff, ' ', 1, "xor %a,zl" },
    { 0x3d, 0xff, ' ', 1, "xor %a,xh" },
    { 0x3e, 0xff, ' ', 1, "xor %a,yl" },
    { 0x3f, 0xff, ' ', 1, "xor %a,yh" },

    { 0x71, 0xff, ' ', 3, "subw %A,'a16_16'" },
    { 0x72, 0xff, ' ', 2, "subw %A,('nsp_16')" },
    { 0x73, 0xff, ' ', 1, "subw %A,%R" },
    { 0x75, 0xff, ' ', 3, "sbcw %A,'a16_16'" },
    { 0x76, 0xff, ' ', 2, "sbcw %A,('nsp_16')" },
    { 0x77, 0xff, ' ', 1, "sbcw %A,%R" },

    { 0x78, 0xff, ' ', 3, "addw %A,#'i16'" },
    { 0x79, 0xff, ' ', 3, "addw %A,'a16_16'" },
    { 0x7a, 0xff, ' ', 2, "addw %A,('nsp_16')" },
    { 0x7b, 0xff, ' ', 1, "addw %A,%R" },
    { 0x7c, 0xff, ' ', 3, "adcw %A,#'i16'" },
    { 0x7d, 0xff, ' ', 3, "adcw %A,'a16_16'" },
    { 0x7e, 0xff, ' ', 2, "adcw %A,('nsp_16')" },
    { 0x7f, 0xff, ' ', 1, "adcw %A,%R" },

    { 0xf0, 0xff, ' ', 3, "orw %A,#'i16'" },
    { 0xf1, 0xff, ' ', 3, "orw %A,'a16_16'" },
    { 0xf2, 0xff, ' ', 2, "orw %A,('nsp_16')" },
    { 0xf3, 0xff, ' ', 1, "orw %A,%R" },

    { 0xfc, 0xff, ' ', 3, "xorw %A,#'i16'" },
    { 0xfd, 0xff, ' ', 3, "xorw %A,'a16_16'" },
    { 0xfe, 0xff, ' ', 2, "xorw %A,('nsp_16')" },
    { 0xff, 0xff, ' ', 1, "xorw %A,%R" },

    { 0x40, 0xff, ' ', 3, "srl 'a16_8'" },
    { 0x41, 0xff, ' ', 2, "srl ('nsp_8')" },
    { 0x42, 0xff, ' ', 1, "srl %a" },
    { 0x43, 0xff, ' ', 1, "srl ('ny_8')" },
    { 0x44, 0xff, ' ', 3, "sll 'a16_8'" },
    { 0x45, 0xff, ' ', 2, "sll ('nsp_8')" },
    { 0x46, 0xff, ' ', 1, "sll %a" },
    { 0x47, 0xff, ' ', 1, "sll ('ny_8')" },
    { 0x48, 0xff, ' ', 3, "rrc 'a16_8'" },
    { 0x49, 0xff, ' ', 2, "rrc ('nsp_8')" },
    { 0x4a, 0xff, ' ', 1, "rrc %a" },
    { 0x4b, 0xff, ' ', 1, "rrc ('ny_8')" },
    { 0x4c, 0xff, ' ', 3, "rlc 'a16_8'" },
    { 0x4d, 0xff, ' ', 2, "rlc ('nsp_8')" },
    { 0x4e, 0xff, ' ', 1, "rlc %a" },
    { 0x4f, 0xff, ' ', 1, "rlc ('ny_8')" },

    { 0x50, 0xff, ' ', 3, "inc 'a16_8'" },
    { 0x51, 0xff, ' ', 2, "inc ('nsp_8')" },
    { 0x52, 0xff, ' ', 1, "inc %a" },
    { 0x53, 0xff, ' ', 1, "inc ('ny_8')" },
    { 0x54, 0xff, ' ', 3, "dec 'a16_8'" },
    { 0x55, 0xff, ' ', 2, "dec ('nsp_8')" },
    { 0x56, 0xff, ' ', 1, "dec %a" },
    { 0x57, 0xff, ' ', 1, "dec ('ny_8')" },
    { 0x5c, 0xff, ' ', 3, "tst 'a16_8'" },
    { 0x5d, 0xff, ' ', 2, "tst ('nsp_8')" },
    { 0x5e, 0xff, ' ', 1, "tst %a" },
    { 0x5f, 0xff, ' ', 1, "tst ('ny_8')" },

    { 0xa4, 0xff, ' ', 3, "incw 'a16_16'" },
    { 0xa5, 0xff, ' ', 2, "incw ('nsp_16')" },
    { 0xa6, 0xff, ' ', 3, "incw ('nnz_16')" },
    { 0xa7, 0xff, ' ', 1, "incw %A" },
    { 0xa8, 0xff, ' ', 3, "adcw 'a16_16'" },
    { 0xa9, 0xff, ' ', 2, "adcw ('nsp_16')" },
    { 0xaa, 0xff, ' ', 3, "adcw ('nnz_16')" },
    { 0xab, 0xff, ' ', 1, "adcw %A" },
    { 0xac, 0xff, ' ', 3, "sbcw 'a16_16'" },
    { 0xad, 0xff, ' ', 2, "sbcw ('nsp_16')" },
    { 0xae, 0xff, ' ', 3, "sbcw ('nnz_16')" },
    { 0xaf, 0xff, ' ', 1, "sbcw %A" },
    { 0xb4, 0xff, ' ', 3, "tstw 'a16_16'" },
    { 0xb5, 0xff, ' ', 2, "tstw ('nsp_16')" },
    { 0xb6, 0xff, ' ', 3, "tstw ('nnz_16')" },
    { 0xb7, 0xff, ' ', 1, "tstw %A" },

    { 0x95, 0xff, ' ', 2, "rot %a,#'i8'" },
    { 0x96, 0xff, ' ', 1, "sra %a" },
    { 0x97, 0xff, ' ', 1, "daa %a" },
    { 0x98, 0xff, ' ', 1, "bool %a" },
    { 0xb8, 0xff, ' ', 2, "msk (%A),%a,#'i8'" },
    { 0xbc, 0xff, ' ', 3, "mad x,'a16_8',yl" },
    { 0xbd, 0xff, ' ', 2, "mad x,('nsp_8'),yl" },
    { 0xbe, 0xff, ' ', 3, "mad x,('nnz_8'),yl" },
    { 0xbf, 0xff, ' ', 1, "mad x,('z_8'),yl" },
    
    { 0xb9, 0xff, ' ', 1, "mul %A"},
    { 0xfa, 0xff, ' ', 1, "neg %A"},
    { 0xfb, 0xff, ' ', 1, "boolw %A"},
    { 0xe0, 0xff, ' ', 1, "srlw %A"},
    { 0xe1, 0xff, ' ', 1, "sllw %A"},
    { 0xe2, 0xff, ' ', 1, "rrcw %A"},
    { 0xe3, 0xff, ' ', 1, "rlcw %A"},
    { 0xe6, 0xff, ' ', 2, "rrcw ('nsp_16')"},
    { 0xe7, 0xff, ' ', 2, "rlcw ('nsp_16')"},
    { 0xe4, 0xff, ' ', 1, "sraw %A"},
    { 0xea, 0xff, ' ', 2, "addw sp,#%d"},
    { 0xeb, 0xff, ' ', 2, "addw %A,#%d"},
    { 0xec, 0xff, ' ', 2, "xch f,('nsp_16')"},
    { 0xf8, 0xff, ' ', 3, "cpw %A,#'i16'"},
    { 0xf6, 0xff, ' ', 1, "incnw %A"},
    { 0xf7, 0xff, ' ', 2, "decw ('nsp_16')"},
    { 0xe5, 0xff, ' ', 1, "sllw %A,xl"},
    { 0xee, 0xff, ' ', 1, "sex %A,%a"},
    { 0xef, 0xff, ' ', 1, "zex %A,%a"},

    // branch
    { 0x64, 0xff, ' ', 3, "jp #'a16'" },
    { 0x65, 0xff, ' ', 1, "jp %A" },
    { 0x66, 0xff, ' ', 3, "call #'a16'" },
    { 0x67, 0xff, ' ', 1, "call %A" },
    { 0xba, 0xff, ' ', 1, "ret" },
    { 0xbb, 0xff, ' ', 1, "reti" },

    { 0xd0, 0xff, ' ', 2, "jr %r" },
    { 0xd1, 0xff, ' ', 2, "dnjnz yh,%r" },
    { 0xd2, 0xff, ' ', 2, "jrz %r" },
    { 0xd3, 0xff, ' ', 2, "jrnz %r" },
    { 0xd4, 0xff, ' ', 2, "jrc %r" },
    { 0xd5, 0xff, ' ', 2, "jrnc %r" },
    { 0xd6, 0xff, ' ', 2, "jrn %r" },
    { 0xd7, 0xff, ' ', 2, "jrnn %r" },
    { 0xd9, 0xff, ' ', 2, "jrno %r" },
    { 0xda, 0xff, ' ', 2, "jrsge %r" },
    { 0xdb, 0xff, ' ', 2, "jrslt %r" },
    { 0xdd, 0xff, ' ', 2, "jrsle %r" },
    { 0xdf, 0xff, ' ', 2, "jrle %r" },
    
    // other
    { 0x00, 0xff, ' ', 1, "trap" },
    { 0x08, 0xff, ' ', 1, "nop" },
    { 0x9a, 0xff, ' ', 1, "thrd %a" },

    { 0, 0, 0, 0, 0, 0 }
  };


struct cpu_entry cpus_f8[]=
  {
    {"F8"	, CPU_F8, 0		, "F8", ""},

    {NULL, CPU_NONE, 0, "", ""}
  };

u16_t tick_tab_f8[256]= {
  /*           _0    _1    _2    _3      _4    _5    _6    _7      _8    _9    _a    _b      _c    _d    _e    _f */
  /* 0_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 1_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 2_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 3_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 4_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 5_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 6_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 7_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 8_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* 9_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* a_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* b_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* c_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* d_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* e_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0, 
  /* f_ */    0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0
};

// Bitmasks of P_XXXX
enum {
  // shorts for allowed prefixes
  PN		= P_NONE,                                    // none
  PS		= P_SWAP,                                    // swap
  PA		= P_SWAP|P_ALT1|P_ALT2|P_ALT3|P_ALT4|P_ALT5, // any
  PW            = P_ALT2|P_ALT3,                             // altacc16 selection only
  P6		= P_ALT2|P_ALT3|P_ALT4,                      // 2
  PD		= P_ALT1|P_ALT2|P_ALT3|P_ALT4|P_ALT5,        // any except swap
  P2		= P_SWAP|P6                                  // 02
};

u8_t allowed_prefs[256]= {
  /*          _0 _1 _2 _3   _4 _5 _6 _7   _8 _9 _a _b   _c _d _e _f */
  /* 0_ */    PN,PA,PA,PA,  PA,PA,PA,PA,  PN,PA,PA,PA,  PA,PA,PA,PA,
  /* 1_ */    PD,PA,PA,PA,  PA,PA,PA,PA,  PD,PA,PA,PA,  PA,PA,PA,PA,
  /* 2_ */    PD,PA,PA,PA,  PA,PA,PA,PA,  PD,PA,PA,PA,  PA,PA,PA,PA,
  /* 3_ */    PD,PA,PA,PA,  PA,PA,PA,PA,  PD,PA,PA,PA,  PA,PA,PA,PA,
  /* 4_ */    PN,PN,PD,PN,  PN,PN,PD,PN,  PN,PN,PD,PN,  PN,PN,PD,PN,
  /* 5_ */    PN,PN,PD,PN,  PN,PN,PD,PN,  PN,PN,PD,PN,  PN,PN,PD,PN,
  /* 6_ */    PN,PN,PD,PN,  PN,PW,PN,PW,  PN,PN,PN,PN,  PN,PN,PN,PN,
  /* 7_ */    PA,PA,PA,PA,  PD,PA,PA,PA,  PD,PA,PA,PA,  PD,PA,PA,PA, // Not correct - this line is too permissive for the 16-bit two-operand instructions.
  /* 8_ */    PD,PD,PD,PD,  PD,PD,PA,PA,  PA,PA,PA,PD,  PD,PD,PD,PD,
  /* 9_ */    PN,PD,PD,P6,  PN,PD,PD,PD,  PD,PD,PD,PN,   0, 0, 0, 0,
  /* a_ */    PN,PN,PN,PW,  PN,PN,PN,PW,  PN,PN,PN,PW,  PN,PN,PN,PW,
  /* b_ */    PN,PN,PN,PW,  P6,P6,P6,P6,  P6,P6,PN,PN,  PN,PN,PN,PN,
  /* c_ */    P6,P6,P6,P6,  P6,P6,P6,P6,  P6,P6,P6,P6,  P6,PD,PW,PN, // Not correct - this line is too permissive for the ldw (y), x and ldw (n, y), x instructions.
  /* d_ */    PA,PN,PN,PN,  PN,PN,PW,PN,  PS,PN,PN,PW,  PN,PS,PD,PS, // Not correct - this line is too permissive for the ldw x, y instruction.
  /* e_ */    P6,P6,P6,P6,  P6,P6,PN,PN,  PN,P6,PN,P6,  PN,PN,PD,PD,
  /* f_ */    PD,PA,PA,PA,  PW,PW,P6,PN,  P6,PN,P6,P6,  PD,PA,PA,PA  // Not correct - this line is too permissive for the 16-bit two-operand instructions.
};


/* End of f8.src/glob.cc */
