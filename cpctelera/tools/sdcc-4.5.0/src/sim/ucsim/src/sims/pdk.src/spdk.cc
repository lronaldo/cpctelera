/*
 * Simulator of microcontrollers (spdk.cc)
 *
 * Copyright (C) 2016 Drotos Daniel
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

#include <stdlib.h>
  
// prj
#include "globals.h"
#include "utils.h"

// sim.src
//#include "appcl.h"

// local
#include "glob.h"
#include "simpdkcl.h"


struct cpu_entry *
gen_cpus()
{
  int ti, i, nr;
  struct cpu_entry *t;
  nr= 0;
  for (i= 0; cpus_pdk[i].type_str; i++)
    nr++;
  for (i= 0; fppinfo[i].part; i++)
    nr++;
  t= (struct cpu_entry *)malloc(sizeof(struct cpu_entry) * (nr+2));
  ti= 0;
  for (i= 0; cpus_pdk[i].type_str; i++)
    {
      t[ti].type_str= cpus_pdk[i].type_str;
      t[ti].type= cpus_pdk[i].type;
      t[ti].subtype= cpus_pdk[i].subtype;
      t[ti].type_help= cpus_pdk[i].type_help;
      t[ti].sub_help= cpus_pdk[i].sub_help;
      ti++;
    }
  for (i= 0; fppinfo[i].part; i++)
    {
      t[ti].type= CPU_NONE;
      if (fppinfo[i].part[0] == 0) continue;
      if (strcmp(fppinfo[i].arm_sym, "84B") == 0) t[ti].type= CPU_PDK13;
      if (strcmp(fppinfo[i].arm_sym, "13" ) == 0) t[ti].type= CPU_PDK13;
      if (strcmp(fppinfo[i].arm_sym, "85A") == 0) t[ti].type= CPU_PDK14;
      if (strcmp(fppinfo[i].arm_sym, "14" ) == 0) t[ti].type= CPU_PDK14;
      if (strcmp(fppinfo[i].arm_sym, "86B") == 0) t[ti].type= CPU_PDK15;
      if (strcmp(fppinfo[i].arm_sym, "15" ) == 0) t[ti].type= CPU_PDK15;
      if (strcmp(fppinfo[i].arm_sym, "83A") == 0) t[ti].type= CPU_PDK16;
      if (strcmp(fppinfo[i].arm_sym, "16" ) == 0) t[ti].type= CPU_PDK16;
      if (t[ti].type != CPU_NONE)
	{
	  t[ti].type_str= fppinfo[i].part;
	  t[ti].subtype= 0;
	  switch (t[ti].type)
	    {
	    case CPU_PDK13: t[ti].type_help= "PDK13"; break;
	    case CPU_PDK14: t[ti].type_help= "PDK14"; break;
	    case CPU_PDK15: t[ti].type_help= "PDK15"; break;
	    case CPU_PDK16: t[ti].type_help= "PDK16"; break;
	    default: t[ti].type_help= ""; break;
	    }
	  t[ti].sub_help= "";
	  ti++;
	}
    }
  t[ti].type_str= NULL;
  t[ti].type= CPU_NONE;
  t[ti].subtype= 0;
  t[ti].type_help= NULL;
  t[ti].sub_help= NULL;
  return t;
}


int
main(int argc, char *argv[])
{
  class cl_sim *sim;

  app_start_at= dnow();
  cpus= gen_cpus();
  application= new cl_app();
  application->set_name("spdk");
  application->init(argc, argv);
  sim= new cl_simpdk(application);
  if (sim->init())
    sim->state|= SIM_QUIT;
  application->set_simulator(sim);
  application->run();
  application->done();
  delete application;
  return(0);
}


/* End of pdk.src/spdk.cc */
