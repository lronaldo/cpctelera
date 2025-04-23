/*
 * Simulator of microcontrollers (p2223cl.h)
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

#ifndef P2223CL_HEADER
#define P2223CL_HEADER

#include "p1516cl.h"


#define CLP2 cl_p2223

#define setZSw(v)  { F&= ~(Z|S); if (!(v)) F|=Z; if ((v)&0x80000000) F|=S; cF.W(F); }
#define setZSnw(v) { F&= ~(Z|S); if (!(v)) F|=Z; if ((v)&0x80000000) F|=S; }

class cl_p2223;

class cl_f_write: public cl_memory_operator
{
public:
  class cl_p2223 *uc;
public:
  cl_f_write(class cl_memory_cell *acell,
	     class cl_p2223 *the_uc):
    cl_memory_operator(acell)
  {
    uc= the_uc;
  }
  virtual t_mem write(t_mem val);
};


class cl_sfr_op: public cl_memory_operator
{
public:
  class cl_p2223 *uc;
  t_addr addr;
public:
  cl_sfr_op(class cl_memory_cell *acell,
	    class cl_p2223 *the_uc,
	    t_addr a):
    cl_memory_operator(acell)
  {
    uc= the_uc;
    addr= a;
  }
  virtual t_mem write(t_mem val);
  virtual t_mem read(void);
};


class cl_p2223: public cl_p1516
{
public:
  //bool dbg_reg;
  chars id_chars;
  class cl_address_space *sfr;
public:
  cl_p2223(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual struct dis_entry *dis_tbl(void);
  virtual char *disassc(t_addr addr, chars *comment);
  //virtual void analyze_start(void);
  virtual void analyze(t_addr addr);
  virtual t_addr next_inst(t_addr addr);
  virtual void print_regs(class cl_console_base *con);

  virtual bool cond(t_mem code);
  virtual int inst_alu_1op(t_mem code);
  virtual int inst_alu(t_mem code);
  virtual int inst_mem(t_mem code);
  virtual int inst_ext(t_mem code);
  virtual int inst_uncond(t_mem code);
  virtual int inst_call(t_mem code);
  virtual int exec_inst(void);
};


#endif

/* End of p1516.src/p2223cl.h */
