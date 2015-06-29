/*
 * Simulator of microcontrollers (uc52.cc)
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

#include "ddconfig.h"

#include <stdio.h>

// local
#include "uc52cl.h"
#include "regs51.h"
#include "timer2cl.h"


/*
 * Making an 8052 CPU object
 */

cl_uc52::cl_uc52(int Itype, int Itech, class cl_sim *asim):
  cl_51core(Itype, Itech, asim)
{
}


void
cl_uc52::mk_hw_elements(void)
{
  class cl_hw *h;

  cl_51core::mk_hw_elements();
  hws->add(h= new cl_timer2(this, 2, "timer2", t2_default|t2_down));
  h->init();
}

void
cl_uc52::make_memories(void)
{
  class cl_address_space *as;

  rom= as= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);
  iram= as= new cl_address_space("iram", 0, 0x100, 8);
  as->init();
  address_spaces->add(as);
  sfr= as= new cl_address_space("sfr", 0x80, 0x80, 8);
  as->init();
  address_spaces->add(as);
  xram= as= new cl_address_space("xram", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("iram_chip", 0x100, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("iram"), chip, 0, 0xff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("xram_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("xram"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("sfr_chip", 0x80, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("sfr"), chip, 0x80, 0xff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  acc= sfr->get_cell(ACC);
  psw= sfr->get_cell(PSW);
}


void
cl_uc52::clear_sfr(void)
{
  cl_51core::clear_sfr();
  sfr->write(T2CON, 0);
  sfr->write(TH2, 0);
  sfr->write(TL2, 0);
  sfr->write(RCAP2L, 0);
  sfr->write(RCAP2H, 0);
}


/*
 * Calculating address of indirectly addressed IRAM cell
 *
 */

class cl_memory_cell *
cl_uc52::get_indirect(uchar addr, int *res)
{
  *res= resGO;
  return(iram->get_cell(addr));
}


/* End of s51.src/uc52.cc */
