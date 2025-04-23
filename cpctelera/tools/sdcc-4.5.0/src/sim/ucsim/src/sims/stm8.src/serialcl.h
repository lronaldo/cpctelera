/*
 * Simulator of microcontrollers (serialcl.h)
 *
 * Copyright (C) 2015 Drotos Daniel
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

#ifndef STM8_SERIALCL_HEADER
#define STM8_SERIALCL_HEADER

#include "fiocl.h"
#include "stypes.h"
#include "pobjcl.h"

#include "uccl.h"
#include "serial_hwcl.h"

#include "newcmdposixcl.h"


class cl_serial_listener;

class cl_serial: public cl_serial_hw
{
 protected:
  bool clk_enabled;
  bool clk_en_is_set;
  t_addr base;
  int type, txit, rxit;
  bool    sr_read;	// last op was read of SR
  bool    en;		// USART is enabled
 public:
  cl_serial(class cl_uc *auc,
	    t_addr abase,
	    int ttype, int atxit, int arxit);
  virtual ~cl_serial(void);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return 10; }
  virtual int dev_size(void) { return 12; }

  virtual void new_hw_added(class cl_hw *new_hw);
  virtual void added_to_uc(void);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual int tick(int cycles);
  virtual void start_send();
  virtual void restart_send();
  virtual void finish_send();
  virtual void received();
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he,
                      void *params);

  virtual void pick_div();
  virtual void pick_ctrl();
  virtual void show_writable(bool val);
  virtual void show_readable(bool val);
  virtual void show_tx_complete(bool val);
  virtual void show_idle(bool vol);
  virtual void set_dr(t_mem val);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of stm8.src/serialcl.h */
