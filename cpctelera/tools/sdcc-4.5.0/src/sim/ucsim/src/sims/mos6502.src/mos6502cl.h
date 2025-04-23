/*
 * Simulator of microcontrollers (mos6502cl.h)
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

#ifndef MOS6502CL_HEADER
#define MOS6502CL_HEADER

#include "uccl.h"
#include "memcl.h"
#include "decode.h"
#include "itsrccl.h"


#define rA  (A)
#define rX  (X)
#define rY  (Y)
#define rSP (SP)
#define rS  (SP)
#define rCC (CC)
#define rP  (CC)
#define rF  (CC)
#define cP  (cCC)
#define cF  (cCC)
#define cS  (cSP)

enum {
  mN	= 0x80,
  flagN	= 0x80,
  mS	= 0x80,
  flagS	= 0x80,
  mV	= 0x40,
  flagV	= 0x40,
  mO	= 0x40,
  flagO	= 0x40,
  mB	= 0x10,
  flagB	= 0x10,
  mD	= 0x08,
  flagD	= 0x08,
  mI	= 0x04,
  flagI	= 0x04,
  mZ	= 0x02,
  flagZ	= 0x02,
  mC	= 0x01,
  flagC	= 0x01
};

// Vectors
enum {
  NMI_AT	= 0xfffa,
  RESET_AT	= 0xfffc,
  IRQ_AT	= 0xfffe
};


class cl_c65: public cl_cell8
{
#ifdef DEVEL
  virtual t_mem read(void);
  virtual t_mem get(void);
  virtual t_mem write(t_mem val);
  virtual t_mem set(t_mem val);
#endif
};

class cl_as65: public cl_address_space
{
public:
  cl_as65(const char *id, t_addr astart, t_addr asize, int awidth):
    cl_address_space(id, astart, asize, awidth) {}
  virtual class cl_memory_cell *cell_template();
};


/*
 * Base of MOS6502 processor
 */

