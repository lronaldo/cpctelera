/*
 * Simulator of microcontrollers (m6800cl.h)
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

#ifndef M6800CL_HEADER
#define M6800CL_HEADER

#include "ddconfig.h"

#include "uccl.h"
#include "memcl.h"
#include "itsrccl.h"
#include "decode68.h"
#include "iwrap.h"


extern instruction_wrapper_fn itab[256];

struct acc_t {
  union {
    u16_t Dr;
    struct {
#ifdef WORDS_BIGENDIAN
      u8_t Ar;
      u8_t Br;
#else
      u8_t Br;
      u8_t Ar;
#endif
    } a8;
  } DAB;
};

struct cc_tip {
  union {
    u16_t cc16;
    struct {
#ifdef WORDS_BIGENDIAN
      u8_t cch;
      u8_t ccl;
#else
      u8_t ccl;
      u8_t cch;
#endif
    } cc8;
  } cc16;
};

#define rA  (acc.DAB.a8.Ar)
#define A   (acc.DAB.a8.Ar)
#define rB  (acc.DAB.a8.Br)
#define B   (acc.DAB.a8.Br)
#define rD  (acc.DAB.Dr)
#define D   (acc.DAB.Dr)
#define rCC (CC.cc16.cc8.ccl)
#define rF  (CC.cc16.cc8.ccl)
#define rIX (IX)
#define rX  (IX)
#define rSP (SP)

#define cF  (cCC)
#define cX  (cIX)


//  Flag bit masks
enum {
  mC	= 0x01,
  flagC	= 0x01,
  mO	= 0x02,
  flagO	= 0x02,
  mV	= 0x02,
  flagV	= 0x02,
  mZ	= 0x04,
  flagZ	= 0x04,
  mN	= 0x08,
  flagN	= 0x08,
  mI	= 0x10,
  flagI	= 0x10,
  mH	= 0x20,
  flagH	= 0x20,
  mA	= 0x40,
  flagA	= 0x40
};

#define ifCS	(rF&mC)
#define ifCC	(!(rF&mC))
#define ifMI	(rF&mN)
#define ifPL	(!(rF&mN))
#define ifVS	(rF&mV)
#define ifVC	(!(rF&mV))
#define ifEQ	(rF&mZ)
#define ifNE	(!(rF&mZ))
#define ifLT	( (rF&mN) ^ ((rF&mV)?mN:0) )
#define ifLE	( (rF&mZ) | (((rF&mN)?mZ:0) ^ ((rF&mV)?mZ:0)) )
#define ifGE	(!ifLT)
#define ifGT	(!ifLE)
#define ifLS	( ((rF&mC)?mZ:0) | (rF&mZ) )
#define ifHI	(!ifLS)
#define ifA	(true)
#define ifN	(false)

extern i8_t p0ticks[256];

class cl_idx16: public cl_cell16
{
public:
  chars name;
public:
  cl_idx16(): cl_cell16() { name= ""; }
};


class cl_mop16: public cl_cell16
{
protected:
  cl_memory_cell *l, *h;
  u16_t addr;
  class cl_address_space *as;
  class cl_uc *uc;
public:
  cl_mop16(): cl_cell16() {}
  virtual void set_uc(class cl_uc *iuc);
  virtual void a(u16_t iaddr);
  virtual void r(u16_t iaddr);
  virtual t_mem read(void) { return h->R()*256 + l->R(); }
  virtual t_mem get(void)  { return h->get()*256 + l->get(); }
  virtual t_mem write(t_mem val) { return (h->W(val>>8))*256 + l->W(val); }
  virtual t_mem set(t_mem val) { h->set(val>>8); l->set(val); return val; }
};

  
/*
 * Base of M6800 processor
 */

