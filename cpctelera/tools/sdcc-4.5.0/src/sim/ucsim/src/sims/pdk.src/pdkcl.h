/*
 * Simulator of microcontrollers (pdkcl.h)
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

#ifndef PDKCL_HEADER
#define PDKCL_HEADER

#include "stypes.h"

#include "uccl.h"
#include "itsrccl.h"

#include "regspdk.h"


const t_addr io_size = 128;

#define BIT_Z	0x01  // zero status, 1=zero, 0=nonzero
#define BIT_C	0x02  // carry status(addition and subtraction)
#define BIT_AC  0x04  // sign, 1=negative, 0=positive (or zero)
#define BIT_OV  0x08  // signed overflow, 1=overflow, 0=no overflow
#define BIT_ALL	(BIT_Z | BIT_C | BIT_AC | BIT_OV)  // all bits

#define BITPOS_Z 0    // 1
#define BITPOS_C 1    // 2H
#define BITPOS_AC 2    // 4H
#define BITPOS_OV 3    // 8H

enum flag {
  flag_z,
  flag_c,
  flag_ac,
  flag_ov,
};

#define fC  ((rF&BIT_C)>>BITPOS_C)
#define fZ  ((rF&BIT_Z)>>BITPOS_Z)
#define fAC ((rF&BIT_AC)>>BITPOS_AC)
#define fA  ((rF&BIT_AC)>>BITPOS_AC)
#define fOV ((rF&BIT_OV)>>BITPOS_OV)
#define fO  ((rF&BIT_OV)>>BITPOS_OV)

#define SETZ(boolval) (cF->W((boolval)?(rF|BIT_Z):(rF&~BIT_Z)))
#define SETC(boolval) (cF->W((boolval)?(rF|BIT_C):(rF&~BIT_C)))

#define CODE_MASK(op, m) ((code & ~(m)) == (op))

#define regs8 sfr

enum pdk_mode_t {
  pm_run	= 0,
  pm_pd		= 1, // powerdown (stopsys)
  pm_ps		= 2, // powersave (stopexe)
};


struct fppinfo_t {
  const char *part;
  int fpp_num;
  int ram_size;
  int rom_size;
  const char *arm_sym;
  const char *ice_sym;
};

enum regaccess_t {
  NO= 0,
  RO= 1,
  WO= 2,
  RW= 3,
  WR= RW,
  DI= 4
};

struct reginfo_t {
  const char *part;
  const char *reg;
  enum regaccess_t access;
  int address;
};


class cl_xtal_writer: public cl_memory_operator
{
public:
  class cl_pdk *puc;
public:
  cl_xtal_writer(class cl_pdk *apuc, class cl_memory_cell *acell):
    cl_memory_operator(acell) { puc= apuc; }
  virtual t_mem write(t_mem val);
};


/*
 * Base type of STM8 microcontrollers
 */

class cl_pdk;

