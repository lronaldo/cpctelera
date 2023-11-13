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

//#include "ddconfig.h"

//#include <stdio.h>

// local
#include "uc51rcl.h"
#include "regs51.h"
//#include "types51.h"
#include "wdtcl.h"


/*
 * Making an 8051r CPU object
 */

cl_uc51r::cl_uc51r(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc52(Itype, asim)
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
  add_hw(h= new cl_wdt(this, 0x3fff));
  h->init();
}


void
cl_uc51r::make_memories(void)
{
  cl_uc52::make_memories();
}

void
cl_uc51r::make_chips(void)
{
  cl_uc52::make_chips();
  
  eram_chip= new cl_memory_chip("eram_chip", 0x100, 8);
  eram_chip->init();
  memchips->add(eram_chip);
}

void
cl_uc51r::decode_xram(void)
{
  class cl_address_decoder *ad;
  class cl_banker *b;

  ad= new cl_address_decoder(xram, xram_chip, 0x100, 0xffff, 0x100);
  ad->init();
  xram->decoders->add(ad);
  ad->activate(0);
  
  b= new cl_banker(sfr, AUXR, 0x02, //0,
		   xram, 0, 0xff);
  b->init();
  xram->decoders->add(b);
  b->add_bank(0, eram_chip, 0);
  b->add_bank(1, xram_chip, 0);
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


/* End of s51.src/uc51r.cc */
