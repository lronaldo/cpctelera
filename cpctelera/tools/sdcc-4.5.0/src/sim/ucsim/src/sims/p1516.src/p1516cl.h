/*
 * Simulator of microcontrollers (p1516cl.h)
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

#ifndef P1516CL_HEADER
#define P1516CL_HEADER

#include "uccl.h"
#include "memcl.h"


/*
 * Base of P1516 processor
 */

enum flags
  {
   S= 1,
   N= 1,
   C= 2,
   Z= 4,
   O= 8,
   V= 8,
   // p2223 flags
   P= 0x10, // 1:Pre 0:Post
   U= 0x20 // 1:Up  0:Down
  };


enum res2btn_t {
  r2b_none= 0,
  r2b_alarmed= 1,
  r2b_activated= 2
};


class cl_pc_write: public cl_memory_operator
{
protected:
  class cl_uc *uc;
public:
  cl_pc_write(class cl_memory_cell *acell, class cl_uc *the_uc);
  virtual t_mem write(t_mem val);
};


class cl_p1516: public cl_uc
{
public:
  u32_t F;
  u32_t R[16];
  cl_memory_cell *RC[16];
  cl_cell32 cF;
  cl_address_space *regs;
  class cl_porto *pa, *pb, *pc, *pd;
  class cl_porti *pi, *pj;
  class cl_brd_ctrl *bc;
  class cl_memory_chip *rom_chip;
  enum res2btn_t r2b_state;
public:
  //class cl_address_space *rom;
 public:
  cl_p1516(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);
  
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  virtual int clock_per_cycle(void) { return 1; }
  virtual double def_xtal(void) { return 25000000; }
  
  virtual struct dis_entry *dis_tbl(void);
  virtual char *disassc(t_addr addr, chars *comment);
  virtual void analyze_start(void);
  virtual void analyze(t_addr addr);
  virtual int inst_length(t_addr addr) { return 1; }
  virtual void print_regs(class cl_console_base *con);

  virtual bool cond(t_mem code);
  virtual t_mem inst_ad(t_mem ra, t_mem rb, u32_t c);
  virtual int inst_alu(t_mem code);
  virtual int exec_inst(void);

  virtual void btn_edge(int btn, bool press);
};

#define SET_C(v) ( cF.W( (F&~C) | ((v)?C:0) ))
#define SET_Z(v) ( cF.W( (F&~Z) | (((v)==0)?Z:0) ))
#define SET_S(v) ( cF.W( (F&~S) | ((v)?S:0) ))

#endif

/* End of p1516.src/p1516cl.h */