class cl_fpp: public cl_uc
{
public:
  class cl_pdk *puc;
  int id;
  class cl_address_space *ram;
  class cl_address_space *sfr;
  u8_t rA, rF, rSP, rTMP;
  class cl_cell8 cA;
  class cl_memory_cell *cF, *cSP;
public:
  cl_fpp(int aid, class cl_pdk *the_puc, class cl_sim *asim);
  cl_fpp(int aid, class cl_pdk *the_puc, struct cpu_entry *IType, class cl_sim *asim);
  virtual int init(void);
  virtual void act(void);
  
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);

  virtual double def_xtal(void) { return 8000000; }
  
  virtual struct dis_entry *dis_tbl(void)=0;
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual int m_mask(void)= 0;
  virtual int io_mask(void)= 0;
  virtual int rom_mask(void)= 0;
  //virtual char *disass(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment);
  virtual void print_regs(class cl_console_base *con);

  virtual int execute(unsigned int code) { return resINV; }
  virtual int exec_inst(void);
  virtual void push(u16_t word);
  virtual void pushlh(u8_t low, u8_t high);
  virtual u8_t pop(void) { cSP->W(rSP-1); vc.rd++; return ram->read(rSP); }
  virtual void stack_check_overflow(void);
  virtual void stack_check_overflow(t_addr sp_before);

  virtual const char *get_disasm_info(t_addr addr,
                                      int *ret_len,
                                      int *ret_branch,
                                      int *immed_offset,
                                      struct dis_entry **dentry);
  virtual bool is_call(t_addr addr);

  virtual void reset(void);

  // originaly in instcl.h
  int get_mem(unsigned int addr) { vc.rd++; return ram->read(addr); }
  u8_t rd8(unsigned int addr) { vc.rd++; return ram->read(addr); }
  u16_t rd16(u16_t addr) { vc.rd+= 2; return ram->read(addr+1)*256+ram->read(addr); }
  void wr8(u16_t addr, u8_t val) { vc.wr++; ram->write(addr, val); }
  void wr16(u16_t addr, u16_t val)
  { vc.wr+=2; ram->write(addr, val); ram->write(addr+1, val>>8); }
  u8_t add_to(u8_t initial, int value, bool carry = false);
  u8_t sub_to(u8_t initial, int value, bool carry = false);
  u8_t get_io(t_addr addr) { return sfr->read(addr); }
  int store_io(t_addr addr, int value) { sfr->write(addr, value); return resGO; }
  void store_flag(flag n, int value);
  /*
  int execute_pdk13(unsigned int code);
  int execute_pdk14(unsigned int code);
  int execute_pdk15(unsigned int code);
  */
};


class cl_pdk_cell: public cl_cell8
{
public:
  class cl_pdk *puc;
public:
  cl_pdk_cell(class cl_pdk *the_puc):
    cl_cell8()
  {
    puc= the_puc;
  }
};
  
class cl_act_cell: public cl_pdk_cell
{
public:
  cl_act_cell(class cl_pdk *the_puc): cl_pdk_cell(the_puc) {}
  virtual t_mem write(t_mem val);
};
  
class cl_nuof_cell: public cl_pdk_cell
{
public:
  cl_nuof_cell(class cl_pdk *the_puc): cl_pdk_cell(the_puc) {}
  virtual t_mem write(t_mem val);
};


class cl_fppen_op: public cl_memory_operator
{
public:
  class cl_pdk *puc;
public:
  cl_fppen_op(class cl_pdk *the_puc, class cl_memory_cell *acell);
  virtual t_mem write(t_mem val);
};


class cl_mulrh_op: public cl_memory_operator
{
public:
  class cl_pdk *puc;
public:
  cl_mulrh_op(class cl_pdk *the_puc, class cl_memory_cell *acell);
  virtual t_mem read(void);
};


class cl_pdk: public cl_uc
{
public:
  class cl_fpp *fpps[8];
  class cl_address_space *ram;
  class cl_address_space *regs8;
  u8_t rFPPEN, act, nuof_fpp, rMULRH;
  bool single;
  class cl_memory_cell *cFPPEN, *cact, *cnuof_fpp;
  class cl_osc *osc;
  class cl_t16 *t16;
  class cl_wdt *wdt;
  enum pdk_mode_t mode;
public:
  cl_pdk(struct cpu_entry *IType, class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void make_memories(void);
  virtual void mk_hw_elements(void);
  virtual class cl_fpp *mk_fpp(int id);
  virtual double def_xtal(void) { return 8000000; }
  virtual void reset(void);
  
  virtual u8_t set_fppen(u8_t val);
  virtual u8_t set_act(u8_t val);
  virtual u8_t set_nuof(u8_t val);
  virtual t_addr get_pc(int id);
  virtual void set_pc(int id, t_addr new_pc);
  
  virtual int exec_inst(void);

  virtual char *disassc(t_addr addr, chars *comment);
  virtual void print_regs(class cl_console_base *con);
};


/*
class cl_pdk_cpu: public cl_hw
{
 protected:
  class cl_memory_cell *regs[11];
 public:
  cl_pdk_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return 2; }

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};
*/


#endif

/* End of pdk.src/pdkcl.h */
