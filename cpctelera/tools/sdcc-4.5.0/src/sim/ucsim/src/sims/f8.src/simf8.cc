/*
 * Simulator of microcontrollers (simf8.cc)
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

#include "simf8cl.h"
#include "f8cl.h"
#include "glob.h"


cl_simf8::cl_simf8(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simf8::mk_controller(void)
{
  int i;
  const char *typ= 0;
  class cl_optref type_option(this);
  class cl_f8 *uc;

  /* Replace 1s to flagO in p table */
  for (int ii= 0; ii<256; ii++)
    if (!ptab[ii]) // odd=1, even=0
      ptab[ii]= flagO;
    else
      ptab[ii]= 0;
  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= "F8";
  while ((cpus_f8[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_f8[i].type_str) != 0))
    i++;
  if (cpus_f8[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_f8[i].type)
    {
    case CPU_F8:
      uc= new cl_f8(this);
      return uc;
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of f8.src/simf8.cc */
