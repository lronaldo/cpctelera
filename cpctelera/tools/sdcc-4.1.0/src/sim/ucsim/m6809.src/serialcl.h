/*
 * Simulator of microcontrollers (@@F@@)
 *
 * Copyright (C) @@S@@,@@Y@@ Drotos Daniel, Talker Bt.
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

#ifndef M6809_SERIALCL_HEADER
#define M6809_SERIALCL_HEADER

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

class cl_serial: public cl_serial_hw
{
 protected:
  t_addr base;
  class cl_memory_cell *regs[2];
  int div;
  int mcnt;
  class cl_m6809_src_base *is_r, *is_t;
  class cl_memory_cell *r_cr;         // Copy of written CR value
  class cl_memory_cell *r_sr;         // Simulated SR value
  u8_t  s_in;         // Serial channel input reg
  u8_t  s_out;        // Serial channel output reg
  u8_t  s_txd;	      // TX data register
  bool  s_sending;    // Transmitter is working (s_out is not empty)
  bool  s_receiving;  // Receiver is working (s_in is shifting)
  bool  s_tx_written; // TX data reg has been written
  int   s_rec_bit;    // Bit counter of receiver
  int   s_tr_bit;     // Bit counter of transmitter
  uchar bits;         // Nr of bits to send/receive
  bool  ren;          // Receiving is enabled
  bool  ten;          // Transmitter is enabled
 public:
  cl_serial(class cl_uc *auc, int aid, t_addr abase);
  virtual ~cl_serial(void);
  virtual int init(void);
  virtual int cfg_size(void) { return 11; }
  virtual const char *cfg_help(t_addr addr);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);

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

/* End of m6809.src/serialcl.h */
