/*
 * Simulator of microcontrollers (i8085cl.h)
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


#ifndef I8085CL_HEADER
#define I8085CL_HEADER

#include "i8080cl.h"

/* 8085 only */
#define RIM	instruction_20
#define SIM	instruction_30

/* Undocumented */
#define ARHL	instruction_10
#define RDEL	instruction_18
#define DSUB	instruction_08
#define JNX5	instruction_dd
#define JX5	instruction_fd
#define RSTV	instruction_cb
#define LDHI	instruction_28
#define LDSI	instruction_38
#define LHLX	instruction_ed
#define SHLX	instruction_d9


/*
 * Special handling of flags
 */

class cl_flag85_op: public cl_memory_operator
{
public:
  cl_flag85_op(class cl_memory_cell *acell): cl_memory_operator(acell) {}
  virtual t_mem write(t_mem val);
};


/*
 * i8085 processor
 */

class cl_i8085: public cl_i8080
{
 public:
  cl_i8085(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_cpu_hw(void);
  virtual void make_memories(void);
  virtual class cl_memory_operator *make_flag_op(void);

  virtual int clock_per_cycle(void) { return 1; }
  //virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment=NULL);

  virtual void print_regs(class cl_console_base *con);

  virtual u16_t *tick_tab(void) { return tick_tab_8085; }

  virtual int inx(class cl_memory_cell &op);
  virtual int dcx(class cl_memory_cell &op);

  virtual int RIM(t_mem code);
  virtual int SIM(t_mem code);

  virtual int ARHL(t_mem code);
  virtual int RDEL(t_mem code);
  virtual int DSUB(t_mem code);
  virtual int JNX5(t_mem code);
  virtual int JX5 (t_mem code);
  virtual int RSTV(t_mem code);
  virtual int LDHI(t_mem code);
  virtual int LDSI(t_mem code);
  virtual int LHLX(t_mem code);
  virtual int SHLX(t_mem code);
};


enum i8085cpu_confs
  {
   i8085cpu_sp_limit	= 0,
   i8085cpu_nuof	= 1
  };

class cl_i8085_cpu: public cl_hw
{
public:
  cl_i8085_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return i8085cpu_nuof; }
  virtual const char *cfg_help(t_addr addr);

  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
};


#endif

/* End of i8085.src/i8085cl.h */
