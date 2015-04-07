/*
 * Simulator of microcontrollers (simhc08.cc)
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

#include "globals.h"

// local
#include "simhc08cl.h"
#include "hc08cl.h"


cl_simhc08::cl_simhc08(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simhc08::mk_controller(void)
{
  int i;
  const char *typ= NIL;
  class cl_optref type_option(this);

  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == NIL)
    typ= "HC08";

  while ((cpus_hc08[i].type_str != NULL) &&
	 (strcmp(typ, cpus_hc08[i].type_str) != 0))
    i++;
  if (cpus_hc08[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }

  switch (cpus_hc08[i].type)
    {
    case CPU_HC08:
    case CPU_HCS08:
      return(new cl_hc08(cpus_hc08[i].type, cpus_hc08[i].technology, this));
    }

  return(NULL);
}


/* End of hc08.src/simhc08.cc */
