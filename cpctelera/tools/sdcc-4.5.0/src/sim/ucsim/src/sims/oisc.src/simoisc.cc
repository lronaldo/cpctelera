/*
 * Simulator of microcontrollers (simoisc.cc)
 *
 * Copyright (C) 2024 Drotos Daniel
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

#include "simoisccl.h"
#include "oisccl.h"
#include "urisccl.h"
#include "misc16cl.h"
#include "emcl.h"
#include "glob.h"


cl_simoisc::cl_simoisc(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simoisc::mk_controller(void)
{
  int i;
  const char *typ= 0;
  class cl_optref type_option(this);
  class cl_oisc *uc;

  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= "EM";
  while ((cpus_oisc[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_oisc[i].type_str) != 0))
    i++;
  if (cpus_oisc[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_oisc[i].type)
    {
    case CPU_OISC:
      uc= new cl_oisc(this);
      uc->type= &cpus_oisc[i];
      return uc;
    case CPU_URISC:
      uc= new cl_urisc(this);
      uc->type= &cpus_oisc[i];
      return uc;
    case CPU_MISC16:
      uc= new cl_misc16(this);
      uc->type= &cpus_oisc[i];
      return uc;
    case CPU_EM:
      uc= new cl_em(this);
      uc->type= &cpus_oisc[i];
      return uc;
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of oisc.src/simoisc.cc */
