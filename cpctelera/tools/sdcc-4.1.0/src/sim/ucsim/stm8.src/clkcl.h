/*
 * Simulator of microcontrollers (stm8.src/clkcl.h)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

#ifndef CLKCL_HEADER
#define CLKCL_HEADER

#include "hwcl.h"

class cl_clk_event: public cl_base
{
 public:
  enum hw_cath cath;
  int id;
 cl_clk_event(void): cl_base() {}
 cl_clk_event(enum hw_cath acath, int aid): cl_base()
    { cath= acath; id= aid; }
  virtual void set(enum hw_cath acath, int aid)
    { cath= acath; id= aid; }
};

class cl_clk: public cl_hw
{
 protected:
  t_addr base;
  class cl_memory_cell
    *ckdivr,
    *pckenr1,
    *pckenr2,
    *pckenr3;
 public:
  cl_clk(class cl_uc *auc);
  virtual int init(void);

  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual bool tim(int id) { return false; }
  virtual bool usart(int id) { return false; }
  
  virtual void reset(void) {}
};

class cl_clk_saf: public cl_clk
{
 public:
  cl_clk_saf(class cl_uc *auc);
  virtual int init(void);

  virtual void reset(void);

  virtual bool tim(int id);
  virtual bool usart(int id);
};

class cl_clk_all: public cl_clk
{
 public:
  cl_clk_all(class cl_uc *auc);
  virtual int init(void);

  virtual void reset(void);

  virtual bool tim(int id);
  virtual bool usart(int id);
};

class cl_clk_l101: public cl_clk
{
 public:
  cl_clk_l101(class cl_uc *auc);
  virtual int init(void);

  virtual void reset(void);

  virtual bool tim(int id);
  virtual bool usart(int id);
};


#endif

/* End of stm8.src/clkcl.h */
