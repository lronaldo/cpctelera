/*
 * Simulator of microcontrollers (cmd.src/cmdconfcl.h)
 *
 * Copyright (C) 2001,01 Drotos Daniel, Talker Bt.
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

#ifndef CMD_CMD_CONF_HEADER
#define CMD_CMD_CONF_HEADER

#include "newcmdcl.h"


// CONF
//COMMAND_ON(uc,cl_conf_cmd);
class cl_conf_cmd: public cl_cmd
{
 public:
 cl_conf_cmd(const char *aname,
	     int  can_rep,
	     const char *short_help,
	     const char *long_help):
  cl_cmd(operate_on_uc, aname, can_rep, short_help, long_help) {}
  virtual int do_work(class cl_uc *uc ,
		      class cl_cmdline *cmdline, class cl_console_base *con);
};

// CONF OBJECTS
COMMAND_ON(app,cl_conf_objects_cmd);

// JAJ
COMMAND_ON(app,cl_jaj_cmd);


#endif

/* End of cmd.src/cmd_confcl.h */
