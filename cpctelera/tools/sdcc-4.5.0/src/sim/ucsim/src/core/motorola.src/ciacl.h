/*
 * Simulator of microcontrollers (ciacl.h)
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

#ifndef CIACL_HEADER
#define CIACL_HEADER

//#include "fiocl.h"
//#include "stypes.h"
//#include "pobjcl.h"

#include "uccl.h"
#include "serial_hwcl.h"

//#include "newcmdposixcl.h"

enum acia_cfg
  {
    acia_cfg_base	= serconf_nr+0,
    acia_cfg_cr		= serconf_nr+1,
    acia_cfg_sr		= serconf_nr+2,
    acia_cfg_req	= serconf_nr+3,
  };

class cl_serial_listener;

class cl_cia: public cl_serial_hw
{
 protected:
  class cl_it_src *is_r, *is_t;
  class cl_memory_cell *r_cr;         // Copy of written CR value
  class cl_memory_cell *r_sr;         // Simulated SR value
 public:
  cl_cia(class cl_uc *auc, int aid, t_addr abase);
  virtual ~cl_cia(void);
  virtual int init(void);
  virtual void prepare_rebase(t_addr new_base);
  virtual unsigned int cfg_size(void) { return 11; }
  virtual const char *cfg_help(t_addr addr);
  virtual int dev_size(void) { return 2; }

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);

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
  virtual void set_sr_irq(void);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of motorola.src/ciacl.h */
