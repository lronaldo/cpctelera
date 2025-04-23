/*
 * Simulator of microcontrollers (f8cl.h)
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

#ifndef F8CL_HEADER
#define F8CL_HEADER

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

#define rX (rpX.X)
#define rXL (rpX.r.XL)
#define rXH (rpX.r.XH)
#define rY (rpY.Y)
#define rYL (rpY.r.YL)
#define rYH (rpY.r.YH)
#define rZ (rpZ.Z)
#define rZL (rpZ.r.ZL)
#define rZH (rpZ.r.ZH)


enum {
  flagO	 = 0x01,
  flagZ	 = 0x02,
  flagN	 = 0x04,
  flagC	 = 0x08,
  flagH	 = 0x10,

  flagA  = flagH,
  flagS  = flagN,
  flagV  = flagO,

  flagCZ = flagC|flagZ,
  flagZN = flagZ|flagN,
  flagCZN= flagC|flagZ|flagN,
  flagOZN= flagO|flagZ|flagN,
  
  fAll   = flagO|flagZ|flagN|flagC|flagH,
  fAll_H = flagO|flagZ|flagN|flagC
};

#define COND_Z		(rF&flagZ)
#define COND_NZ		(!(rF&flagZ))
#define COND_C		(rF&flagC)
#define COND_NC		(!(rF&flagC))
#define COND_N		(rF&flagN)
#define COND_NN		(!(rF&flagN))
#define COND_O		(rF&flagO)
#define COND_NO		(!(rF&flagO))
#define COND_SGE	((COND_N&&COND_O)||(!COND_N&&!COND_O))
#define COND_SLT	((COND_N&&!COND_O)||(!COND_N&&COND_O))
#define COND_SGT	(!(COND_Z||(COND_SLT)))
#define COND_SLE	(!COND_SGT)
#define COND_GT		(COND_C&&!COND_Z)
#define COND_LE		(!COND_C||COND_Z)

enum {
  P_NONE	= 0,
  P_SWAP	= 0x01, // (0) swapop
  P_ALT1	= 0x02, // (1) altacc1    XH
  P_ALT2	= 0x04, // (2) altacc2    YL  X
  P_ALT3	= 0x08, // (2) altacc3    ZL  Z
  P_ALT4	= 0x10, // (2) altacc4    YH  Z
  P_ALT5	= 0x20, // (2) altacc5    ZH
};

#define IFSWAP if (prefixes&P_SWAP)

/*
 * Base of f8 processor
 */
