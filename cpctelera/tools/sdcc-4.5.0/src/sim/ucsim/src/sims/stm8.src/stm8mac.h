/*
 * Simulator of microcontrollers (stm8mac.h)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
 *
 * Copyright (C) 2015 Drotos Daniel
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

// shift positions
#define BITPOS_C 0  // 1
#define BITPOS_Z 1  // 2H
#define BITPOS_N 2  // 4H
#define BITPOS_I0 3  // 8H
#define BITPOS_H 4  // 10H
#define BITPOS_I1 5  // 20H
#define BITPOS_V 7  // 80H

#define store2(addr, val) { ram->write((t_addr) (addr), (val >> 8) & 0xff); \
                            ram->write((t_addr) (addr+1), val & 0xff); \
  			    vc.wr+= 2; }
#define store1(addr, val) { ram->write((t_addr) (addr), val); vc.wr++; }
//#define get1(addr) ram->read((t_addr) (addr))
#define get1(addr) get_1(addr)
#define fetch2() ((fetch() << 8) | fetch() )
#define fetch1() fetch()
#define push2(val) {							\
    t_addr sp_before= regs.SP;						\
    store1(regs.SP,(val));						\
    cSP.W(regs.SP-1);							\
    store1(regs.SP,(val)>>8);						\
    cSP.W(regs.SP-1);							\
    class cl_stack_op *so=						\
      new cl_stack_push(instPC,val,sp_before,regs.SP);			\
    so->init();								\
    stack_write(so);							\
  }
#define push1(val) {							\
    t_addr sp_before= regs.SP;						\
    store1(regs.SP,(val));						\
    cSP.W(regs.SP-1);							\
    class cl_stack_op *so=						\
      new cl_stack_push(instPC,val,sp_before,regs.SP);			\
    so->init();								\
    stack_write(so);							\
  }
#define pop2(var) {u8_t h,l;cSP.W(regs.SP+1);h=get1(regs.SP);cSP.W(regs.SP+1);l=get1(regs.SP);var=h*256+l;}
#define pop1(var) {cSP.W(regs.SP+1); var=get1(regs.SP);}


#define FLAG_SET(f) {regs.CC |= f;}
#define FLAG_CLEAR(f) {regs.CC &= ~(f);}
#define FLAG_ASSIGN(f,c) {regs.CC = (c) ? regs.CC | (f) : regs.CC & ~(f);}
#define FLAG_NZ(f) { \
      regs.CC = (regs.CC & ~(BIT_N|BIT_Z)) \
      | (((f) & 0xff) ? 0 : BIT_Z) \
      | (((f) & 0x80) ? BIT_N : 0) \
      ; }
#define EA_IMM(c) ((((c) >> 4) & 0xf)==0xa)
#define OPERAND(code,prefix) (EA_IMM(code) ? fetch() : get1(fetchea(code,prefix)))
#define OPERANDW(code,prefix) (EA_IMM(code) ? fetch2() : get2(fetchea(code,prefix)))


/* End of sm8.src/stm8mac.h */
