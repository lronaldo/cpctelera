/*
 * Simulator of microcontrollers (simstm8.cc)
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

/* $Id: simstm8.cc 607 2017-01-19 11:11:44Z drdani $ */

// prj
#include "globals.h"

// local
#include "simstm8cl.h"
#include "stm8cl.h"


cl_simstm8::cl_simstm8(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simstm8::mk_controller(void)
{
  int i;
  char *typ= 0;
  class cl_optref type_option(this);

  type_option.init();
  type_option.use(cchars("cpu_type"));
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= cchars("STM8S");
  while ((cpus_stm8[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_stm8[i].type_str) != 0))
    i++;
  if (cpus_stm8[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_stm8[i].type)
    {
    case CPU_STM8S:
    case CPU_STM8L:
    case CPU_STM8L101:
      return(new cl_stm8(&cpus_stm8[i], this));
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of stm8.src/simstm8.cc */
