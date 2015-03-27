/*
 * Simulator of microcontrollers (simz80.cc)
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
#include "simz80cl.h"
#include "z80cl.h"
#include "r2kcl.h"
#include "lr35902cl.h"

cl_simz80::cl_simz80(class cl_app *the_app):
  cl_sim(the_app)
{}

class cl_uc *
cl_simz80::mk_controller(void)
{
  int i;
  const char *typ= NIL;
  class cl_optref type_option(this);

  type_option.init();
  type_option.use("cpu_type");
  i= 0;
  if ((typ= type_option.get_value(typ)) == NIL)
    typ= "Z80";

  while ((cpus_z80[i].type_str != NULL) &&
	 (strcmp(typ, cpus_z80[i].type_str) != 0))
    i++;
  if (cpus_z80[i].type_str == NULL)
    {
      fprintf(stderr, "Unknown processor type. "
	      "Use -H option to see known types.\n");
      return(NULL);
    }

  switch (cpus_z80[i].type)
    {
    case CPU_Z80:
    case CPU_Z180:
      return(new cl_z80(cpus_z80[i].type, cpus_z80[i].technology, this));
    // Add Rabbits, etc here.

    case CPU_R2K:
      return(new cl_r2k (cpus_z80[i].type, cpus_z80[i].technology, this));
      
    case CPU_R3KA:
      return(new cl_r3ka(cpus_z80[i].type, cpus_z80[i].technology, this));
      
    case CPU_LR35902:
      return(new cl_lr35902(cpus_z80[i].type, cpus_z80[i].technology, this));
    }

  return(NULL);
}


/* End of z80.src/simz80.cc */
