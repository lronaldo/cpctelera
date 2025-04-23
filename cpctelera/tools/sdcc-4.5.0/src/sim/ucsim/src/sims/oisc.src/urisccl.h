/*
 * Simulator of microcontrollers (urisccl.h)
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

#ifndef URISCCL_HEADER
#define URISCCL_HEADER

#include "oisccl.h"


enum {
  flagO= 0x08,
  flagN= 0x04,
  flagS= flagN,
  flagC= 0x02,
  flagZ= 0x01
};

#define ACC rA
#define A   rA
#define F   rF


class cl_urisc: public cl_oisc
{
public:
  u8_t rF;
  class cl_cell8 cF;
public:
  cl_urisc(class cl_sim *asim);
  virtual void print_regs(class cl_console_base *con);
  virtual const char *dis_src(t_addr addr);
  virtual const char *dis_dst(t_addr addr);
  virtual chars dis_comment(t_addr src, t_addr dst);

  virtual u16_t add(u16_t a, u16_t b, u16_t c);
  virtual void init_alu(void);
  virtual u16_t read(u16_t addr);
  virtual u16_t write(u16_t addr, u16_t val);
};


#endif

/* End of oisc.src/urisccl.h */
