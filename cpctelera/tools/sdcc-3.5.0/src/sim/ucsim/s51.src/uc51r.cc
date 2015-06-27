/*
 * Simulator of microcontrollers (uc51r.cc)
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
#include "uc51rcl.h"
#include "regs51.h"
#include "types51.h"
#include "wdtcl.h"


/*
 * Making an 8051r CPU object
 */

cl_uc51r::cl_uc51r(int Itype, int Itech, class cl_sim *asim):
  cl_uc52(Itype, Itech, asim)
{
  /*  int i;
  for (i= 0; i < ERAM_SIZE; i++)
  ERAM[i]= 0;*/
  clock_out= 0;
}


void
cl_uc51r::mk_hw_elements(void)
{
  class cl_hw *h;

  cl_uc52::mk_hw_elements();
  hws->add(h= new cl_wdt(this, 0x3fff));
  h->init();
  hws->add(h= new cl_uc51r_dummy_hw(this));
  h->init();
}


void
cl_uc51r::make_memories(void)
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
  chip= new cl_memory_chip("eram_chip", 0x100, 8);
  chip->init();
  memchips->add(chip);

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


/*
 * Resetting of the microcontroller
 *
 * Original method is extended with handling of WDT.
 */

void
cl_uc51r::reset(void)
{
  cl_uc52::reset();
  sfr->write(SADDR, 0);
  sfr->write(SADEN, 0);
  sfr->write(AUXR, 0);
}

void
cl_uc51r::clear_sfr(void)
{
  cl_uc52::clear_sfr();
  sfr->write(SADDR, 0);
  sfr->write(SADEN, 0);
  sfr->write(AUXR, 0);
  sfr->write(IPH, 0);
}


void
cl_uc51r::received(int c)
{
  t_mem br= sfr->get(SADDR) | sfr->get(SADEN);
  int scon= sfr->get(SCON);

  if ((0 < scon >> 6) &&
      (scon & bmSM2))
    {
      if (/* Check for individual address */
	  ((sfr->get(SADDR) & sfr->get(SADEN)) == (c & sfr->get(SADEN)))
	  ||
	  /* Check for broadcast address */
	  (br == (br & c)))
	sfr->set_bit1(SCON, bmRI);
      return;
    }
  sfr->set_bit1(SCON, bmRI);
}


/*
 */

cl_uc51r_dummy_hw::cl_uc51r_dummy_hw(class cl_uc *auc):
  cl_hw(auc, HW_DUMMY, 0, "_51r_dummy")
{}

int
cl_uc51r_dummy_hw::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  if (!sfr)
    {
      fprintf(stderr, "No SFR to register %s[%d] into\n", id_string, id);
    }
  //use_cell(sfr, PSW, &cell_psw, wtd_restore);
  register_cell(sfr, AUXR, &cell_auxr, wtd_restore);
  return(0);
}

void
cl_uc51r_dummy_hw::write(class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_auxr)
    {
      class cl_address_space *xram= uc->address_space(MEM_XRAM_ID);
      class cl_memory_chip *eram=
	dynamic_cast<class cl_memory_chip *>(uc->memory("eram_chip"));
      class cl_address_decoder *d;
      if (eram &&
	  xram)
	{
	  if (*val & bmEXTRAM)
	    d= new cl_address_decoder(xram, uc->memory("xram_chip"), 0,0xff,0);
	  else
	    d= new cl_address_decoder(xram, eram, 0, 0xff, 0);
	  d->init();
	  xram->decoders->add(d);
	  d->activate(0);
	}
    }
  /*else if (cell == cell_pcon)
    {
      printf("PCON write 0x%x (PC=0x%x)\n", *val, uc->PC);
      uc->sim->stop(0);
      }*/
}

/*void
cl_uc51r_dummy_hw::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3o= ep->pins & ep->prev_value;
      t_mem p3n= ep->new_pins & ep->new_value;
      if ((p3o & bm_INT0) &&
	  !(p3n & bm_INT0))
	uc51->p3_int0_edge++;
      if ((p3o & bm_INT1) &&
	  !(p3n & bm_INT1))
	uc51->p3_int1_edge++;
    }
}*/


/* End of s51.src/uc51r.cc */
