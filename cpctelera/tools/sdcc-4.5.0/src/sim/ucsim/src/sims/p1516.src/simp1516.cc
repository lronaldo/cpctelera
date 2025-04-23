/*
 * Simulator of microcontrollers (simp1516.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
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

// local
#include "simp1516cl.h"
#include "p1516cl.h"
#include "p2223cl.h"
#include "glob.h"


cl_simp1516::cl_simp1516(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simp1516::mk_controller(void)
{
  int i;
  const char *typ= 0;
  class cl_optref type_option(this);
  class cl_p1516 *uc;

  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= "P2223";
  while ((cpus_p1516[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_p1516[i].type_str) != 0))
    i++;
  if (cpus_p1516[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_p1516[i].type)
    {
    case CPU_P1516:
      uc= new cl_p1516(this);
      return uc;
    case CPU_P2223:
      uc= new cl_p2223(this);
      return uc;
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of p1516.src/simp1516.cc */
