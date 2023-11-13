/*
 * Simulator of microcontrollers (savr.cc)
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

//#include <stdio.h>

  // prj
#include "globals.h"

// sim.src
//#include "appcl.h"

// local
#include "simavrcl.h"


int
main(int argc, char *argv[])
{
  class cl_sim *sim;
  
  application= new cl_app();
  application->set_name("savr");
  application->init(argc, argv);
  sim= new cl_simavr(application);
  if (sim->init())
    sim->state|= SIM_QUIT;
  application->set_simulator(sim);
  //sim->main();
  application->run();
  application->done();
  delete application;
  return(0);
}


/* End of avr.src/savr.cc */
