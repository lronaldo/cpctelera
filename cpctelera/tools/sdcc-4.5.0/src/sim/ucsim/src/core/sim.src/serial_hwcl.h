/*
 * Simulator of microcontrollers (serial_hwcl.h)
 *
 * Copyright (C) 2016 Drotos Daniel
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
  serconf_nl		= 7,
  serconf_nr		= 8
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
  friend class cl_serial_listener;
protected:
  class cl_optref *serial_in_file_option;
  class cl_optref *serial_out_file_option;
  class cl_optref *serial_port_option;
  class cl_optref *serial_iport_option;
  class cl_optref *serial_oport_option;
  class cl_optref *serial_ifirst_option;
  class cl_optref *serial_raw_option;
  class cl_serial_listener *listener_io, *listener_i, *listener_o;
  //class cl_hw_io *io;
  char input;
  bool input_avail;
  char menu;
  bool is_raw;
  bool sending_nl;
  int skip_nl;
  //u32_t nl_value;
  int nl_send_idx;
  // common state variables
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
  // clock divider
  int cpb;
  int mcnt;
  // mapping into memory space
  class cl_memory_cell **regs;
  class cl_address_space *as;
  t_addr base;
  chars var_names;
public:
  cl_serial_hw(class cl_uc *auc, int aid, chars aid_string);
  virtual ~cl_serial_hw(void);
  virtual int init(void);
  virtual void map(class cl_address_space *new_as, t_addr new_base);
  virtual unsigned int cfg_size(void) { return serconf_nr; }
  virtual const char *cfg_help(t_addr addr);
  virtual int dev_size(void) { return 1; }
  
  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual bool is_nl(char c) { return (c=='\n') || (c=='\r'); }
  virtual char opposite_nl(char c) { return (c=='\n')?'\r':((c=='\r')?'\n':c); }
  virtual u8_t get_input(void);

  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual void new_i(class cl_f *f_in);
  virtual void new_o(class cl_f *f_out);
  virtual void set_raw(void);
  virtual void del_listener_i(void);
  virtual void del_listener_o(void);
  virtual bool proc_input(void);
  virtual void show_menu(void);
  virtual bool proc_not_in_menu(cl_f *fin, cl_f *fout);
  virtual bool proc_in_menu(cl_f *fin, cl_f *fout);
  virtual void refresh_display(bool force) {}
  virtual void draw_state_time(bool force) {}
  virtual void draw_display(void) {}
  
  virtual void reset(void);
  virtual bool prediv_bitcnt(int cycles);
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
