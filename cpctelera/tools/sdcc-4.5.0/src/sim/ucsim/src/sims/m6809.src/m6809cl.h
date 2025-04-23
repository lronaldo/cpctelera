/*
 * Simulator of microcontrollers (m6809cl.h)
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

#ifndef M6809CL_HEADER
#define M6809CL_HEADER

#include "uccl.h"
#include "memcl.h"


/*
 * Base of M6809 processor
 */

struct reg_t {
  u16_t U;
  u16_t S;
  u16_t X;
  u16_t Y;
  union {
    u16_t rD;
    struct {
#ifdef WORDS_BIGENDIAN
      u8_t rA;
      u8_t rB;
#else
      u8_t rB;
      u8_t rA;
#endif      
    } a8;
  } acc;
  u8_t DP;
  u8_t CC;
};

#define A  (reg.acc.a8.rA)
#define B  (reg.acc.a8.rB)
#define D  (reg.acc.rD)

enum flags
  {
   flagC= 1,
   flagV= 2,
   flagO= 2,
   flagZ= 4,
   flagN= 8,
   flagS= 8,
   flagI= 16,
   flagH= 32,
   flagF= 64,
   flagE= 128
  };

class cl_m6809_src_base;

class cl_m6809: public cl_uc
{
public:
  class cl_address_space *regs8;
  class cl_address_space *regs16;
  struct reg_t reg;
  bool en_nmi;
  bool cwai;
public:
  class cl_address_space *rom;
  class cl_it_src *src_irq, *src_firq, *src_nmi;
protected:
  u8_t *reg8_ptr[8];
  u16_t *reg16_ptr[8];
public:
  cl_m6809(class cl_sim *asim);
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
  virtual void analyze_start(void);
  virtual void analyze(t_addr addr);
  virtual void disass_indexed(t_addr *addr, chars *work, int siz);
  virtual void disass_immediate(t_addr *addr, chars *work, int siz);
  virtual char *disass(t_addr addr);
  virtual void print_regs(class cl_console_base *con);
  virtual int indexed_length(t_addr addr);
  virtual int inst_length(t_addr addr);
  virtual int longest_inst(void) { return 5; }

  virtual int index2ea(u8_t idx, t_addr *res_ea);
  virtual void push_regs(bool do_cc);
  virtual void pull_regs(bool do_cc);
  
  virtual int inst_add8 (t_mem code,  u8_t *acc,  u8_t op, bool c, bool store, bool invert_c);
  virtual int inst_add16(t_mem code, u16_t *acc, u16_t op, bool c, bool store, bool invert_c, bool is_sub);
  virtual int inst_bool (t_mem code, char bop, u8_t *acc, u8_t op, bool store);
  virtual int inst_ld8  (t_mem code,  u8_t *acc,  u8_t op);
  virtual int inst_ld16 (t_mem code, u16_t *acc, u16_t op);
  virtual int inst_st8  (t_mem code,  u8_t  src, t_addr ea);
  virtual int inst_st16 (t_mem code, u16_t  src, t_addr ea);
  
  virtual int inst_alu(t_mem code);

  virtual int inst_10(t_mem code);
  virtual int inst_branch(t_mem code, bool l);
  virtual int inst_30(t_mem code);

  virtual int inst_neg(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_com(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_lsr(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_ror(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_asr(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_asl(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_rol(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_dec(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_inc(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_tst(t_mem code, u8_t *acc, t_addr ea, u8_t op8);
  virtual int inst_clr(t_mem code, u8_t *acc, t_addr ea, u8_t op8);

  virtual int inst_low(t_mem code);
  
  virtual int inst_page1(t_mem code);

  virtual int inst_page2(t_mem code);
    
  virtual int exec_inst(void);
  virtual int priority_of(uchar nuof_it) { return nuof_it; }
  virtual int accept_it(class it_level *il);
  virtual bool it_enabled(void) { return true; }
};

#define SET_C(v) ( (reg.CC)= ((reg.CC)&~flagC) | ((v)?flagC:0) )
#define SET_Z(v) ( (reg.CC)= ((reg.CC)&~flagZ) | ((v==0)?flagZ:0) )
#define SET_S(v) ( (reg.CC)= ((reg.CC)&~flagS) | ((v)?flagS:0) )
#define SET_O(v) ( (reg.CC)= ((reg.CC)&~flagV) | ((v)?flagV:0) )
#define SET_H(v) ( (reg.CC)= ((reg.CC)&~flagH) | ((v)?flagH:0) )

  
#endif

/* End of m6809.src/m6809.cc */
