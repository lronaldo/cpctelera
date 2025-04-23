/*
 * Simulator of microcontrollers (simi8048.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include "simi8048cl.h"
#include "i8041cl.h"
#include "glob.h"


cl_simi8048::cl_simi8048(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simi8048::mk_controller(void)
{
  int i;
  const char *typ= 0;
  class cl_optref type_option(this);
  class cl_i8020 *uc;

  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= "I8050";
  while ((cpus_8048[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_8048[i].type_str) != 0))
    i++;
  if (cpus_8048[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_8048[i].type)
    {
    case CPU_I8021:
      uc= new cl_i8021(this);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8022:
      uc= new cl_i8022(this);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8048:
      uc= new cl_i8048(this);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8049:
      uc= new cl_i8048(this, 2048, 128);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8050:
      uc= new cl_i8048(this, 4096, 256);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8035:
      uc= new cl_i8048(this, 4096, 64);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8039:
      uc= new cl_i8048(this, 4096, 128);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8040:
      uc= new cl_i8048(this, 4096, 256);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8041:
      uc= new cl_i8041(this, 4096, 256);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    case CPU_I8041A:
      uc= new cl_i8041A(this, 4096, 256);
      uc->set_id(cpus_8048[i].type_help);
      uc->type= &cpus_8048[i];
      return uc;
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of i8048.src/simi8048.cc */
