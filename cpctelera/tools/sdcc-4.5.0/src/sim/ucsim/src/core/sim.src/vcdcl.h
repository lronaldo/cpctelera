/*
 * Simulator of microcontrollers (vcdcl.h)
 *
 * Copyright (C) 2017 Drotos Daniel
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

#ifndef VCDCL_HEADER
#define VCDCL_HEADER

#include "hwcl.h"


class cl_vcd_var;

class cl_vcd: public cl_hw
{
 private:
  char var_id;
  char *filename;
  FILE *fd;
  class cl_vcd_var *vars;
  double starttime, timescale, event, pausetime;
  int state;
  bool started, paused, dobreak;
  chars modul;
  char word[64];

 public:
  cl_vcd(class cl_uc *auc, int aid, chars aid_string);
  int init(void);

  inline bool is_running(void) const { return started && !paused; }

  inline char get_next_var_id(void) { return var_id++; }

  virtual bool set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual void set_help(class cl_console_base *con);

  virtual int tick(int cycles);

  virtual void report(class cl_vcd_var *var, t_mem v);
  virtual void print_info(class cl_console_base *con);

 private:
  inline void reset_next_var_id(void) { var_id = 33; }
  void add_var(class cl_console_base *con, class cl_memory_cell *cell, int bitnr_low, int bitnr_high)
    {
      add_var(con, 0, cell, bitnr_low, bitnr_high);
    }
  void add_var(class cl_console_base *con, char id, class cl_memory_cell *cell, int bitnr_low, int bitnr_high);
  void add_var(class cl_console_base *con, class cl_memory *m, t_addr a, int bitnr_low, int bitnr_high);
  void del_var(class cl_console_base *con, class cl_memory_cell *cell, int bitnr_low, int bitnr_high);
  void del_var(class cl_console_base *con, class cl_memory *m, t_addr a, int bitnr_low, int bitnr_high);

  FILE *open_vcd(class cl_console_base *con);
  void close_vcd(void);

  bool read_word(unsigned int i);
  void clear_vars(void);
  bool parse_header(cl_console_base *con);
};


#endif

/* End of sim.src/vcdcl.h */
