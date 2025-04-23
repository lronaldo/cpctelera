/*
 * Simulator of microcontrollers (i8080cl.h)
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

#ifndef I8080CL_HEADER
#define I8080CL_HEADER

#include "uccl.h"

#include "glob.h"
#include "decode.h"


#ifdef WORDS_BIGENDIAN
#define REGPAIR(N,N16,NH,NL) union			\
		      {				\
			u16_t N16;		\
			struct {		\
			  u8_t NH;		\
			  u8_t NL;		\
			} r;			\
  } N
#else
#define REGPAIR(N,N16,NH,NL) union			\
		      {				\
			u16_t N16;		\
			struct {		\
			  u8_t NL;		\
			  u8_t NH;		\
			} r;			\
  } N
#endif

#define rAF (rpAF.AF)
#define rA  (rpAF.r.A)
#define rF  (rpAF.r.F)

#define rBC (rpBC.BC)
#define rB  (rpBC.r.B)
#define rC  (rpBC.r.C)

#define rDE (rpDE.DE)
#define rD  (rpDE.r.D)
#define rE  (rpDE.r.E)

#define rHL (rpHL.HL)
#define rH  (rpHL.r.H)
#define rL  (rpHL.r.L)

#define rM  (vc.rd++,rom->read(rHL))


enum {
  flagC	= 0x01,
  flagP	= 0x04,
  flagA	= 0x10,
  flagZ	= 0x40,
  flagS	= 0x80,
  /* 8085 extra flags */
  flagK = 0x20,
  flagX = flagK,
  flagX5= flagK,
  flagV = 0x02,

  fAll= flagC|flagP|flagA|flagZ|flagS|flagK|flagV,
  fAll_C= flagP|flagA|flagZ|flagS|flagK|flagV,
  fAll_A= flagC|flagP|flagZ|flagS|flagK|flagV,
};

#define ADDV(  a,b,r) (((a)&(b)&~(r))|(~(a)&~(b)&(r)))
#define ADDV8( a,b,c) ((ADDV(a,b,c)&0x80)?flagV:0)
#define ADDV16(a,b,c) ((ADDV(a,b,c)&0x8000)?flagV:0)
#define X5(r)         (((rF&flagV)?flagK:0)^(((r)&0x80)?flagK:0))
#define X516(r)       (((rF&flagV)?flagK:0)^(((r)&0x8000)?flagK:0))


/*
 * Special handling of flags
 */

class cl_flag80_op: public cl_memory_operator
{
public:
  cl_flag80_op(class cl_memory_cell *acell): cl_memory_operator(acell) {}
  virtual t_mem write(t_mem val);
};


/*
 * Base of i8080 processor
 */

