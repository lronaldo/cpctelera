/*
 * Simulator of microcontrollers (s51.src/uc380.cc)
 *
 * Copyright (C) 2017,17 Drotos Daniel, Talker Bt.
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

//#include <string.h>

#include "portcl.h"

#include "uc380cl.h"


cl_uc380::cl_uc380(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc52(Itype, asim)
{
}

int
cl_uc380::init(void)
{
  int ret;
  ret= cl_uc52::init();

  return ret;
}


void
cl_uc380::mk_hw_elements(void)
{
  cl_uc52::mk_hw_elements();

  class cl_port *p4= new cl_port(this, 4, 0xc7);
  add_hw(p4);
  p4->init();

  class cl_port_ui *d= (class cl_port_ui *)get_hw(cchars("dport"), NULL);
  if (d)
    {
      class cl_port_data pd;
      pd.init();
      pd.cell_dir= NULL;
      
      pd.set_name("P4");
      pd.cell_p  = p4->cell_p;
      pd.cell_in = p4->cell_in;
      pd.keyset  = chars(keysets[4]);
      pd.basx    = 1;
      pd.basy    = 4+7;
      d->add_port(&pd, 4);
    }
}

void
cl_uc380::make_chips(void)
{
  cl_uc52::make_chips();
  sfr_chip->set_name("sfr_0_chip");
  sfr_f_chip= new cl_memory_chip("sfr_f_chip", 0x80, 8, 0);
  sfr_f_chip->init();
  memchips->add(sfr_f_chip);
}

void
cl_uc380::decode_sfr(void)
{
  cl_uc52::decode_sfr();
  t_addr sfrpage= 0xbf;
  class cl_banker *ad;

  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xe4, 0xe4);
  ad->init();
  ad->set_name("sfr_banker_0xe4");
  ad->add_bank(0  , sfr_chip  , 0xe4-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xe4-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xe4, 0xe4, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xc8, 0xc8);
  ad->init();
  ad->set_name("sfr_banker_0xc8");
  ad->add_bank(0  , sfr_chip  , 0xc8-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xc8-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xc8, 0xc8, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xca, 0xcf);
  ad->init();
  ad->set_name("sfr_banker_0xca-0xcf");
  ad->add_bank(0  , sfr_chip  , 0xca-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xca-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xca, 0xcf, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xc0, 0xc2);
  ad->init();
  ad->set_name("sfr_banker_0xc0-0xc2");
  ad->add_bank(0  , sfr_chip  , 0xc0-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xc0-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xc0, 0xc2, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xb9, 0xb9);
  ad->init();
  ad->set_name("sfr_banker_0xb9");
  ad->add_bank(0  , sfr_chip  , 0xb9-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xb9-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xb9, 0xb9, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0xbc, 0xbc);
  ad->init();
  ad->set_name("sfr_banker_0xbc");
  ad->add_bank(0  , sfr_chip  , 0xbc-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0xbc-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0xbc, 0xbc, NULL);
  ad->activate(NULL);
  
  ad= new cl_banker(sfr, sfrpage, 0x0f,
		    sfr, 0x91, 0x95);
  ad->init();
  ad->set_name("sfr_banker_0x91-0x95");
  ad->add_bank(0  , sfr_chip  , 0x91-0x80);
  ad->add_bank(0xf, sfr_f_chip, 0x91-0x80);
  sfr->decoders->add(ad);
  sfr->undecode_area(ad, 0x91, 0x95, NULL);
  ad->activate(NULL);
  
  ad->activate(NULL);
}

void
cl_uc380::clear_sfr(void)
{
  cl_uc52::clear_sfr();

  sfr->write(/*P4*/0xc7, 0xff);
}


/* End of s51.src/uc380.cc */
