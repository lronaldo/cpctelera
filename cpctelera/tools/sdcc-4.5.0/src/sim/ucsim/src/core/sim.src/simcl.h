/*
 * Simulator of microcontrollers (simcl.h)
 *
 * Copyright (C) 1999 Drotos Daniel
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

#ifndef SIM_SIMCL_HEADER
#define SIM_SIMCL_HEADER

#include <stdio.h>

// prj
#include "pobjcl.h"

// cmd
#include "newcmdcl.h"

// gui
#include "guicl.h"

// local
#include "uccl.h"
#include "argcl.h"

class cl_sim: public cl_base
{
public:
  class cl_app *app;
  int state; // See SIM_XXXX
  int argc; char **argv;

  //class cl_commander *cmd;
  class cl_uc *uc;
  class cl_gui *gui;
  class cl_hw *simif;
  
  double start_at, stop_at;
  unsigned long start_tick;
  unsigned long steps_done;
  unsigned long steps_todo; // use this if not 0
  uint32_t      exec_limit; // max nr of instr in run/step
  
public:
  cl_sim(class cl_app *the_app);
  virtual ~cl_sim(void);
  virtual int init(void);
  
  virtual class cl_uc *mk_controller(void);
  virtual void build_cmdset(class cl_cmdset *cmdset);

  virtual class cl_uc *get_uc(void) { return(uc); }

  virtual void start(class cl_console_base *con, unsigned long steps_to_do);
  virtual void emulation(class cl_console_base *con);
  virtual void stop(int reason);
  virtual void change_run(int reason= resSIMIF);
  virtual int step(void);
  virtual void set_limit(u32_t new_limit);
};


class cl_rgdb_listener: public cl_listen_console
{
protected:
  class cl_sim *sim;
public:
  cl_rgdb_listener(int serverport, class cl_app *the_app, cl_sim *asim);
  virtual class cl_console_base *mk_console(cl_f *fi, cl_f *fo);
};

class cl_rgdb: public cl_console
{
protected:
  class cl_sim *sim;
  bool thread_id_reported;
  bool ack;
public:
  cl_rgdb(cl_f *fi, cl_f *fo, class cl_app *the_app, class cl_sim *asim);
  virtual int init(void);
  virtual int read_line(void);
  virtual int proc_input(class cl_cmdset *cmdset);
  virtual int procq(chars l);
  virtual int procg(void);
  virtual int reply(const char *s);
  virtual int reply(chars s) { return reply(s.c_str()); }
  virtual void send(const char *s);
};


#endif

/* End of sim.src/simcl.h */