class cl_mos6502: public cl_uc  
{
public:
  u8_t A, X, Y, SP, CC, i8d;
  class cl_cell8 cA, cX, cY, cSP, cCC, ci8;
  class cl_it_src *src_irq, *src_nmi, *src_brk;
  bool set_b;
  chars *my_id;
public:
  cl_mos6502(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_cpu_hw(void);
  virtual void make_memories(void);

  virtual double def_xtal(void) { return 1000000; }
  virtual int clock_per_cycle(void) { return 1; }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual void analyze_start(void);
  virtual void analyze(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment=NULL);
  virtual t_addr read_addr(class cl_memory *m, t_addr start_addr);

  virtual void print_regs(class cl_console_base *con);
  virtual int inst_length(t_addr addr);
  virtual bool is_call(t_addr addr);

  virtual int exec_inst(void);
  virtual int priority_of(uchar nuof_it) { return nuof_it; }
  virtual int accept_it(class it_level *il);
  virtual bool it_enabled(void);
  virtual void push_addr(t_addr a);
  virtual t_addr pop_addr(void);
  virtual void stack_check_overflow(class cl_stack_op *op);
  
  virtual class cl_cell8 &imm8(void);
  // read operands
  virtual class cl_cell8 &zpg(void);
  virtual class cl_cell8 &zpgX(void);
  virtual class cl_cell8 &zpgY(void);
  virtual class cl_cell8 &abs(void);
  virtual class cl_cell8 &absX(void);
  virtual class cl_cell8 &absY(void);
  virtual class cl_cell8 &ind(void);
  virtual class cl_cell8 &zind(void);
  virtual class cl_cell8 &indX(void);
  virtual class cl_cell8 &indY(void);
  // write operands
  virtual class cl_cell8 &dstzpg(void) { vc.wr++; return zpg(); }
  virtual class cl_cell8 &dstzpgX(void) { vc.wr++; return zpgX(); }
  virtual class cl_cell8 &dstzpgY(void) { vc.wr++; return zpgY(); }
  virtual class cl_cell8 &dstabs(void) { vc.wr++; return abs(); }
  virtual class cl_cell8 &dstabsX(void) { vc.wr++; return absX(); }
  virtual class cl_cell8 &dstabsY(void) { vc.wr++; return absY(); }
  virtual class cl_cell8 &dstind(void) { vc.wr++; return ind(); }
  virtual class cl_cell8 &dstzind(void) { vc.wr++; return zind(); }
  virtual class cl_cell8 &dstindX(void) { vc.wr++; return indX(); }
  virtual class cl_cell8 &dstindY(void) { vc.wr++; return indY(); }
  // read-modify-write operands
  virtual class cl_cell8 &rmwzpg(void) { vc.rd++;vc.wr++;tick(2); return zpg(); }
  virtual class cl_cell8 &rmwzpgX(void) { vc.rd++;vc.wr++;tick(2); return zpgX(); }
  virtual class cl_cell8 &rmwzpgY(void) { vc.rd++;vc.wr++;tick(2); return zpgY(); }
  virtual class cl_cell8 &rmwabs(void) { vc.rd++;vc.wr++;tick(2); return abs(); }
  virtual class cl_cell8 &rmwabsX(void) { vc.rd++;vc.wr++;tick(3); return absX(); }
  virtual class cl_cell8 &rmwabsY(void) { vc.rd++;vc.wr++;tick(3); return absY(); }
  virtual class cl_cell8 &rmwind(void) { vc.rd++;vc.wr++;tick(1); return ind(); }
  virtual class cl_cell8 &rmwzind(void) { vc.rd++;vc.wr++;tick(1); return zind(); }
  virtual class cl_cell8 &rmwindX(void) { vc.rd++;vc.wr++;tick(1); return indX(); }
  virtual class cl_cell8 &rmwindY(void) { vc.rd++;vc.wr++;tick(1); return indY(); }
  //virtual u8_t i8(void) { return fetch(); }
  virtual u16_t i16(void) { u8_t h, l; l=fetch(); h= fetch(); return h*256+l; }

  virtual int NOP(t_mem code);
  virtual int BRK(t_mem code);
  virtual int RTI(t_mem code);
  virtual int CLI(t_mem code);
  virtual int SEI(t_mem code);
  virtual int PHP(t_mem code);
  virtual int CLC(t_mem code);
  virtual int PLP(t_mem code);
  virtual int SEc(t_mem code);
  virtual int PHA(t_mem code);
  virtual int PLA(t_mem code);
  virtual int DEY(t_mem code);
  virtual int TYA(t_mem code);
  virtual int TAY(t_mem code);
  virtual int CLV(t_mem code);
  virtual int INY(t_mem code);
  virtual int CLD(t_mem code);
  virtual int INX(t_mem code);
  virtual int SED(t_mem code);
  virtual int TXA(t_mem code);
  virtual int TXS(t_mem code);
  virtual int TAX(t_mem code);
  virtual int TSX(t_mem code);
  virtual int DEX(t_mem code);

  virtual int ora(class cl_cell8 &op);
  virtual int ORAix(t_mem code) { return ora(indX()); }
  virtual int ORAiy(t_mem code) { return ora(indY()); }
  virtual int ORAz (t_mem code) { return ora(zpg()); }
  virtual int ORAzx(t_mem code) { return ora(zpgX()); }
  virtual int ORA8 (t_mem code) { return ora(imm8()); }
  virtual int ORAay(t_mem code) { return ora(absY()); }
  virtual int ORAa (t_mem code) { return ora(abs()); }
  virtual int ORAax(t_mem code) { return ora(absX()); }

  virtual int And(class cl_cell8 &op);
  virtual int ANDix(t_mem code) { return And(indX()); }
  virtual int ANDiy(t_mem code) { return And(indY()); }
  virtual int ANDz (t_mem code) { return And(zpg()); }
  virtual int ANDzx(t_mem code) { return And(zpgX()); }
  virtual int AND8 (t_mem code) { return And(imm8()); }
  virtual int ANDay(t_mem code) { return And(absY()); }
  virtual int ANDa (t_mem code) { return And(abs()); }
  virtual int ANDax(t_mem code) { return And(absX()); }

  virtual int eor(class cl_cell8 &op);
  virtual int EORix(t_mem code) { return eor(indX()); }
  virtual int EORiy(t_mem code) { return eor(indY()); }
  virtual int EORz (t_mem code) { return eor(zpg()); }
  virtual int EORzx(t_mem code) { return eor(zpgX()); }
  virtual int EOR8 (t_mem code) { return eor(imm8()); }
  virtual int EORay(t_mem code) { return eor(absY()); }
  virtual int EORa (t_mem code) { return eor(abs()); }
  virtual int EORax(t_mem code) { return eor(absX()); }

  virtual int adc(class cl_cell8 &op);
  virtual int ADCix(t_mem code) { return adc(indX()); }
  virtual int ADCiy(t_mem code) { return adc(indY()); }
  virtual int ADCz (t_mem code) { return adc(zpg()); }
  virtual int ADCzx(t_mem code) { return adc(zpgX()); }
  virtual int ADC8 (t_mem code) { return adc(imm8()); }
  virtual int ADCay(t_mem code) { return adc(absY()); }
  virtual int ADCa (t_mem code) { return adc(abs()); }
  virtual int ADCax(t_mem code) { return adc(absX()); }

  virtual int sta(class cl_cell8 &op);
  virtual int STAix(t_mem code) { return sta(dstindX()); }
  virtual int STAiy(t_mem code) { return sta(dstindY()); }
  virtual int STAz (t_mem code) { return sta(dstzpg()); }
  virtual int STAzx(t_mem code) { return sta(dstzpgX()); }
  virtual int STAay(t_mem code) { return sta(dstabsY()); }
  virtual int STAa (t_mem code) { return sta(dstabs()); }
  virtual int STAax(t_mem code) { tick(1); return sta(dstabsX()); }

  virtual int lda(class cl_cell8 &op);
  virtual int LDAix(t_mem code) { return lda(indX()); }
  virtual int LDAiy(t_mem code) { return lda(indY()); }
  virtual int LDAz (t_mem code) { return lda(zpg()); }
  virtual int LDAzx(t_mem code) { return lda(zpgX()); }
  virtual int LDA8 (t_mem code) { return lda(imm8()); }
  virtual int LDAay(t_mem code) { return lda(absY()); }
  virtual int LDAa (t_mem code) { return lda(abs()); }
  virtual int LDAax(t_mem code) { return lda(absX()); }

  virtual int sbc(class cl_cell8 &op);
  virtual int SBCix(t_mem code) { return sbc(indX()); }
  virtual int SBCiy(t_mem code) { return sbc(indY()); }
  virtual int SBCz (t_mem code) { return sbc(zpg()); }
  virtual int SBCzx(t_mem code) { return sbc(zpgX()); }
  virtual int SBC8 (t_mem code) { return sbc(imm8()); }
  virtual int SBCay(t_mem code) { return sbc(absY()); }
  virtual int SBCa (t_mem code) { return sbc(abs()); }
  virtual int SBCax(t_mem code) { return sbc(absX()); }

  virtual int cmp(class cl_cell8 &op1, class cl_cell8 &op2);
  virtual int CMPix(t_mem code) { return cmp(cA, indX()); }
  virtual int CMPiy(t_mem code) { return cmp(cA, indY()); }
  virtual int CMPz (t_mem code) { return cmp(cA, zpg()); }
  virtual int CMPzx(t_mem code) { return cmp(cA, zpgX()); }
  virtual int CMP8 (t_mem code) { return cmp(cA, imm8()); }
  virtual int CMPay(t_mem code) { return cmp(cA, absY()); }
  virtual int CMPa (t_mem code) { return cmp(cA, abs()); }
  virtual int CMPax(t_mem code) { return cmp(cA, absX()); }

  virtual int CPY8(t_mem code) { return cmp(cY, imm8()); }
  virtual int CPYz(t_mem code) { return cmp(cY, zpg()); }
  virtual int CPYa(t_mem code) { return cmp(cY, abs()); }
  virtual int CPX8(t_mem code) { return cmp(cX, imm8()); }
  virtual int CPXz(t_mem code) { return cmp(cX, zpg()); }
  virtual int CPXa(t_mem code) { return cmp(cX, abs()); }

  virtual int st(u8_t reg, class cl_cell8 &op);
  virtual int STYz (t_mem code) { return st(rY, dstzpg()); }
  virtual int STYzx(t_mem code) { return st(rY, dstzpgX()); }
  virtual int STYa (t_mem code) { return st(rY, dstabs()); }
  virtual int STXz (t_mem code) { return st(rX, dstzpg()); }
  virtual int STXzy(t_mem code) { return st(rX, dstzpgY()); }
  virtual int STXa (t_mem code) { return st(rX, dstabs()); }

  virtual int ld(class cl_cell8 &reg, class cl_cell8 &op);
  virtual int LDY8 (t_mem code) { return ld(cY, imm8()); }
  virtual int LDYz (t_mem code) { return ld(cY, zpg()); }
  virtual int LDYzx(t_mem code) { return ld(cY, zpgX()); }
  virtual int LDYa (t_mem code) { return ld(cY, abs()); }
  virtual int LDYax(t_mem code) { return ld(cY, absX()); }
  virtual int LDX8 (t_mem code) { return ld(cX, imm8()); }
  virtual int LDXz (t_mem code) { return ld(cX, zpg()); }
  virtual int LDXzy(t_mem code) { return ld(cX, zpgY()); }
  virtual int LDXa (t_mem code) { return ld(cX, abs()); }
  virtual int LDXay(t_mem code) { return ld(cX, absY()); }

  virtual int inc(class cl_cell8 &op);
  virtual int dec(class cl_cell8 &op);
  virtual int INCz (t_mem code) { return inc(rmwzpg()); }
  virtual int INCzx(t_mem code) { return inc(rmwzpgX()); }
  virtual int INCa (t_mem code) { return inc(rmwabs()); }
  virtual int INCax(t_mem code) { return inc(rmwabsX()); }
  virtual int DECz (t_mem code) { return dec(rmwzpg()); }
  virtual int DECzx(t_mem code) { return dec(rmwzpgX()); }
  virtual int DECa (t_mem code) { return dec(rmwabs()); }
  virtual int DECax(t_mem code) { return dec(rmwabsX()); }
  
  virtual int asl(class cl_cell8 &op);
  virtual int ASLz (t_mem code) { return asl(rmwzpg()); }
  virtual int ASLzx(t_mem code) { return asl(rmwzpgX()); }
  virtual int ASL  (t_mem code) { tick(1); return asl(cA); }
  virtual int ASLa (t_mem code) { return asl(rmwabs()); }
  virtual int ASLax(t_mem code) { return asl(rmwabsX()); }

  virtual int lsr(class cl_cell8 &op);
  virtual int LSRz (t_mem code) { return lsr(rmwzpg()); }
  virtual int LSRzx(t_mem code) { return lsr(rmwzpgX()); }
  virtual int LSR  (t_mem code) { tick(1); return lsr(cA); }
  virtual int LSRa (t_mem code) { return lsr(rmwabs()); }
  virtual int LSRax(t_mem code) { return lsr(rmwabsX()); }

  virtual int rol(class cl_cell8 &op);
  virtual int ROLz (t_mem code) { return rol(rmwzpg()); }
  virtual int ROLzx(t_mem code) { return rol(rmwzpgX()); }
  virtual int ROL  (t_mem code) { tick(1); return rol(cA); }
  virtual int ROLa (t_mem code) { return rol(rmwabs()); }
  virtual int ROLax(t_mem code) { return rol(rmwabsX()); }

  virtual int ror(class cl_cell8 &op);
  virtual int RORz (t_mem code) { return ror(rmwzpg()); }
  virtual int RORzx(t_mem code) { return ror(rmwzpgX()); }
  virtual int ROR  (t_mem code) { tick(1); return ror(cA); }
  virtual int RORa (t_mem code) { return ror(rmwabs()); }
  virtual int RORax(t_mem code) { return ror(rmwabsX()); }

  virtual int bit(class cl_cell8 &op);
  virtual int BITz(t_mem code) { return bit(zpg()); }
  virtual int BITa(t_mem code) { return bit(abs()); }

  virtual int JMPa(t_mem code);
  virtual int JMPi(t_mem code);

  virtual int JSR(t_mem code);
  virtual int RTS(t_mem code);
  
  virtual int branch(bool cond);
  virtual int BPL(t_mem code) { return branch(!(rF & flagN)); }
  virtual int BMI(t_mem code) { return branch( (rF & flagN)); }
  virtual int BVC(t_mem code) { return branch(!(rF & flagV)); }
  virtual int BVS(t_mem code) { return branch( (rF & flagV)); }
  virtual int BCC(t_mem code) { return branch(!(rF & flagC)); }
  virtual int BCS(t_mem code) { return branch( (rF & flagC)); }
  virtual int BNE(t_mem code) { return branch(!(rF & flagZ)); }
  virtual int BEQ(t_mem code) { return branch( (rF & flagZ)); }
};


/* Unused bits of CC forced to be 1 */

class cl_cc_operator: public cl_memory_operator
{
public:
  cl_cc_operator(class cl_memory_cell *acell): cl_memory_operator(acell) {}
  virtual t_mem write(t_mem val) { return val/*|= 0x20*/; }
};


#endif

/* End of mos6502.src/mos6502.cc */
