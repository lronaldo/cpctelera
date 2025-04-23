/*
 * Simulator of microcontrollers (ucsim.cc)
 *
 * Copyright (C) 1997 Drotos Daniel
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

// prj
#include "globals.h"
#include "appcl.h"

#include "charscl.h"

class cl_general_uc: public cl_uc
{
public:
  u8_t r;
public:
  cl_general_uc(cl_sim *s): cl_uc(s) {}
  virtual void make_memories(void);
  virtual int exec_inst()
  {
    t_mem code;
    if (fetch(&code))
      return resBREAKPOINT;
    tick(1);
    r= rom->read(PC);
    vc.rd++;
    rom->write(PC, r+1);
    vc.wr++;
    r= 34;
    return resGO;
  }
};

void
cl_general_uc::make_memories(void)
{
  class cl_address_space *as;
  class cl_memory_chip *ch;
  class cl_address_decoder *ad;

  rom= as= new cl_address_space("nas", 0, 0x100000, 8);
  as->init();
  address_spaces->add(as);

  ch= new cl_chip8("chip", 0x100000, 8/*, 0*/);
  ch->init();
  memchips->add(ch);
  
  ad= new cl_address_decoder(as, ch, 0, 0x100000-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}

class cl_general_sim: public cl_sim
{
public:
  cl_general_sim(cl_app *a): cl_sim(a) {}
  virtual class cl_uc *mk_controller(void)
  {
    class cl_uc *uc= new cl_general_uc(this);
    return uc;
  }
};

int
main(int argc, char *argv[])
{
  int ret;
  class cl_sim *sim;
  volatile double fd, id;
  unsigned int i= 0;
  
  app_start_at= dnow();
  {
    fd= 0;
    while (++i < 100000000) fd+= 1.0;
    fd= dnow()-app_start_at;
    fd= 100.0/fd;
  }
  application= new cl_app();
  application->set_name("ucsim");
  application->init(argc, argv);
  sim= new cl_general_sim(application);
  if (sim->init())
    sim->state|= SIM_QUIT;
  application->set_simulator(sim);
  /*
  {
    id= dnow();
    i= 0;
    while (++i < 1000000) sim->step();
    id= dnow() - id;
    id= (1.0*1000)/id;
    fprintf(stderr, "\n%f MFlop ", fd);
    fprintf(stderr, "%f kips\n", id);
  }
  */
  ret= application->run();
  application->done();
  return(ret);
}

/* End of apps/ucsim.cc */
