/*
 * Simulator of microcontrollers (uc89c51rcl.h)
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

#ifndef UC89C51RCL_HEADER
#define UC89C51RCL_HEADER

#include "ddconfig.h"

#include "uc51rcl.h"


class cl_uc89c51r: public cl_uc51r
{
public:
  //int t0_overflows;
  uchar dpl0, dph0;
  uchar dpl1, dph1;
  uchar dps;

public:
  cl_uc89c51r(int Itype, int Itech, class cl_sim *asim);
  virtual void mk_hw_elements(void);
  virtual void make_memories(void);

  virtual void  reset(void);
  virtual void  pre_inst(void);
  virtual void  post_inst(void);
  virtual int   it_priority(uchar ie_mask);
  virtual void  print_regs(class cl_console_base *con);

  //virtual void  do_extra_hw(int cycles);
  //virtual int   t0_overflow(void);
  /*virtual int   do_pca(int cycles);
  virtual int   do_pca_counter(int cycles);
  virtual int   do_pca_module(int nr);*/
};

class cl_89c51r_dummy_hw: public cl_hw
{
protected:
  class cl_memory_cell *auxr1;
public:
  cl_89c51r_dummy_hw(class cl_uc *auc);
  virtual int init(void);

  virtual void write(class cl_memory_cell *cell, t_mem *val);
};

#endif

/* End of s51.src/uc89c51rcl.h */