class cl_f8: public cl_uc
{
public:
  REGPAIR(rpX, X, XH, XL); class cl_cell8 cXH, cXL; class cl_cell16 cX;
  REGPAIR(rpY, Y, YH, YL); class cl_cell8 cYH, cYL; class cl_cell16 cY;
  REGPAIR(rpZ, Z, ZH, ZL); class cl_cell8 cZH, cZL; class cl_cell16 cZ;
  u16_t rSP; class cl_cell16 cSP;
  u8_t rF; class cl_cell8 cF;
  t_addr sp_limit;
  class cl_cell8 *acc8;
  class cl_cell16 *acc16;
  class cl_cell16 *rop16; // The 16-bit register that acc8 is part of.
  int prefixes;
public:
  cl_f8(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_cpu_hw(void);
  virtual void make_memories(void);

  virtual double def_xtal(void) { return 25000000; }
  virtual int clock_per_cycle(void) { return 1; }
  virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual int inst_length(t_addr addr);
  virtual u8_t a8(u8_t prefs);
  virtual u16_t a16(u8_t prefs);
  virtual const char *a8_name(u8_t prefs);
  virtual const char *a16_name(u8_t prefs);
  virtual const char *r16_name(u8_t prefs);
  virtual const char *a16h_name(u8_t prefs);
  virtual const char *a16l_name(u8_t prefs);
  virtual char *disassc(t_addr addr, chars *comment=NULL);
  virtual int longest_inst(void) { return 5; }

  virtual void print_regs(class cl_console_base *con);

  virtual u16_t fetch16(void) { u16_t v= fetch(); v+= fetch()*256; return v; }
  virtual i8_t d(void) { return fetch(); }
  virtual i16_t sexd(void);
  virtual void push1(u8_t v);
  virtual void push2(u16_t v);
  virtual void push2(u8_t h, u8_t l);
  virtual u8_t pop1(void);
  virtual u16_t pop2(void);
  virtual void stack_check_overflow(t_addr sp_before);
  
  // memory cells addressed by 8 bit addressing modes
  // call necessary fetches
  virtual class cl_cell8 &m_i(void);
  virtual class cl_cell8 &m_mm(void);
  virtual class cl_cell8 &m_n_sp(void);
  virtual class cl_cell8 &m_nn_z(void);
  virtual class cl_cell8 &m_y(void);
  virtual class cl_cell8 &m_n_y(void);
  virtual class cl_cell8 &m_z(void);

  // memory addresses by addressing modes
  // call necessary fetches
  virtual u16_t a_i(void);
  virtual u16_t a_mm(void);
  virtual u16_t a_n_sp(void);
  virtual u16_t a_nn_z(void);
  virtual u16_t a_y(void);
  virtual u16_t a_n_y();
  virtual u16_t a_acc16(void);
  
  virtual void clear_prefixes();
  virtual int exec_inst(void);

  // data moves: imove.cc
  // 8 bit moves
  int ld8_a_i(u8_t op2);
  int ld8_a_m(class cl_cell8 &m);
  int ld8_m_a(class cl_cell8 &m);
  int ld8_a_r(class cl_cell8 &r);
  int LD8_A_I(t_mem code)   { return ld8_a_i(fetch()); }
  int LD8_A_M(t_mem code)   { return ld8_a_m(m_mm()); }
  int LD8_A_NSP(t_mem code) { return ld8_a_m(m_n_sp()); }
  int LD8_A_NNZ(t_mem code) { return ld8_a_m(m_nn_z()); }
  int LD8_A_Y(t_mem code)   { return ld8_a_m(m_y()); }
  int LD8_A_NY(t_mem code)  { return ld8_a_m(m_n_y()); }
  int LD8_A_XH(t_mem code)  { return ld8_a_r(cXH); }
  int LD8_A_YL(t_mem code)  { return ld8_a_r(cYL); }
  int LD8_A_YH(t_mem code)  { return ld8_a_r(cYH); }
  int LD8_A_ZL(t_mem code)  { return ld8_a_r(cZL); }
  int LD8_A_ZH(t_mem code)  { return ld8_a_r(cZH); }
  int LD8_M_A(t_mem code)   { return ld8_m_a(m_mm()); }
  int LD8_NSP_A(t_mem code) { return ld8_m_a(m_n_sp()); }
  int LD8_NNZ_A(t_mem code) { return ld8_m_a(m_nn_z()); }
  int LD8_Y_A(t_mem code)   { return ld8_m_a(m_y()); }
  int LD8_NY_A(t_mem code)  { return ld8_m_a(m_n_y()); }
  // 16 bit moves
  int ldw_a_i(u16_t op2);
  int ldw_a_m(u16_t addr);
  int ldw_m_a(u16_t addr);
  int ldw_m_r(u16_t addr, u16_t r);
  int ldw_a_r(u16_t r);
  int ldw_r_m(u16_t addr);
  int LDW_A_I(t_mem code)    { return ldw_a_i(fetch16()); }
  int LDW_A_M(t_mem code)    { return ldw_a_m(a_mm()); }
  int LDW_A_NSP(t_mem code)  { return ldw_a_m(a_n_sp()); }
  int LDW_A_NNZ(t_mem code)  { return ldw_a_m(a_nn_z()); }
  int LDW_A_NY(t_mem code)   { return ldw_a_m(a_n_y()); }
  int LDW_A_AM(t_mem code)   { return ldw_a_m(acc16->get()); }
  int LDW_A_X(t_mem code)    { return ldw_a_r(rX); }
  int LDW_A_Z(t_mem code)    { return ldw_a_r(rZ); }
  int LDW_A_D(t_mem code)    { return ldw_a_i(sexd()); }
  int LDW_M_A(t_mem code)    { return ldw_m_a(a_mm()); }
  int LDW_NSP_A(t_mem code)  { return ldw_m_a(a_n_sp()); }
  int LDW_NNZ_A(t_mem code)  { return ldw_m_a(a_nn_z()); }
  int LDW_X_A(t_mem code)    { cX.W(acc16->get()); return resGO; }
  int LDW_Z_A(t_mem code)    { cZ.W(acc16->get()); return resGO; }
  int LDW_AM_X(t_mem code)   { return ldw_m_r(a_acc16(), rop16->get()); }
  int LDW_NAM_X(t_mem code)  { return ldw_m_r(a_n_y(), rop16->get()); }
  int LDW_DSP_A(t_mem code);
  int LDW_X_AM(t_mem code)   { return ldw_r_m(acc16->get()); }
  // other moves
  int LDI_Y_Z(t_mem code);
  int LDWI_Y_Z(t_mem code);
  int PUSH_M(t_mem code);
  int PUSH_NSP(t_mem code);
  int PUSH_A(t_mem code);
  int PUSH_NY(t_mem code);
  int PUSH_I(t_mem code);
  int PUSHW_M(t_mem code);
  int PUSHW_NSP(t_mem code);
  int PUSHW_NNZ(t_mem code);
  int PUSHW_A(t_mem code);
  int PUSHW_I(t_mem code);
  int POP_A(t_mem code);
  int POPW_A(t_mem code);
  int XCH_A_NSP(t_mem code);
  int XCH_A_Y(t_mem code);
  int XCH_A_A(t_mem code);
  int XCHW_X_Y(t_mem code);
  int XCHW_Y_NSP(t_mem code);
  int CAX(t_mem code);
  int CAXW(t_mem code);
  int CLR_M(t_mem code);
  int CLR_NSP(t_mem code);
  int CLR_A(t_mem code);
  int CLR_NY(t_mem code);
  int CLRW_M(t_mem code);
  int CLRW_NSP(t_mem code);
  int CLRW_NNZ(t_mem code);
  int CLRW_A(t_mem code);
  int xchb(int b);
  int XCHB_0(t_mem code) { return xchb(0); }
  int XCHB_1(t_mem code) { return xchb(1); }
  int XCHB_2(t_mem code) { return xchb(2); }
  int XCHB_3(t_mem code) { return xchb(3); }
  int XCHB_4(t_mem code) { return xchb(4); }
  int XCHB_5(t_mem code) { return xchb(5); }
  int XCHB_6(t_mem code) { return xchb(6); }
  int XCHB_7(t_mem code) { return xchb(7); }
  
  // arithmetic (ALU) instructions: ialu.cc
  // 8-bit 2-op-inst
  int add8(class cl_cell8 *op1, class cl_cell8 *op2, bool usec, bool memop);
  int sub8(class cl_cell8 *op1, class cl_cell8 *op2, bool usec, bool memop, bool cmp);
  int Or8 (class cl_cell8 *op1, class cl_cell8 *op2, bool memop);
  int And8(class cl_cell8 *op1, class cl_cell8 *op2, bool memop);
  int Xor8(class cl_cell8 *op1, class cl_cell8 *op2, bool memop);
  int ADD_I  (t_mem code) { return add8(acc8, &m_i()   , false, false); }
  int ADD_M  (t_mem code) { return add8(acc8, &m_mm()  , false, true ); }
  int ADD_NSP(t_mem code) { return add8(acc8, &m_n_sp(), false, true ); }
  int ADD_NNZ(t_mem code) { return add8(acc8, &m_nn_z(), false, true ); }
  int ADD_ZL (t_mem code) { return add8(acc8, &cZL     , false, false); }
  int ADD_XH (t_mem code) { return add8(acc8, &cXH     , false, false); }
  int ADD_YL (t_mem code) { return add8(acc8, &cYL     , false, false); }
  int ADD_YH (t_mem code) { return add8(acc8, &cYH     , false, false); }
  int ADC_I  (t_mem code) { return add8(acc8, &m_i()   , true , false); }
  int ADC_M  (t_mem code) { return add8(acc8, &m_mm()  , true , true ); }
  int ADC_NSP(t_mem code) { return add8(acc8, &m_n_sp(), true , true ); }
  int ADC_NNZ(t_mem code) { return add8(acc8, &m_nn_z(), true , true ); }
  int ADC_ZL (t_mem code) { return add8(acc8, &cZL     , true , false); }
  int ADC_XH (t_mem code) { return add8(acc8, &cXH     , true , false); }
  int ADC_YL (t_mem code) { return add8(acc8, &cYL     , true , false); }
  int ADC_YH (t_mem code) { return add8(acc8, &cYH     , true , false); }
  int SUB_M  (t_mem code) { return sub8(acc8, &m_mm()  , false, true , false); }
  int SUB_NSP(t_mem code) { return sub8(acc8, &m_n_sp(), false, true , false); }
  int SUB_NNZ(t_mem code) { return sub8(acc8, &m_nn_z(), false, true , false); }
  int SUB_ZL (t_mem code) { return sub8(acc8, &cZL     , false, false, false); }
  int SUB_XH (t_mem code) { return sub8(acc8, &cXH     , false, false, false); }
  int SUB_YL (t_mem code) { return sub8(acc8, &cYL     , false, false, false); }
  int SUB_YH (t_mem code) { return sub8(acc8, &cYH     , false, false, false); }
  int SBC_M  (t_mem code) { return sub8(acc8, &m_mm()  , true , true , false); }
  int SBC_NSP(t_mem code) { return sub8(acc8, &m_n_sp(), true , true , false); }
  int SBC_NNZ(t_mem code) { return sub8(acc8, &m_nn_z(), true , true , false); }
  int SBC_ZL (t_mem code) { return sub8(acc8, &cZL     , true , false, false); }
  int SBC_XH (t_mem code) { return sub8(acc8, &cXH     , true , false, false); }
  int SBC_YL (t_mem code) { return sub8(acc8, &cYL     , true , false, false); }
  int SBC_YH (t_mem code) { return sub8(acc8, &cYH     , true , false, false); }
  int CP_I   (t_mem code) { return sub8(acc8, &m_i()   , false, true , true ); }
  int CP_M   (t_mem code) { return sub8(acc8, &m_mm()  , false, true , true ); }
  int CP_NSP (t_mem code) { return sub8(acc8, &m_n_sp(), false, true , true ); }
  int CP_NNZ (t_mem code) { return sub8(acc8, &m_nn_z(), false, true , true ); }
  int CP_ZL  (t_mem code) { return sub8(acc8, &cZL     , false, false, true ); }
  int CP_XH  (t_mem code) { return sub8(acc8, &cXH     , false, false, true ); }
  int CP_YL  (t_mem code) { return sub8(acc8, &cYL     , false, false, true ); }
  int CP_YH  (t_mem code) { return sub8(acc8, &cYH     , false, false, true ); }
  int OR_I   (t_mem code) { return Or8 (acc8, &m_i()   , true  ); }
  int OR_M   (t_mem code) { return Or8 (acc8, &m_mm()  , true  ); }
  int OR_NSP (t_mem code) { return Or8 (acc8, &m_n_sp(), true  ); }
  int OR_NNZ (t_mem code) { return Or8 (acc8, &m_nn_z(), true  ); }
  int OR_ZL  (t_mem code) { return Or8 (acc8, &cZL     , false ); }
  int OR_XH  (t_mem code) { return Or8 (acc8, &cXH     , false ); }
  int OR_YL  (t_mem code) { return Or8 (acc8, &cYL     , false ); }
  int OR_YH  (t_mem code) { return Or8 (acc8, &cYH     , false ); }
  int AND_I  (t_mem code) { return And8(acc8, &m_i()   , true  ); }
  int AND_M  (t_mem code) { return And8(acc8, &m_mm()  , true  ); }
  int AND_NSP(t_mem code) { return And8(acc8, &m_n_sp(), true  ); }
  int AND_NNZ(t_mem code) { return And8(acc8, &m_nn_z(), true  ); }
  int AND_ZL (t_mem code) { return And8(acc8, &cZL     , false ); }
  int AND_XH (t_mem code) { return And8(acc8, &cXH     , false ); }
  int AND_YL (t_mem code) { return And8(acc8, &cYL     , false ); }
  int AND_YH (t_mem code) { return And8(acc8, &cYH     , false ); }
  int XOR_I  (t_mem code) { return Xor8(acc8, &m_i()   , true  ); }
  int XOR_M  (t_mem code) { return Xor8(acc8, &m_mm()  , true  ); }
  int XOR_NSP(t_mem code) { return Xor8(acc8, &m_n_sp(), true  ); }
  int XOR_NNZ(t_mem code) { return Xor8(acc8, &m_nn_z(), true  ); }
  int XOR_ZL (t_mem code) { return Xor8(acc8, &cZL     , false ); }
  int XOR_XH (t_mem code) { return Xor8(acc8, &cXH     , false ); }
  int XOR_YL (t_mem code) { return Xor8(acc8, &cYL     , false ); }
  int XOR_YH (t_mem code) { return Xor8(acc8, &cYH     , false ); }
  // 8-bit 1-op-inst
  int SRL_M(t_mem code);
  int SRL_NSP(t_mem code);
  int SRL_A(t_mem code);
  int SRL_NY(t_mem code);
  int SLL_M(t_mem code);
  int SLL_NSP(t_mem code);
  int SLL_A(t_mem code);
  int SLL_NY(t_mem code);
  int RRC_M(t_mem code);
  int RRC_NSP(t_mem code);
  int RRC_A(t_mem code);
  int RRC_NY(t_mem code);
  int RLC_M(t_mem code);
  int RLC_NSP(t_mem code);
  int RLC_A(t_mem code);
  int RLC_NY(t_mem code);
  int INC_M(t_mem code);
  int INC_NSP(t_mem code);
  int INC_A(t_mem code);
  int INC_NY(t_mem code);
  int DEC_M(t_mem code);
  int DEC_NSP(t_mem code);
  int DEC_A(t_mem code);
  int DEC_NY(t_mem code);
  int TST_M(t_mem code);
  int TST_NSP(t_mem code);
  int TST_A(t_mem code);
  int TST_NY(t_mem code);
  
  // 16-bit 2-op-inst
  virtual u16_t add16(u16_t a, u16_t b, int c, bool sub);
  virtual int add16(u16_t opaddr, bool usec);
  virtual int add16(/*op2=x*/bool usec);
  virtual int sub16(u16_t opaddr, bool usec);
  virtual int sub16(/*op2=x*/bool usec);
  virtual u16_t or16(u16_t a, u16_t b);
  virtual int or16(u16_t opaddr);
  virtual int or16(void);
  virtual u16_t xor16(u16_t a, u16_t b);
  virtual int xor16(u16_t opaddr);
  virtual int xor16(void);
  int SUBW_M  (t_mem code) { return sub16(a_mm()  , false); }
  int SUBW_NSP(t_mem code) { return sub16(a_n_sp(), false); }
  int SUBW_X  (t_mem code) { return sub16(          false); }
  int SBCW_M  (t_mem code) { return sub16(a_mm()  , true); }
  int SBCW_NSP(t_mem code) { return sub16(a_n_sp(), true); }
  int SBCW_X  (t_mem code) { return sub16(          true); }
  int ADDW_I  (t_mem code) { return add16(a_i()   , false); }
  int ADDW_M  (t_mem code) { return add16(a_mm()  , false); }
  int ADDW_NSP(t_mem code) { return add16(a_n_sp(), false); }
  int ADDW_X  (t_mem code) { return add16(          false); }
  int ADCW_I  (t_mem code) { return add16(a_i()   , true); }
  int ADCW_M  (t_mem code) { return add16(a_mm()  , true); }
  int ADCW_NSP(t_mem code) { return add16(a_n_sp(), true); }
  int ADCW_X  (t_mem code) { return add16(          true); }
  int ORW_I   (t_mem code) { return or16(a_i()    ); }
  int ORW_M   (t_mem code) { return or16(a_mm()   ); }
  int ORW_NSP (t_mem code) { return or16(a_n_sp() ); }
  int ORW_X   (t_mem code) { return or16(         ); }
  int XORW_I  (t_mem code) { return xor16(a_i()   ); }
  int XORW_M  (t_mem code) { return xor16(a_mm()  ); }
  int XORW_NSP(t_mem code) { return xor16(a_n_sp()); }
  int XORW_X  (t_mem code) { return xor16(        ); }

  // 16-bit 1-op-inst
  int INCW_M(t_mem code);
  int INCW_NSP(t_mem code);
  int INCW_NNZ(t_mem code);
  int INCW_A(t_mem code);
  int ADCW1_M(t_mem code);
  int ADCW1_NSP(t_mem code);
  int ADCW1_NNZ(t_mem code);
  int ADCW1_A(t_mem code);
  int SBCW1_M(t_mem code);
  int SBCW1_NSP(t_mem code);
  int SBCW1_NNZ(t_mem code);
  int SBCW1_A(t_mem code);
  int TSTW1_M(t_mem code);
  int TSTW1_NSP(t_mem code);
  int TSTW1_NNZ(t_mem code);
  int TSTW1_A(t_mem code);

  // 8-bit 0-op-inst
  int mad(class cl_cell8 &op);
  int ROT(t_mem code);
  int SRA(t_mem code);
  int DAA(t_mem code);
  int BOOL_A(t_mem code);
  int MSK(t_mem code);
  int MAD_M(t_mem code)   { return mad(m_mm()); }
  int MAD_NSP(t_mem code) { return mad(m_n_sp()); }
  int MAD_NNZ(t_mem code) { return mad(m_nn_z()); }
  int MAD_Z(t_mem code)   { return mad(m_z()); }
  int XCH_F_NSP(t_mem code);

  // 16-bit 0-op-inst
  int MUL(t_mem code);
  int NEGW(t_mem code);
  int BOOLW(t_mem code);
  int SRLW(t_mem code);
  int SLLW(t_mem code);
  int RRCW(t_mem code);
  int RLCW_A(t_mem code);
  int RLCW_NSP(t_mem code);
  int RRCW_NSP(t_mem code);
  int SRAW(t_mem code);
  int ADDW_SP_D(t_mem code);
  int ADDW_A_D(t_mem code);
  int LDW_A_SP(t_mem code);
  int CPW(t_mem code);
  int INCNW(t_mem code);
  int DECW_NSP(t_mem code);
  int SLLW_A_XL(t_mem code);
  int SEX(t_mem code);
  int ZEX(t_mem code);
  
  // branches: ibranch.cc
  virtual int JP_I(t_mem code);
  virtual int JP_A(t_mem code);
  virtual int CALL_I(t_mem code);
  virtual int CALL_A(t_mem code);
  virtual int RET(t_mem code);
  virtual int RETI(t_mem code);
  virtual int jr(bool cond);
  virtual int JR(t_mem code)    { return jr(true); }
  virtual int DNJNZ(t_mem code);
  virtual int JRZ(t_mem code)   { return jr(COND_Z); }
  virtual int JRNZ(t_mem code)  { return jr(COND_NZ); }
  virtual int JRC(t_mem code)   { return jr(COND_C); }
  virtual int JRNC(t_mem code)  { return jr(COND_NC); }
  virtual int JRN(t_mem code)   { return jr(COND_N); }
  virtual int JRNN(t_mem code)  { return jr(COND_NN); }
  virtual int JRNO(t_mem code)  { return jr((prefixes&P_SWAP) ? COND_O : COND_NO); }
  virtual int JRSGE(t_mem code) { return jr(COND_SGE); }
  virtual int JRSLT(t_mem code) { return jr(COND_SLT); }
  virtual int JRSLE(t_mem code) { return jr((prefixes&P_SWAP) ? COND_SGT : COND_SLE); }
  virtual int JRLE(t_mem code)  { return jr((prefixes&P_SWAP) ? COND_GT : COND_LE); }
  
  // other instructions: inst.cc
  virtual int NOP(t_mem code);
  virtual int TRAP(t_mem code);
  virtual int THRD(t_mem code);
};


enum f8cpu_confs
  {
   f8cpu_sp_limit	= 0,
   f8cpu_nuof		= 1
  };

class cl_f8_cpu: public cl_hw
{
public:
  cl_f8_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return f8cpu_nuof; }
  virtual const char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of f8.src/f8cl.h */
