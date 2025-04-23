/*
 * Simulator of microcontrollers (portcl.h)
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

#ifndef PORTCL_HEADER
#define PORTCL_HEADER

#include "port_hwcl.h"


enum port_cfg {
  port_on	= 0,
  port_pin	= 1,
  port_value	= 2,
  port_odr	= 3,
  port_nuof	= 4
};

enum port_widths {
  port_4bit	= 4,
  port_8bit	= 8
};

class cl_qport: public cl_hw
{
 public:
  class cl_address_space *port_as;
  t_addr addr;
  u8_t mask, width;
  class cl_memory_cell *pcell;
 public:
  cl_qport(class cl_uc *auc, int aid,
	   class cl_address_space *apas, t_addr aaddr,
	   enum port_widths awidth);
  cl_qport(class cl_uc *auc, int aid,
	   class cl_address_space *apas, t_addr aaddr,
	   enum port_widths awidth,
	   const char *aid_string);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return port_nuof; }
  virtual const char *cfg_help(t_addr addr);
  
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void print_info(class cl_console_base *con);
};


class cl_p2: public cl_qport
{
 public:
  cl_p2(class cl_uc *auc, int aid,
	class cl_address_space *apas, t_addr aaddr,
	enum port_widths awidth);
  virtual void set_low(u8_t val);
};


enum pext_cfg {
  pext_on	= 0,
  pext_pin4	= 4,
  pext_pin5	= 5,
  pext_pin6	= 6,
  pext_pin7	= 7,
  pext_odr4	= 8,
  pext_odr5	= 9,
  pext_odr6	= 10,
  pext_odr7	= 11,
  pext_dir4	= 12,
  pext_dir5	= 13,
  pext_dir6	= 14,
  pext_dir7	= 15,
  pext_nuof	= 16
};

class cl_pext: public cl_qport
{
public:
  class cl_memory_cell *pcell4, *pcell5, *pcell6, *pcell7;
  class cl_p2 *p2;
public:
  cl_pext(class cl_uc *auc, int aid,
	  class cl_address_space *apas, t_addr aaddr,
	  class cl_p2 *the_p2);
  virtual int init(void);
  virtual void reset(void);
  virtual unsigned int cfg_size(void) { return pext_nuof; }
  virtual const char *cfg_help(t_addr addr);
  
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual u8_t in(u8_t addr);
  virtual void out(u8_t addr, u8_t val);
  virtual void orl(u8_t addr, u8_t val);
  virtual void anl(u8_t addr, u8_t val);
  
  virtual void print_info(class cl_console_base *con);
};

  
#endif

/* End of i8048.src/portcl.h */
