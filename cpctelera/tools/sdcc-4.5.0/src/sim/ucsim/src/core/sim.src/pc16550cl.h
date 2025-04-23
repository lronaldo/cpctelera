/*
 * Simulator of microcontrollers (pc16550cl.h)
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

#ifndef PC16550_HEADER
#define PC16550_HEADER

#include "serial_hwcl.h"

enum pc16550_reg_idx
  {
    rrbr	= 0, // dlab=0 read
    rthr	= 0, // dlab=0 write
    rier	= 1, // dlab=0
    riir	= 2, // read
    rfcr	= 2, // write
    rlcr	= 3,
    rmcr	= 4,
    rlsr	= 5,
    rmsr	= 6,
    rscr	= 7,
    rdll	= 0, // dlab=1
    rdlm	= 1, // dlab=1

    tdr		= 0,
    rdr		= 0,
  };

enum pc16550_cfg
  {
    pc16550_cfg_base	= serconf_nr+0,
  };

class cl_pc16550: public cl_serial_hw
{
 protected:
  class cl_cell8 cfcr, cdll, cdlm;
  u8_t fcr, dll, dlm;
  bool dlab;
 public:
  cl_pc16550(class cl_uc *auc, int aid);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return serconf_nr+1; }
  virtual const char *cfg_help(t_addr addr);
  virtual int dev_size(void) { return 8; }
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
  virtual void pick_div();
  virtual void pick_ctrl();
  virtual void show_writable(bool val);
  virtual void show_readable(bool val);
  virtual void show_tx_complete(bool val);
  virtual void show_idle(bool vol);
  virtual void set_rdr(t_mem val);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of sim.src/pc16550cl.h */