class cl_m6800: public cl_uc
{
public:
  struct acc_t acc;
  struct cc_tip CC;
  u16_t IX, SP;
  class cl_cell8 cA, cB, cCC;
  class cl_cell16 cSP;
  class cl_idx16 cIX;
  class cl_it_src *src_irq, *src_nmi, *src_swi;
  class cl_idx16 *cI;
  class cl_mop16 mop16;
  bool wai;
  u16_t IRQ_AT, SWI_AT, NMI_AT, RESET_AT;
public:
  cl_m6800(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_cpu_hw(void);
  virtual void make_memories(void);
  virtual void setup_ccr(void);
  
  virtual i8_t *tick_tab(t_mem code) { return p0ticks; }
  virtual double def_xtal(void) { return 1000000; }
  virtual int clock_per_cycle(void) { return 1; }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment=NULL);
  virtual t_addr read_addr(class cl_memory *m, t_addr start_addr);
  virtual void analyze_start(void);
  virtual void analyze(t_addr addr);
  virtual int inst_length(t_addr addr);
  virtual int longest_inst(void) { return 4; }
  
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);
  virtual int priority_of(uchar nuof_it) { return nuof_it; }
  virtual int accept_it(class it_level *il);
  virtual bool it_enabled(void) { return true; }
  virtual void push_regs(bool inst_part);
  virtual void pull_regs(bool inst_part);
  
  virtual class cl_memory_cell &idx(void);
  virtual class cl_memory_cell &ext(void);
  virtual class cl_memory_cell &dir(void);
  u8_t i8(void) { return fetch(); }
  u16_t i16(void) { u8_t h, l; h= fetch(); l= fetch(); return h*256+l; }
  u8_t iop(void) { vc.rd++; return idx().R(); }
  u8_t eop(void) { vc.rd++; return ext().R(); }
  u8_t dop(void) { vc.rd++; return dir().R(); }
  u16_t iop16(void);
  u16_t eop16(void);
  u16_t dop16(void);
  t_addr iaddr(void);
  t_addr eaddr(void);
  t_addr daddr(void);
  t_addr raddr(void);
  virtual class cl_memory_cell &idst(void) { vc.rd++; vc.wr++; return idx(); }
  virtual class cl_memory_cell &edst(void) { vc.rd++; vc.wr++; return ext(); }
  virtual class cl_memory_cell &ddst(void) { vc.rd++; vc.wr++; return dir(); }
  
  virtual int sub(class cl_memory_cell &dest, u8_t op, bool c);
  virtual int sub(class cl_memory_cell &dest, u8_t op);
  virtual int sbc(class cl_memory_cell &dest, u8_t op);
  virtual int cmp(u8_t op1, u8_t op2);
  virtual int add(class cl_memory_cell &dest, u8_t op, bool c);
  virtual int add(class cl_memory_cell &dest, u8_t op);
  virtual int adc(class cl_memory_cell &dest, u8_t op);
  virtual int neg(class cl_memory_cell &dest);
  virtual int com(class cl_memory_cell &dest);
  virtual int lsr(class cl_memory_cell &dest);
  virtual int ror(class cl_memory_cell &dest);
  virtual int asr(class cl_memory_cell &dest);
  virtual int asl(class cl_memory_cell &dest);
  virtual int rol(class cl_memory_cell &dest);
  virtual int dec(class cl_memory_cell &dest);
  virtual int inc(class cl_memory_cell &dest);
  virtual int tst(u8_t op);
  virtual int clr(class cl_memory_cell &dest);
  virtual int And(class cl_memory_cell &dest, u8_t op);
  virtual int bit(u8_t op1, u8_t op2);
  virtual int eor(class cl_memory_cell &dest, u8_t op);
  virtual int Or (class cl_memory_cell &dest, u8_t op);
  virtual int lda(class cl_memory_cell &dest, u8_t op);
  virtual int sta(class cl_memory_cell &dest, u8_t op);
  virtual int cpx(u16_t op);
  virtual int ldsx(class cl_cell16 &dest, u16_t op);
  virtual int stsx(t_addr a, u16_t op);
  virtual int call(t_addr a);
  virtual int branch(t_addr a, bool cond);
  
  virtual int NOP(t_mem code);
  virtual int TAP(t_mem code);
  virtual int TPA(t_mem code);
  virtual int INX(t_mem code);
  virtual int DEX(t_mem code);
  virtual int CLV(t_mem code);
  virtual int SEV(t_mem code);
  virtual int CLC(t_mem code);
  virtual int SEc(t_mem code);
  virtual int CLI(t_mem code);
  virtual int SEI(t_mem code);

  virtual int SBA(t_mem code) { return sub(cA, rB, false); }
  virtual int CBA(t_mem code) { return cmp(rA, rB); }
  virtual int TAB(t_mem code);
  virtual int TBA(t_mem code);
  virtual int DAA(t_mem code);
  virtual int ABA(t_mem code) { return add(cA, rB, false); }

  virtual int BRA(t_mem code) { return branch(raddr(), ifA); }
  virtual int BRN(t_mem code) { return branch(raddr(), ifN); }
  virtual int BHI(t_mem code) { return branch(raddr(), ifHI); }
  virtual int BLS(t_mem code) { return branch(raddr(), ifLS); }
  virtual int BCC(t_mem code) { return branch(raddr(), ifCC); }
  virtual int BCS(t_mem code) { return branch(raddr(), ifCS); }
  virtual int BNE(t_mem code) { return branch(raddr(), ifNE); }
  virtual int BEQ(t_mem code) { return branch(raddr(), ifEQ); }
  virtual int BVC(t_mem code) { return branch(raddr(), ifVC); }
  virtual int BVS(t_mem code) { return branch(raddr(), ifVS); }
  virtual int BPL(t_mem code) { return branch(raddr(), ifPL); }
  virtual int BMI(t_mem code) { return branch(raddr(), ifMI); }
  virtual int BGE(t_mem code) { return branch(raddr(), ifGE); }
  virtual int BLT(t_mem code) { return branch(raddr(), ifLT); }
  virtual int BGT(t_mem code) { return branch(raddr(), ifGT); }
  virtual int BLE(t_mem code) { return branch(raddr(), ifLE); }
  
  virtual int TSX(t_mem code);
  virtual int INS(t_mem code);
  virtual int PULA(t_mem code);
  virtual int PULB(t_mem code);
  virtual int DES(t_mem code);
  virtual int TXS(t_mem code);
  virtual int PSHA(t_mem code);
  virtual int PSHB(t_mem code);
  virtual int RTS(t_mem code);

  virtual int RTI(t_mem code);
  virtual int WAI(t_mem code);
  virtual int SWI(t_mem code);
  
  virtual int NEGA(t_mem code) { return neg(cA); }
  virtual int COMA(t_mem code) { return com(cA); }
  virtual int LSRA(t_mem code) { return lsr(cA); }
  virtual int RORA(t_mem code) { return ror(cA); }
  virtual int ASRA(t_mem code) { return asr(cA); }
  virtual int ASLA(t_mem code) { return asl(cA); }
  virtual int ROLA(t_mem code) { return rol(cA); }
  virtual int DECA(t_mem code) { return dec(cA); }
  virtual int INCA(t_mem code) { return inc(cA); }
  virtual int TSTA(t_mem code) { return tst(rA); }
  virtual int CLRA(t_mem code) { return clr(cA); }

  virtual int NEGB(t_mem code) { return neg(cB); }
  virtual int COMB(t_mem code) { return com(cB); }
  virtual int LSRB(t_mem code) { return lsr(cB); }
  virtual int RORB(t_mem code) { return ror(cB); }
  virtual int ASRB(t_mem code) { return asr(cB); }
  virtual int ASLB(t_mem code) { return asl(cB); }
  virtual int ROLB(t_mem code) { return rol(cB); }
  virtual int DECB(t_mem code) { return dec(cB); }
  virtual int INCB(t_mem code) { return inc(cB); }
  virtual int TSTB(t_mem code) { return tst(rB); }
  virtual int CLRB(t_mem code) { return clr(cB); }

  virtual int NEGi(t_mem code) { return neg(idst()); }
  virtual int COMi(t_mem code) { return com(idst()); }
  virtual int LSRi(t_mem code) { return lsr(idst()); }
  virtual int RORi(t_mem code) { return ror(idst()); }
  virtual int ASRi(t_mem code) { return asr(idst()); }
  virtual int ASLi(t_mem code) { return asl(idst()); }
  virtual int ROLi(t_mem code) { return rol(idst()); }
  virtual int DECi(t_mem code) { return dec(idst()); }
  virtual int INCi(t_mem code) { return inc(idst()); }
  virtual int TSTi(t_mem code) { vc.rd++; return tst(idx().R()); }
  virtual int JMPi(t_mem code);
  virtual int CLRi(t_mem code) { vc.wr++; return clr(idx()); }

  virtual int NEGe(t_mem code) { return neg(edst()); }
  virtual int COMe(t_mem code) { return com(edst()); }
  virtual int LSRe(t_mem code) { return lsr(edst()); }
  virtual int RORe(t_mem code) { return ror(edst()); }
  virtual int ASRe(t_mem code) { return asr(edst()); }
  virtual int ASLe(t_mem code) { return asl(edst()); }
  virtual int ROLe(t_mem code) { return rol(edst()); }
  virtual int DECe(t_mem code) { return dec(edst()); }
  virtual int INCe(t_mem code) { return inc(edst()); }
  virtual int TSTe(t_mem code) { vc.rd++; return tst(ext().R()); }
  virtual int JMPe(t_mem code);
  virtual int CLRe(t_mem code) { vc.wr++; return clr(ext()); }

  virtual int SUBA8(t_mem code) { return sub(cA, i8(), false); }
  virtual int CMPA8(t_mem code) { return cmp(rA, i8()); }
  virtual int SBCA8(t_mem code) { return sub(cA, i8(), true); }
  virtual int ANDA8(t_mem code) { return And(cA, i8()); }
  virtual int BITA8(t_mem code) { return bit(rA, i8()); }
  virtual int LDAA8(t_mem code) { return lda(cA, i8()); }
  virtual int EORA8(t_mem code) { return eor(cA, i8()); }
  virtual int ADCA8(t_mem code) { return add(cA, i8(), true); }
  virtual int ORAA8(t_mem code) { return Or (cA, i8()); }
  virtual int ADDA8(t_mem code) { return add(cA, i8(), false); }
  virtual int CPX16(t_mem code) { return cpx(i16()); }
  virtual int BSR  (t_mem code) { return call(raddr()); }
  virtual int LDS16(t_mem code) { return ldsx(cSP, i16()); }

  virtual int SUBAd(t_mem code) { return sub(cA, dop(), false); }
  virtual int CMPAd(t_mem code) { return cmp(rA, dop()); }
  virtual int SBCAd(t_mem code) { return sub(cA, dop(), true); }
  virtual int ANDAd(t_mem code) { return And(cA, dop()); }
  virtual int BITAd(t_mem code) { return bit(rA, dop()); }
  virtual int LDAAd(t_mem code) { return lda(cA, dop()); }
  virtual int STAAd(t_mem code) { return lda(ddst(), rA); }
  virtual int EORAd(t_mem code) { return eor(cA, dop()); }
  virtual int ADCAd(t_mem code) { return add(cA, dop(), true); }
  virtual int ORAAd(t_mem code) { return Or (cA, dop()); }
  virtual int ADDAd(t_mem code) { return add(cA, dop(), false); }
  virtual int CPXd (t_mem code) { return cpx(dop16()); }
  virtual int LDSd (t_mem code) { return ldsx(cSP, dop16()); }
  virtual int STSd (t_mem code) { return stsx(daddr(), rSP); }

  virtual int SUBAi(t_mem code) { return sub(cA, iop(), false); }
  virtual int CMPAi(t_mem code) { return cmp(rA, iop()); }
  virtual int SBCAi(t_mem code) { return sub(cA, iop(), true); }
  virtual int ANDAi(t_mem code) { return And(cA, iop()); }
  virtual int BITAi(t_mem code) { return bit(rA, iop()); }
  virtual int LDAAi(t_mem code) { return lda(cA, iop()); }
  virtual int STAAi(t_mem code) { return lda(idst(), rA); }
  virtual int EORAi(t_mem code) { return eor(cA, iop()); }
  virtual int ADCAi(t_mem code) { return add(cA, iop(), true); }
  virtual int ORAAi(t_mem code) { return Or (cA, iop()); }
  virtual int ADDAi(t_mem code) { return add(cA, iop(), false); }
  virtual int CPXi (t_mem code) { return cpx(iop16()); }
  virtual int JSRi (t_mem code) { return call(iaddr()); }
  virtual int LDSi (t_mem code) { return ldsx(cSP, iop16()); }
  virtual int STSi (t_mem code) { return stsx(iaddr(), rSP); }

  virtual int SUBAe(t_mem code) { return sub(cA, eop(), false); }
  virtual int CMPAe(t_mem code) { return cmp(rA, eop()); }
  virtual int SBCAe(t_mem code) { return sub(cA, eop(), true); }
  virtual int ANDAe(t_mem code) { return And(cA, eop()); }
  virtual int BITAe(t_mem code) { return bit(rA, eop()); }
  virtual int LDAAe(t_mem code) { return lda(cA, eop()); }
  virtual int STAAe(t_mem code) { return lda(edst(), rA); }
  virtual int EORAe(t_mem code) { return eor(cA, eop()); }
  virtual int ADCAe(t_mem code) { return add(cA, eop(), true); }
  virtual int ORAAe(t_mem code) { return Or (cA, eop()); }
  virtual int ADDAe(t_mem code) { return add(cA, eop(), false); }
  virtual int CPXe (t_mem code) { return cpx(eop16()); }
  virtual int JSRe (t_mem code) { return call(eaddr()); }
  virtual int LDSe (t_mem code) { return ldsx(cSP, eop16()); }
  virtual int STSe (t_mem code) { return stsx(eaddr(), rSP); }

  virtual int SUBB8(t_mem code) { return sub(cB, i8(), false); }
  virtual int CMPB8(t_mem code) { return cmp(rB, i8()); }
  virtual int SBCB8(t_mem code) { return sub(cB, i8(), true); }
  virtual int ANDB8(t_mem code) { return And(cB, i8()); }
  virtual int BITB8(t_mem code) { return bit(rB, i8()); }
  virtual int LDAB8(t_mem code) { return lda(cB, i8()); }
  virtual int EORB8(t_mem code) { return eor(cB, i8()); }
  virtual int ADCB8(t_mem code) { return add(cB, i8(), true); }
  virtual int ORAB8(t_mem code) { return Or (cB, i8()); }
  virtual int ADDB8(t_mem code) { return add(cB, i8(), false); }
  virtual int LDX16(t_mem code) { return ldsx(cX, i16()); }

  virtual int SUBBd(t_mem code) { return sub(cB, dop(), false); }
  virtual int CMPBd(t_mem code) { return cmp(rB, dop()); }
  virtual int SBCBd(t_mem code) { return sub(cB, dop(), true); }
  virtual int ANDBd(t_mem code) { return And(cB, dop()); }
  virtual int BITBd(t_mem code) { return bit(rB, dop()); }
  virtual int LDABd(t_mem code) { return lda(cB, dop()); }
  virtual int STABd(t_mem code) { return lda(ddst(), rB); }
  virtual int EORBd(t_mem code) { return eor(cB, dop()); }
  virtual int ADCBd(t_mem code) { return add(cB, dop(), true); }
  virtual int ORABd(t_mem code) { return Or (cB, dop()); }
  virtual int ADDBd(t_mem code) { return add(cB, dop(), false); }
  virtual int LDXd (t_mem code) { return ldsx(cX, dop16()); }
  virtual int STXd (t_mem code) { return stsx(daddr(), rX); }

  virtual int SUBBi(t_mem code) { return sub(cB, iop(), false); }
  virtual int CMPBi(t_mem code) { return cmp(rB, iop()); }
  virtual int SBCBi(t_mem code) { return sub(cB, iop(), true); }
  virtual int ANDBi(t_mem code) { return And(cB, iop()); }
  virtual int BITBi(t_mem code) { return bit(rB, iop()); }
  virtual int LDABi(t_mem code) { return lda(cB, iop()); }
  virtual int STABi(t_mem code) { return lda(idst(), rB); }
  virtual int EORBi(t_mem code) { return eor(cB, iop()); }
  virtual int ADCBi(t_mem code) { return add(cB, iop(), true); }
  virtual int ORABi(t_mem code) { return Or (cB, iop()); }
  virtual int ADDBi(t_mem code) { return add(cB, iop(), false); }
  virtual int LDXi (t_mem code) { return ldsx(cX, iop16()); }
  virtual int STXi (t_mem code) { return stsx(iaddr(), rX); }

  virtual int SUBBe(t_mem code) { return sub(cB, eop(), false); }
  virtual int CMPBe(t_mem code) { return cmp(rB, eop()); }
  virtual int SBCBe(t_mem code) { return sub(cB, eop(), true); }
  virtual int ANDBe(t_mem code) { return And(cB, eop()); }
  virtual int BITBe(t_mem code) { return bit(rB, eop()); }
  virtual int LDABe(t_mem code) { return lda(cB, eop()); }
  virtual int STABe(t_mem code) { return lda(edst(), rB); }
  virtual int EORBe(t_mem code) { return eor(cB, eop()); }
  virtual int ADCBe(t_mem code) { return add(cB, eop(), true); }
  virtual int ORABe(t_mem code) { return Or (cB, eop()); }
  virtual int ADDBe(t_mem code) { return add(cB, eop(), false); }
  virtual int LDXe (t_mem code) { return ldsx(cX, eop16()); }
  virtual int STXe (t_mem code) { return stsx(eaddr(), rX); }
};


/* Unused bits of CC forced to be 1 */

class cl_cc_operator: public cl_memory_operator
{
public:
  cl_cc_operator(class cl_memory_cell *acell): cl_memory_operator(acell) {}
  virtual t_mem write(t_mem val) { return val|= 0xc0; }
};


#endif

/* End of motorola.src/m6800cl.h */
