/*
 * Simulator of microcontrollers (z80cl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 *
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#ifndef R2KCL_HEADER
#define R2KCL_HEADER

#include "z80cl.h"

  /* TODO: maybe this should become an enum */
#define IOI  1  // next instruction uses internal I/O space
#define IOE  2  // next instruction uses external I/O space

#define MMIDR 0x10   /* MMU Instruction/Data Register */
#define SADR  0xC0   /* Serial A Data Register in IOI (internal I/O space) */


class cl_r2k;

class rabbit_mmu {
public:
  cl_r2k     *parent_p;
  
  /* Note: DEF_REGPAIR is defined in regsz80.h */
  
  u8_t  xpc;
  u8_t  dataseg;
  u8_t  stackseg;
  u8_t  segsize;
  
  u8_t  io_flag; /* pseudo register for ioi/ioe prefixes */
  
  u8_t  mmidr;  /* MMU Instruction/Data Register __at 0x10 */
  
  rabbit_mmu( cl_r2k *parent_ip ):parent_p(parent_ip),
    xpc(0), dataseg(0), stackseg(0), segsize(0xFF)
    { }
  
  u32_t/*TYPE_UDWORD*/  logical_addr_to_phys( u16_t logical_addr );
};


class cl_r2k: public cl_z80
{
public:
  // from cl_z80:
  //class cl_memory *ram;
  //class cl_memory *rom;
  //struct t_regs regs;  
  
  rabbit_mmu   mmu;

  u16_t ins_start;  /* PC value for start of the current instruction */
  u8_t ip;  /* interrupt priority register */
  
  /* iir, eir registers are not full supported */
  u8_t iir;
  u8_t eir;
  
  /* see Rabbit Family of Microprocessors: Instruction Reference Manual */
  /*   019-0098 * 090409-L */
  

public:
  cl_r2k(struct cpu_entry *Itype, class cl_sim *asim);
  virtual int init(void);
  virtual char *id_string(void);
  
  //virtual t_addr get_mem_size(enum mem_class type);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  
  virtual struct dis_entry *dis_tbl(void);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);
  virtual int exec_code(t_mem code);
  
  virtual const char *get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset);
  
  
  virtual void store1( u16_t addr, t_mem val );
  virtual void store2( u16_t addr, u16_t val );
  
  virtual u8_t  get1( u16_t addr );
  virtual u16_t  get2( u16_t addr );
  
  virtual t_mem       fetch1( void );
  virtual u16_t  fetch2( void );
  
  virtual t_mem fetch(void);
  virtual bool fetch(t_mem *code) {
    return cl_uc::fetch(code);
  }
  
  // see #include "instcl.h" for Z80 versions
  /* instruction function that are add / modified from the Z80 versions */
  virtual int inst_rst(t_mem code);
  
  virtual int inst_add_sp_d(t_mem code);
  virtual int inst_altd(t_mem code);
  
  virtual int inst_bool   (t_mem code);
  virtual int inst_r2k_ld (t_mem code);
  virtual int inst_r2k_and(t_mem code);
  virtual int inst_r2k_or (t_mem code);
  virtual int inst_r2k_ex (t_mem code);
  
  virtual int inst_ljp(t_mem code);
  virtual int inst_lcall(t_mem code);
  virtual int inst_mul(t_mem code);
  
  virtual int inst_rl_de(t_mem code);
  virtual int inst_rr_de(t_mem code);
  virtual int inst_rr_hl(t_mem code);

  virtual int inst_xd(t_mem prefix);
  
  //virtual int inst_ed(void);
  virtual int inst_ed_(t_mem code);
  
};

class cl_r3ka: public cl_r2k {
 public:
  
  u8_t  SU;
  
  cl_r3ka(struct cpu_entry *Itype, class cl_sim *asim);
  virtual char *id_string(void);
  
  virtual int exec_code(t_mem code);
  
  virtual int inst_ed_(t_mem code);
};

#endif /* R2KCL_HEADER */
