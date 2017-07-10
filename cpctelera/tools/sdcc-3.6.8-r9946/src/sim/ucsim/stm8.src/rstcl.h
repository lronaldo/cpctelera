/*
 * Simulator of microcontrollers (stm8.src/rstcl.h)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
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

/* $Id: rstcl.h 614 2017-01-27 08:39:01Z drdani $ */

#ifndef STM8_RSTCL_HEADER
#define STM8_RSTCL_HEADER

// sim
#include "hwcl.h"


class cl_rst: public cl_hw
{
 public:
  t_addr base;
  class cl_memory_cell *rst_sr;
  t_mem mask;
 public:
  cl_rst(class cl_uc *auc, t_addr abase, t_mem amask);
  //virtual ~cl_rst(void);
  virtual int init(void);

  //virtual void new_hw_added(class cl_hw *new_hw);
  //virtual void added_to_uc(void);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual int tick(int cycles);
};


#endif

/* End of stm8.src/rstcl.cc */
