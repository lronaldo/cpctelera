/*
 * Simulator of microcontrollers (oisc.src/uartcl.h)
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

#ifndef UARTCL_HEADER
#define UARTCL_HEADER

//#include "fiocl.h"
//#include "stypes.h"
//#include "pobjcl.h"

#include "uccl.h"
#include "serial_hwcl.h"


enum uart_reg_idx
  {
   rcpb = 0, // RW 0xfffb
   tdr  = 1, // WO 0xfffc
   tstat= 2, // RO 0xfffd
   rdr  = 3, // RO 0xfffe
   rstat= 4, // RO 0xffff
  };

enum uart_cfg
  {
    uart_cfg_base	= serconf_nr+0,
  };

class cl_uart: public cl_serial_hw
{
 public:
  cl_uart(class cl_uc *auc, int aid, t_addr abase);
  virtual ~cl_uart(void);
  virtual int init(void);
  virtual unsigned int cfg_size(void) { return serconf_nr+1; }
  virtual const char *cfg_help(t_addr addr);
  virtual int dev_size(void) { return 5; }

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
  virtual void set_rdr(t_mem val);
  
  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of oisc.src/uartcl.h */
