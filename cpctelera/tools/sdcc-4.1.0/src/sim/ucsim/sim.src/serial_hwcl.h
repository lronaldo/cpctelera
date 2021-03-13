/*
 * Simulator of microcontrollers (sim.src/serial_hwcl.h)
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

#ifndef SERIAL_HWCL_HEADER
#define SERIAL_HWCL_HEADER

#include "newcmdposixcl.h"

#include "hwcl.h"


enum serial_cfg {
  serconf_on	   	= 0,
  serconf_check_often	= 1,
  serconf_escape	= 2,
  serconf_common	= 3,
  serconf_received	= 4,
  serconf_flowctrl	= 5,
  serconf_able_receive	= 6,
  serconf_nr		= 7
};


class cl_serial_io: public cl_hw_io
{
 public:
 cl_serial_io(class cl_hw *ihw):
  cl_hw_io(ihw)
  {}
  //virtual bool prevent_quit(void) { return true; }
  virtual bool input_avail(void);
};

class cl_serial_hw: public cl_hw
{
 protected:
  class cl_optref *serial_in_file_option;
  class cl_optref *serial_out_file_option;
  class cl_optref *serial_port_option;
  class cl_optref *serial_iport_option;
  class cl_optref *serial_oport_option;
  class cl_serial_listener *listener;
  //class cl_hw_io *io;
  char input;
  bool input_avail;
  char menu;
 public:
  cl_serial_hw(class cl_uc *auc, int aid, chars aid_string);
  virtual ~cl_serial_hw(void);
  virtual int init(void);
  virtual int cfg_size(void) { return serconf_nr; }
  virtual const char *cfg_help(t_addr addr);
  
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual bool proc_input(void);
  virtual void refresh_display(bool force) {}
  virtual void draw_state_time(bool force) {}
  virtual void draw_display(void) {}

  virtual void reset(void);
};

enum ser_listener_for
  {
   sl_io,
   sl_i,
   sl_o
  };

class cl_serial_listener: public cl_listen_console
{
protected:
  enum ser_listener_for sl_for;
 public:
  class cl_serial_hw *serial_hw;
  cl_serial_listener(int serverport, class cl_app *the_app,
		     class cl_serial_hw *the_serial,
		     enum ser_listener_for slf);
  virtual int init(void);
  virtual int proc_input(class cl_cmdset *cmdset);
  virtual bool prevent_quit(void) { return false; }
};


#endif

/* End of sim.src/serial_hwcl.h */
