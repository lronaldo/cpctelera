/*
 * Simulator of microcontrollers (oisc.h)
 *
 * Copyright (C) 2024 Drotos Daniel
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


#ifndef OISCCL_HEADER
#define OISCCL_HEADER

#include "uccl.h"
#include "memcl.h"

#include "oisccl.h"


class cl_oisc;

class cl_op_pass: public cl_memory_operator
{
public:
  u16_t addr;
  class cl_oisc *uc;
public:
  cl_op_pass(class cl_memory_cell *acell, class cl_oisc *auc);
  virtual t_mem read(void);
  virtual t_mem write(t_mem val);
};


/*
 * OISC processor
 */

class cl_oisc: public cl_uc
{
public:
  const char *id_str;
  u16_t rA;
  class cl_cell16 cA;
public:
  cl_oisc(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void) { return id_str; }
  virtual void make_cpu_hw(void);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  virtual struct dis_entry *dis_tbl(void);
  virtual char *disassc(t_addr addr, chars *comment);
  virtual const char *dis_src(t_addr addr);
  virtual const char *dis_dst(t_addr addr);
  virtual chars dis_comment(t_addr src, t_addr dst);
  virtual int inst_length(t_addr addr);
  virtual void print_acc(class cl_console_base *con);
  virtual void print_regs(class cl_console_base *con);

  virtual void reset(void);
  virtual int exec_inst(void);

  virtual void init_alu(void);
  virtual u16_t read(u16_t addr);
  virtual u16_t write(u16_t addr, u16_t val);
};


#endif

/* End of oisc.src/oisccl.h */
