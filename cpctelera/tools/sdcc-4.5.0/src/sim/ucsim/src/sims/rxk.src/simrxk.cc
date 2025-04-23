/*
 * Simulator of microcontrollers (simrxk.cc)
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
#include "simrxkcl.h"
#include "r6kcl.h"


struct cpu_entry cpus_rxk[]=
  {
    {"R2K"	, CPU_R2K , 0, "Rabbit 2000", ""},
    {"2K"	, CPU_R2K , 0, "Rabbit 2000", ""},
    {"2"	, CPU_R2K , 0, "Rabbit 2000", ""},
    {"R3KA"	, CPU_R3KA, 0, "Rabbit 3000A", ""},
    {"3KA"	, CPU_R3KA, 0, "Rabbit 3000A", ""},
    {"3A"	, CPU_R3KA, 0, "Rabbit 3000A", ""},
    {"R3K"	, CPU_R3K , 0, "Rabbit 3000", ""},
    {"3K"	, CPU_R3K , 0, "Rabbit 3000", ""},
    {"3"	, CPU_R3K , 0, "Rabbit 3000", ""},
    {"R4K"	, CPU_R4K , 0, "Rabbit 4000", ""},
    {"4K"	, CPU_R4K , 0, "Rabbit 4000", ""},
    {"4"	, CPU_R4K , 0, "Rabbit 4000", ""},
    {"R5K"	, CPU_R5K , 0, "Rabbit 5000", ""},
    {"5K"	, CPU_R5K , 0, "Rabbit 5000", ""},
    {"5"	, CPU_R5K , 0, "Rabbit 5000", ""},
    {"R6K"	, CPU_R6K , 0, "Rabbit 6000", ""},
    {"6K"	, CPU_R6K , 0, "Rabbit 6000", ""},
    {"6"	, CPU_R6K , 0, "Rabbit 6000", ""},
    {NULL, CPU_NONE, 0, "", ""}
  };


cl_simrxk::cl_simrxk(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simrxk::mk_controller(void)
{
  int i;
  const char *typ= 0;
  class cl_optref type_option(this);
  
  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= "R2K";

  while ((cpus_rxk[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_rxk[i].type_str) != 0))
    i++;
  if (cpus_rxk[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_rxk[i].type)
    {
    case CPU_R2K:
      return(new cl_r2k(this));
    case CPU_R3K:
      return(new cl_r3k(this));
    case CPU_R3KA:
      return(new cl_r3ka(this));
    case CPU_R4K:
      return(new cl_r4k(this));
    case CPU_R5K:
      return(new cl_r5k(this));
    case CPU_R6K:
      return(new cl_r6k(this));
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return NULL;
}


/* End of rxk.src/simrxk.cc */
