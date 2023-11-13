/*
 * Simulator of microcontrollers (stm8cl.h)
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

#ifndef STM8CL_HEADER
#define STM8CL_HEADER

#include "uccl.h"
#include "itsrccl.h"

#include "regsstm8.h"
#include "itccl.h"


/*
 * Base type of STM8 microcontrollers
 */

class cl_stm8: public cl_uc
{
public:
  class cl_address_space *ram;
  class cl_address_space *regs8;
  class cl_address_space *regs16;
  class cl_memory_chip
    *ram_chip, // max 6k
    *eeprom_chip, // max 2k
    *option_chip, // 128 bytes
    *io_chip, // 2k
    *boot_chip, // 2k
    *cpu_chip, // 256 bytes
    *flash_chip; // max 128k
  //class cl_memory *rom;
  struct t_regs regs;
  class cl_itc *itc;
  class cl_it_src *trap_src;
  class cl_flash *flash_ctrl;
  t_addr sp_limit;
public:
  cl_stm8(struct cpu_entry *IType, class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);

  //virtual t_addr get_mem_size(enum mem_class type);
  virtual void mk_port(t_addr base, chars n);
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

  virtual const char *get_disasm_info(t_addr addr,
                                      int *ret_len,
                                      int *ret_branch,
                                      int *immed_offset,
                                      struct dis_entry **dentry);
  virtual bool is_call(t_addr addr);

  virtual void reset(void);

  virtual int  do_interrupt(void);
  virtual int  priority_of(uchar nuof_it);
  virtual int  priority_main(void);
  virtual int  accept_it(class it_level *il);
  virtual bool it_enabled(void);

  virtual void stack_check_overflow(class cl_stack_op *op);

#include "instcl.h"
};


enum stm8_cpu_cfg
  {
   cpuconf_sp_limit	= 0,
  };

class cl_stm8_cpu: public cl_hw
{
 protected:
  class cl_memory_cell *regs[11];
 public:
  cl_stm8_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual int cfg_size(void) { return 2; }

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual const char *cfg_help(t_addr addr);
};


#endif

/* End of stm8.src/stm8cl.h */
