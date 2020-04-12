/*
 * Simulator of microcontrollers (appcl.h)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
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

#ifndef APPCL_HEADER
#define APPCL_HEADER

#include "ddconfig.h"

// prj
#include "pobjcl.h"
#include "optioncl.h"

// sim.src
#include "argcl.h"
#include "simcl.h"


extern bool jaj;

/* Options */
/*
enum opt_types {
  OPT_GENERAL	= 0x0001,
  OPT_SIM	= 0x0002,
  OPT_UC	= 0x0004,
  OPT_PRG_OPT	= (OPT_GENERAL|OPT_SIM|OPT_UC),
  OPT_51	= 0x0010,
  OPT_AVR	= 0x0020,
  OPT_Z80	= 0x0040,
  OPT_HC08	= 0x0080,
  OPT_XA	= 0x0100,
  OPT_TARGET	= (OPT_51|OPT_AVR|OPT_Z80|OPT_HC08|OPT_XA)
};
*/
/*class cl_option: public cl_base
{
public:
  int type;	// See OPT_XXX
  char short_name;
  char *long_name;
  class cl_ustrings *values;

public:
  cl_option(int atype, char sn, char *ln);
  virtual ~cl_option(void);

  virtual int add_value(char *value);
  virtual char *get_value(int index);
};

class cl_options: public cl_list
{
public:
  cl_options(void);
};*/


/* Application */

class cl_app: public cl_base
{
protected:
  class cl_commander_base *commander;
public:
  class cl_sim *sim;
  class cl_ustrings *in_files;
  class cl_options *options;
  int going;
  long expr_result;
  chars startup_command;
  
public:
  cl_app(void);
  virtual ~cl_app(void);

public:
  virtual int init(int argc , char *argv[]);
  virtual int run(void);
  virtual void done(void);

protected:
  virtual int proc_arguments(int argc, char *argv[]);

public:
  class cl_sim *get_sim(void) { return(sim); }
  class cl_uc *get_uc(void);
  class cl_commander_base *get_commander(void) { return(commander); }
  //virtual class cl_cmd *get_cmd(class cl_cmdline *cmdline);
  virtual long eval(chars expr);
  virtual void exec(chars line);
  
public: // messages to broadcast
  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

public:
  virtual void set_simulator(class cl_sim *simulator);
  virtual void remove_simulator(void);

protected:
  virtual void build_cmdset(class cl_cmdset *cs);
  virtual void mk_options(void);

public: // output functions
  virtual int dd_printf(const char *format, ...);
  virtual int dd_cprintf(const char *color_name, const char *format, ...);
  virtual int debug(const char *format, ...);

public:
  virtual void set_option_s(chars opt_name, chars new_value);
};


#endif

/* End of appcl.h */
