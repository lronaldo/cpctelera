/*
 * Simulator of microcontrollers (st7cl.h)
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

#ifndef ST7CL_HEADER
#define ST7CL_HEADER

#include "uccl.h"

#include "regsst7.h"


/*
 * Base type of Z80 microcontrollers
 */

class cl_st7: public cl_uc
{
public:
  class cl_memory *ram;
  class cl_memory *rom;
  class cl_address_space *regs8, *regs16;
  struct t_regs regs;
  t_addr sp_limit;
public:
  cl_st7(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);

  //virtual t_addr get_mem_size(enum mem_class type);
  virtual void make_cpu_hw(void);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual struct dis_entry *dis_tbl(void);
  virtual int inst_length(t_addr addr);
  virtual int inst_branch(t_addr addr);
  virtual int longest_inst(void);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);

  virtual int exec_inst(void);

  virtual void stack_check_overflow(class cl_stack_op *op);

  virtual const char *get_disasm_info(t_addr addr,
                                      int *ret_len,
                                      int *ret_branch,
                                      int *immed_offset,
                                      struct dis_entry **dentry);
  virtual bool is_call(t_addr addr);
  virtual t_mem get_1(t_addr addr);
  virtual t_mem get_2(t_addr addr);
  virtual t_mem get_3(t_addr addr);
  
  virtual void reset(void);
#include "instcl.h"
};


enum st7cpu_confs
  {
   st7cpu_sp_limit	= 0,
   st7cpu_nuof		= 1
  };

class cl_st7_cpu: public cl_hw
{
public:
  cl_st7_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual int cfg_size(void) { return st7cpu_nuof; }
  virtual const char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};

#endif

/* End of st7.src/st7cl.h */
