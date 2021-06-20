/*
 * Simulator of microcontrollers (sim.src/hwcl.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

/* Abstract hw element. It can be a timer, serial line or whatever */

#ifndef SIM_HWCL_HEADER
#define SIM_HWCL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "guiobjcl.h"

// cmd.src
#include "newcmdcl.h"
#include "newcmdposixcl.h"

// local
#include "memcl.h"
#include "uccl.h"


class cl_hw;

class cl_hw_io: public cl_console
{
 protected:
  class cl_hw *hw;
 public:
  cl_hw_io(class cl_hw *ihw);
  virtual int init(void);
  
  virtual int proc_input(class cl_cmdset *cmdset);
  virtual bool prevent_quit(void) { return get_fin() && get_fin()->tty; }
  virtual void print_prompt(void) {}

  virtual void convert2console(void);
  virtual void pass2hw(class cl_hw *new_hw);
};


class cl_hw: public cl_guiobj
{
 public:
  int flags;
  class cl_uc *uc;
  enum hw_cath cathegory;
  int id;
  const char *id_string;
  bool on;
 protected:
  class cl_list *partners;
  class cl_address_space *cfg;
  class cl_hw_io *io;
  int cache_run;
  unsigned int cache_time;
 public:
  cl_hw(class cl_uc *auc, enum hw_cath cath, int aid, const char *aid_string);
  virtual ~cl_hw(void);

  virtual int init(void);
  virtual int cfg_size(void) { return 1; }
  
  virtual void new_hw_adding(class cl_hw *new_hw);
  virtual void new_hw_added(class cl_hw *new_hw);
  virtual void added_to_uc(void) {}
  virtual class cl_hw *make_partner(enum hw_cath cath, int id);

  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);
  virtual bool conf(class cl_memory_cell *cell, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);
  virtual void cfg_set(t_addr addr, t_mem val);
  virtual void cfg_write(t_addr addr, t_mem val);
  virtual t_mem cfg_get(t_addr addr);
  virtual t_mem cfg_read(t_addr addr);
  
  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual class cl_memory_cell *register_cell(class cl_address_space *mem,
					      t_addr addr);
  virtual class cl_memory_cell *register_cell(class cl_memory_cell *cell);
  virtual void unregister_cell(class cl_memory_cell *cell);

  virtual int tick(int cycles);
  virtual void reset(void) {}
  virtual void happen(class cl_hw * /*where*/, enum hw_event /*he*/,
                      void * /*params*/) {}
  virtual void inform_partners(enum hw_event he, void *params);
  virtual void touch(void);
  
  virtual void make_io(void);
  virtual void new_io(class cl_f *f_in, class cl_f *f_out);
  virtual cl_hw_io *get_io(void);
  virtual bool proc_input(void);
  virtual bool handle_input(int c);
  virtual void refresh_display(bool force);
  virtual void draw_display(void);
  virtual cl_hw *next_displayer(void);
  
  virtual void print_info(class cl_console_base *con);
};

class cl_hws: public cl_list
{
 public:
 cl_hws(void): cl_list(2, 2, cchars("hws")) {}
  virtual t_index add(void *item);
  virtual cl_hw *next_displayer(class cl_hw *hw);
};


class cl_partner_hw: public cl_base
{
 protected:
  class cl_uc *uc;
  enum hw_cath cathegory;
  int id;
  class cl_hw *partner;
 public:
  cl_partner_hw(class cl_uc *auc, enum hw_cath cath, int aid);

  virtual class cl_hw *get_partner(void);
  virtual void refresh(void);
  virtual void refresh(class cl_hw *new_hw);

  virtual void happen(class cl_hw *where, enum hw_event he, void *params);
};


#endif

/* End of hwcl.h */