class cl_i8080: public cl_uc
{
public:
  REGPAIR(rpAF, AF, A, F);  class cl_cell8 cA, cF;  class cl_cell16 cAF;
  REGPAIR(rpBC, BC, B, C);  class cl_cell8 cB, cC;  class cl_cell16 cBC;
  REGPAIR(rpDE, DE, D, E);  class cl_cell8 cD, cE;  class cl_cell16 cDE;
  REGPAIR(rpHL, HL, H, L);  class cl_cell8 cH, cL;  class cl_cell16 cHL;
  u16_t rSP; class cl_cell16 cSP;
  int tick_shift;
  t_addr sp_limit;
public:
  cl_i8080(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_cpu_hw(void);
  virtual void make_memories(void);
  virtual class cl_memory_operator *make_flag_op(void);

  virtual double def_xtal(void) { return 1000000; }
  virtual int clock_per_cycle(void) { return 1; }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual void dis_M(chars *comment);
  virtual void dis_rp8(chars *comment, int rp);
  virtual void dis_rp16(chars *comment, int rp);
  virtual char *disassc(t_addr addr, chars *comment=NULL);

  virtual void print_regs(class cl_console_base *con);

  virtual u16_t *tick_tab(void) { return tick_tab_8080; }
  virtual class cl_memory_cell &cM(void);
  virtual u16_t fetch16(void);
  virtual int exec_inst(void);

  virtual void push2(u16_t v);
  virtual u16_t pop2(void);
  virtual void stack_check_overflow(t_addr sp_before);
  virtual int NOP(t_mem code) { return resGO; }
  virtual int HLT(t_mem code);
  virtual int EI(t_mem code);
  virtual int DI(t_mem code);

  // Data transfer
  virtual int mvi8(class cl_memory_cell &dst);
  virtual int lxi16(class cl_memory_cell &dst);
  virtual int ldax(u16_t a);
  virtual int stax(u16_t a);
#include "imovrr.h"
  virtual int MVI_Ai8(t_mem code) { return mvi8(cA); }
  virtual int MVI_Bi8(t_mem code) { return mvi8(cB); }
  virtual int MVI_Ci8(t_mem code) { return mvi8(cC); }
  virtual int MVI_Di8(t_mem code) { return mvi8(cD); }
  virtual int MVI_Ei8(t_mem code) { return mvi8(cE); }
  virtual int MVI_Hi8(t_mem code) { return mvi8(cH); }
  virtual int MVI_Li8(t_mem code) { return mvi8(cL); }
  virtual int MVI_Mi8(t_mem code) { return mvi8(cM()); }

  virtual int LXI_Bi16(t_mem code) { return lxi16(cBC); }
  virtual int LXI_Di16(t_mem code) { return lxi16(cDE); }
  virtual int LXI_Hi16(t_mem code) { return lxi16(cHL); }
  virtual int LXI_Si16(t_mem code) { return lxi16(cSP); }

  virtual int LDA_a16(t_mem code);
  virtual int STA_a16(t_mem code);
  virtual int LHLD_a16(t_mem code);
  virtual int SHLD_a16(t_mem code);
  virtual int LDAX_B(t_mem code) { return ldax(rBC); }
  virtual int LDAX_D(t_mem code) { return ldax(rDE); }
  virtual int STAX_B(t_mem code) { return stax(rBC); }
  virtual int STAX_D(t_mem code) { return stax(rDE); }
  virtual int XCHG(t_mem code);
  virtual int IN_INST(t_mem code);
  virtual int OUT_INST(t_mem code);
  virtual int PUSH_B(t_mem code) { push2(rBC); return resGO; }
  virtual int PUSH_D(t_mem code) { push2(rDE); return resGO; }
  virtual int PUSH_H(t_mem code) { push2(rHL); return resGO; }
  virtual int PUSH_PSW(t_mem code) { push2(rAF); return resGO; }
  virtual int POP_B(t_mem code) { cBC.W(pop2()); return resGO; }
  virtual int POP_D(t_mem code) { cDE.W(pop2()); return resGO; }
  virtual int POP_H(t_mem code) { cHL.W(pop2()); return resGO; }
  virtual int POP_PSW(t_mem code) { cAF.W(pop2()); return resGO; }
  virtual int XTHL(t_mem code);
  virtual int SPHL(t_mem code);
  
  // Arithmetic
  virtual int add8(u8_t op, bool add_c, bool is_daa= false);
  virtual int sub8(u8_t op, bool sub_c, bool cmp);
  virtual int ana(u8_t op);
  virtual int ora(u8_t op);
  virtual int xra(u8_t op);
  virtual int ADD_A(t_mem code) { return add8(rA, false); }
  virtual int ADD_B(t_mem code) { return add8(rB, false); }
  virtual int ADD_C(t_mem code) { return add8(rC, false); }
  virtual int ADD_D(t_mem code) { return add8(rD, false); }
  virtual int ADD_E(t_mem code) { return add8(rE, false); }
  virtual int ADD_H(t_mem code) { return add8(rH, false); }
  virtual int ADD_L(t_mem code) { return add8(rL, false); }
  virtual int ADD_M(t_mem code) { return add8(rM, false); }
  virtual int ADI(t_mem code) { return add8(fetch(), false); }
  virtual int ADC_A(t_mem code) { return add8(rA, true); }
  virtual int ADC_B(t_mem code) { return add8(rB, true); }
  virtual int ADC_C(t_mem code) { return add8(rC, true); }
  virtual int ADC_D(t_mem code) { return add8(rD, true); }
  virtual int ADC_E(t_mem code) { return add8(rE, true); }
  virtual int ADC_H(t_mem code) { return add8(rH, true); }
  virtual int ADC_L(t_mem code) { return add8(rL, true); }
  virtual int ADC_M(t_mem code) { return add8(rM, true); }
  virtual int ACI(t_mem code) { return add8(fetch(), true); }
  
  virtual int SUB_A(t_mem code) { return sub8(rA, false, false); }
  virtual int SUB_B(t_mem code) { return sub8(rB, false, false); }
  virtual int SUB_C(t_mem code) { return sub8(rC, false, false); }
  virtual int SUB_D(t_mem code) { return sub8(rD, false, false); }
  virtual int SUB_E(t_mem code) { return sub8(rE, false, false); }
  virtual int SUB_H(t_mem code) { return sub8(rH, false, false); }
  virtual int SUB_L(t_mem code) { return sub8(rL, false, false); }
  virtual int SUB_M(t_mem code) { return sub8(rM, false, false); }
  virtual int SUI(t_mem code) { return sub8(fetch(), false, false); }
  virtual int SBB_A(t_mem code) { return sub8(rA, true, false); }
  virtual int SBB_B(t_mem code) { return sub8(rB, true, false); }
  virtual int SBB_C(t_mem code) { return sub8(rC, true, false); }
  virtual int SBB_D(t_mem code) { return sub8(rD, true, false); }
  virtual int SBB_E(t_mem code) { return sub8(rE, true, false); }
  virtual int SBB_H(t_mem code) { return sub8(rH, true, false); }
  virtual int SBB_L(t_mem code) { return sub8(rL, true, false); }
  virtual int SBB_M(t_mem code) { return sub8(rM, true, false); }
  virtual int SBI(t_mem code) { return sub8(fetch(), true, false); }
  virtual int CMP_A(t_mem code) { return sub8(rA, false, true); }
  virtual int CMP_B(t_mem code) { return sub8(rB, false, true); }
  virtual int CMP_C(t_mem code) { return sub8(rC, false, true); }
  virtual int CMP_D(t_mem code) { return sub8(rD, false, true); }
  virtual int CMP_E(t_mem code) { return sub8(rE, false, true); }
  virtual int CMP_H(t_mem code) { return sub8(rH, false, true); }
  virtual int CMP_L(t_mem code) { return sub8(rL, false, true); }
  virtual int CMP_M(t_mem code) { return sub8(rM, false, true); }
  virtual int CPI(t_mem code) { return sub8(fetch(), false, true); }
  virtual int dad(u16_t op);
  virtual int DAD_B(t_mem code) { return dad(rBC); }
  virtual int DAD_D(t_mem code) { return dad(rDE); }
  virtual int DAD_H(t_mem code) { return dad(rHL); }
  virtual int DAD_S(t_mem code) { return dad(rSP); }
  virtual int inr(class cl_memory_cell &op);
  virtual int INR_A(t_mem code) { return inr(cA); }
  virtual int INR_B(t_mem code) { return inr(cB); }
  virtual int INR_C(t_mem code) { return inr(cC); }
  virtual int INR_D(t_mem code) { return inr(cD); }
  virtual int INR_E(t_mem code) { return inr(cE); }
  virtual int INR_H(t_mem code) { return inr(cH); }
  virtual int INR_L(t_mem code) { return inr(cL); }
  virtual int INR_M(t_mem code) { vc.rd++; return inr(cM()); }
  virtual int dcr(class cl_memory_cell &op);
  virtual int DCR_A(t_mem code) { return dcr(cA); }
  virtual int DCR_B(t_mem code) { return dcr(cB); }
  virtual int DCR_C(t_mem code) { return dcr(cC); }
  virtual int DCR_D(t_mem code) { return dcr(cD); }
  virtual int DCR_E(t_mem code) { return dcr(cE); }
  virtual int DCR_H(t_mem code) { return dcr(cH); }
  virtual int DCR_L(t_mem code) { return dcr(cL); }
  virtual int DCR_M(t_mem code) { vc.rd++; return dcr(cM()); }
  virtual int inx(class cl_memory_cell &op);
  virtual int dcx(class cl_memory_cell &op);
  virtual int INX_B(t_mem code) { return inx(cBC); }
  virtual int INX_D(t_mem code) { return inx(cDE); }
  virtual int INX_H(t_mem code) { return inx(cHL); }
  virtual int INX_S(t_mem code) { return inx(cSP); }
  virtual int DCX_B(t_mem code) { return dcx(cBC); }
  virtual int DCX_D(t_mem code) { return dcx(cDE); }
  virtual int DCX_H(t_mem code) { return dcx(cHL); }
  virtual int DCX_S(t_mem code) { return dcx(cSP); }

  virtual int ANA_A(t_mem code) { return ana(rA); }
  virtual int ANA_B(t_mem code) { return ana(rB); }
  virtual int ANA_C(t_mem code) { return ana(rC); }
  virtual int ANA_D(t_mem code) { return ana(rD); }
  virtual int ANA_E(t_mem code) { return ana(rE); }
  virtual int ANA_H(t_mem code) { return ana(rH); }
  virtual int ANA_L(t_mem code) { return ana(rL); }
  virtual int ANA_M(t_mem code) { return ana(rM); }
  virtual int ANI(t_mem code) { return ana(fetch()); }
  virtual int ORA_A(t_mem code) { return ora(rA); }
  virtual int ORA_B(t_mem code) { return ora(rB); }
  virtual int ORA_C(t_mem code) { return ora(rC); }
  virtual int ORA_D(t_mem code) { return ora(rD); }
  virtual int ORA_E(t_mem code) { return ora(rE); }
  virtual int ORA_H(t_mem code) { return ora(rH); }
  virtual int ORA_L(t_mem code) { return ora(rL); }
  virtual int ORA_M(t_mem code) { return ora(rM); }
  virtual int ORI(t_mem code) { return ora(fetch()); }
  virtual int XRA_A(t_mem code) { return xra(rA); }
  virtual int XRA_B(t_mem code) { return xra(rB); }
  virtual int XRA_C(t_mem code) { return xra(rC); }
  virtual int XRA_D(t_mem code) { return xra(rD); }
  virtual int XRA_E(t_mem code) { return xra(rE); }
  virtual int XRA_H(t_mem code) { return xra(rH); }
  virtual int XRA_L(t_mem code) { return xra(rL); }
  virtual int XRA_M(t_mem code) { return xra(rM); }
  virtual int XRI(t_mem code) { return xra(fetch()); }

  virtual int RLC(t_mem code);
  virtual int RRC(t_mem code);
  virtual int RAL(t_mem code);
  virtual int RAR(t_mem code);

  virtual int DAA(t_mem code);
  virtual int CMA(t_mem code);
  virtual int CMC(t_mem code);
  virtual int STC(t_mem code);

  // Branches
  virtual int jmp_if(bool cond);
  virtual int call_if(bool cond);
  virtual int ret_if(bool cond);
  virtual int rst(t_mem code);
  virtual int JMP(t_mem code);
  virtual int JNZ(t_mem code) { return jmp_if(!(rF&flagZ)); }
  virtual int JZ (t_mem code) { return jmp_if(  rF&flagZ); }
  virtual int JNC(t_mem code) { return jmp_if(!(rF&flagC)); }
  virtual int JC (t_mem code) { return jmp_if(  rF&flagC); }
  virtual int JPO(t_mem code) { return jmp_if(!(rF&flagP)); }
  virtual int JPE(t_mem code) { return jmp_if(  rF&flagP); }
  virtual int JP (t_mem code) { return jmp_if(!(rF&flagS)); }
  virtual int JM (t_mem code) { return jmp_if(  rF&flagS); }

  virtual int CALL(t_mem code);
  virtual int CNZ(t_mem code) { return call_if(!(rF&flagZ)); }
  virtual int CZ (t_mem code) { return call_if(  rF&flagZ); }
  virtual int CNC(t_mem code) { return call_if(!(rF&flagC)); }
  virtual int CC (t_mem code) { return call_if(  rF&flagC); }
  virtual int CPO(t_mem code) { return call_if(!(rF&flagP)); }
  virtual int CPE(t_mem code) { return call_if(  rF&flagP); }
  virtual int CP (t_mem code) { return call_if(!(rF&flagS)); }
  virtual int CM (t_mem code) { return call_if(  rF&flagS); }

  virtual int RET(t_mem code);
  virtual int RNZ(t_mem code) { return ret_if(!(rF&flagZ)); }
  virtual int RZ (t_mem code) { return ret_if(  rF&flagZ); }
  virtual int RNC(t_mem code) { return ret_if(!(rF&flagC)); }
  virtual int RC (t_mem code) { return ret_if(  rF&flagC); }
  virtual int RPO(t_mem code) { return ret_if(!(rF&flagP)); }
  virtual int RPE(t_mem code) { return ret_if(  rF&flagP); }
  virtual int RP (t_mem code) { return ret_if(!(rF&flagS)); }
  virtual int RM (t_mem code) { return ret_if(  rF&flagS); }

  virtual int RST_0(t_mem code) { return rst(code); }
  virtual int RST_1(t_mem code) { return rst(code); }
  virtual int RST_2(t_mem code) { return rst(code); }
  virtual int RST_3(t_mem code) { return rst(code); }
  virtual int RST_4(t_mem code) { return rst(code); }
  virtual int RST_5(t_mem code) { return rst(code); }
  virtual int RST_6(t_mem code) { return rst(code); }
  virtual int RST_7(t_mem code) { return rst(code); }

  virtual int PCHL(t_mem code);
};


enum i8080cpu_confs
  {
   i8080cpu_sp_limit	= 0,
   i8080cpu_nuof	= 1
  };

class cl_i8080_cpu: public cl_hw
{
public:
  cl_i8080_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return i8080cpu_nuof; }
  virtual const char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of i8085.src/i8080cl.h */
