/*
 * Simulator of microcontrollers (sim51.cc)
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

//#include "ddconfig.h"

#include <stdio.h>
//#include <stdlib.h>
//#include <ctype.h>
//#include <errno.h>
#include <strings.h>
//#include "i_string.h"

#include "globals.h"
//#include "utils.h"
//#include "cmdutil.h"
//#include "charscl.h"

#include "sim51cl.h"
//#include "cmd51cl.h"
//#include "uc51cl.h"
//#include "uc52cl.h"
//#include "uc521cl.h"
#include "uc517cl.h"
//#include "uc51rcl.h"
//#include "uc89c51rcl.h"
#include "uc251cl.h"
#include "uc390cl.h"
#include "uc88xcl.h"
#include "uc320cl.h"
#include "uc380cl.h"
//#include "glob.h"


cl_sim51::cl_sim51(class cl_app *the_app):
  cl_sim(the_app)
{}


class cl_uc *
cl_sim51::mk_controller(void)
{
  int i;
  char *typ= 0;
  class cl_optref type_option(this);

  type_option.init();
  type_option.use(cchars("cpu_type"));
  i= 0;
  if ((typ= type_option.get_value(typ)) == 0)
    typ= cchars("C52");
  while ((cpus_51[i].type_str != NULL) &&
	 (strcasecmp(typ, cpus_51[i].type_str) != 0))
    i++;
  if (cpus_51[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }
  switch (cpus_51[i].type)
    {
    case CPU_51: case CPU_31:
      return(new cl_51core(&cpus_51[i], this));
    case CPU_52: case CPU_32:
      return(new cl_uc52(&cpus_51[i], this));
    case CPU_51R:
      return(new cl_uc51r(&cpus_51[i], this));
    case CPU_89C51R:
      return(new cl_uc89c51r(&cpus_51[i], this));
    case CPU_C521:
      return(new cl_uc521(&cpus_51[i], this));
    case CPU_517:
      return(new cl_uc517(&cpus_51[i], this));
    case CPU_XC88X:
      return(new cl_uc88x(&cpus_51[i], this));
    case CPU_F380:
      return(new cl_uc380(&cpus_51[i], this));
    case CPU_251:
      return(new cl_uc251(&cpus_51[i], this));
    case CPU_DS320:
      return(new cl_uc320(&cpus_51[i], this));
    case CPU_DS390: case CPU_DS390F:
      return(new cl_uc390(&cpus_51[i], this));
    default:
      fprintf(stderr, "Unknown processor type\n");
      return NULL;
    }
  return(NULL);
}


/* End of s51.src/sim51.cc */
