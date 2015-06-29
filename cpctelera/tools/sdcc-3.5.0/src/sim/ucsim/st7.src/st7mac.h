/*
 * Simulator of microcontrollers (st7mac.h)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

// shift positions
#define BITPOS_C 0  // 1
#define BITPOS_Z 1  // 2H
#define BITPOS_N 2  // 4H
#define BITPOS_I 3  // 8H
#define BITPOS_H 4  // 10H

#define store2(addr, val) { ram->set((t_addr) (addr), (val >> 8) & 0xff); \
                            ram->set((t_addr) (addr+1), val & 0xff); }
#define store1(addr, val) ram->set((t_addr) (addr), val)
#define get1(addr) ram->get((t_addr) (addr))
#define get2(addr) ((ram->get((t_addr) (addr)) << 8) | ram->get((t_addr) (addr+1)) )
#define get3(addr) ((ram->get((t_addr) (addr)) << 16) | (ram->get((t_addr) (addr+1)) << 8) |ram->get((t_addr) (addr+2)) )
#define fetch2() ((fetch() << 8) | fetch() )
#define fetch1() fetch()
#define push2(val) {store2(regs.SP-1,(val)); regs.SP-=2; }
#define push1(val) {store1(regs.SP,(val)); regs.SP-=1; }
#define pop2(var) {var=get2(regs.SP+1); regs.SP+=2;}
#define pop1(var) {var=get1(regs.SP+1); regs.SP+=1;}


#define FLAG_SET(f) {regs.CC |= f;}
#define FLAG_CLEAR(f) {regs.CC &= ~(f);}
#define FLAG_ASSIGN(f,c) {regs.CC = (c) ? regs.CC | (f) : regs.CC & ~(f);}
#define EA_IMM(c) ((((c) >> 4) & 0xf)==0xa)
#define OPERAND(code,prefix) (EA_IMM(code) ? fetch() : get1(fetchea(code,prefix)))



