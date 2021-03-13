/*
 * Simulator of microcontrollers (p1516cl.h)
 *
 * Copyright (C) 2020,20 Drotos Daniel, Talker Bt.
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
   C= 2,
   Z= 4,
   O= 8
  };

class cl_p1516: public cl_uc
{
public:
  u8_t F;
  u32_t R[16];
  cl_memory_cell *RC[16];
  cl_address_space *regs;
  class cl_porto *pa, *pb, *pc, *pd;
  class cl_porti *pi, *pj;
public:
  class cl_address_space *rom;
 public:
  cl_p1516(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void set_PC(t_addr addr);

  virtual void mk_hw_elements(void);
  virtual void make_memories(void);
  virtual int clock_per_cycle(void) { return 4; }
  
  virtual struct dis_entry *dis_tbl(void);
  virtual char *disass(t_addr addr, const char *sep);
  virtual void print_regs(class cl_console_base *con);

  virtual t_mem inst_ad(t_mem ra, t_mem rb, u32_t c);
  virtual int inst_alu(t_mem code);
  virtual int exec_inst(void);
};

#define SET_C(v) ( F= (F&~C) | ((v)?C:0) )
#define SET_Z(v) ( F= (F&~Z) | ((v==0)?Z:0) )
#define SET_S(v) ( F= (F&~S) | ((v)?S:0) )

#endif

/* End of p1516.src/p1516.cc */
