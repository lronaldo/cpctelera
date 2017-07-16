/*
 * Simulator of microcontrollers (pca.cc)
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

#include <ctype.h>

// sim.src
#include "itsrccl.h"

// local
#include "pcacl.h"
#include "regs51.h"
#include "types51.h"


cl_pca::cl_pca(class cl_uc *auc, int aid):
  cl_hw(auc, HW_PCA, aid, "pca")
{
  t0_overflows= ECI_edge= 0;
  int i;
  for (i= 0; i < 5; cex_pos[i]= cex_neg[i]= false, i++) ;
}

int
cl_pca::init(void)
{
  cl_hw::init();
  sfr= uc->address_space(MEM_SFR_ID);
  //t_addr CCAPL[5]= {CCAPL[0], CCAPL[1], CCAPL[2], CCAPL[3], CCAPL[4]};
  //t_addr CCAPH[5]= {CCAPH[0], CCAPH[1], CCAPH[2], CCAPH[3], CCAPH[4]};
  //t_addr CCAPM[5]= {CCAPM[0], CCAPM[1], CCAPM[2], CCAPM[3], CCAPM[4]};
  t_addr CCAPL[5]= {CCAP0L, CCAP1L, CCAP2L, CCAP3L, CCAP4L};
  t_addr CCAPH[5]= {CCAP0H, CCAP1H, CCAP2H, CCAP3H, CCAP4H};
  t_addr CCAPM[5]= {CCAPM0, CCAPM1, CCAPM2, CCAPM3, CCAPM4};
  int i;

  if (!sfr)
    {
      fprintf(stderr, "No SFR to register PCA[%d] into\n", id);
    }
  cell_cmod= register_cell(sfr, CMOD);
  cell_ccon= register_cell(sfr, CCON);
  for (i= 0; i < 5; i++)
    {
      cell_ccapl[i]= sfr->get_cell(CCAPL[i]);//use_cell(sfr, CCAPL[i]);
      cell_ccaph[i]= sfr->get_cell(CCAPH[i]);//use_cell(sfr, CCAPH[i]);
      cell_ccapm[i]= register_cell(sfr, CCAPM[i]);
    }
  cell_cl= sfr->get_cell(CL);//use_cell(sfr, CL);
  cell_ch= sfr->get_cell(CH);//use_cell(sfr, CH);
  cl_address_space *bas= uc->address_space("bits");
  cell_cr= register_cell(bas, 0xda);
  return(0);
}

void
cl_pca::added_to_uc(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  class cl_it_src *is;
  
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCCF4,
					      0x0033, false, false,
					      "PCA module #4", 5));
  is->init();
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCCF3,
					      0x0033, false, false,
					      "PCA module #3", 5));
  is->init();
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCCF2,
					      0x0033, false, false,
					      "PCA module #2", 5));
  is->init();
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCCF1,
					      0x0033, false, false,
					      "PCA module #1", 5));
  is->init();
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCCF0,
					      0x0033, false, false,
					      "PCA module #0", 5));
  is->init();
  uc->it_sources->add_at(4, is= new cl_it_src(uc, bmEC,
					      sfr->get_cell(IE), bmEC,
					      sfr->get_cell(CCON), bmCF,
					      0x0033, false, false,
					      "PCA counter", 5));
  is->init();
}

void
cl_pca::write(class cl_memory_cell *cell, t_mem *val)
{
  //uchar bmCEX[5]= {bmCEX0, bmCEX1, bmCEX2, bmCEX3, bmCEX4};
  //uchar bmCCF[5]= {bmCCF0, bmCCF1, bmCCF2, bmCCF3, bmCCF4};

  if (cell == cell_cmod)
    {
      bit_CIDL= *val & bmCIDL;
      bit_WDTE= *val & bmWDTE;
      bit_ECF = *val & bmECF;
      t_mem o= clk_source;
      if ((clk_source= *val & (bmCPS1|bmCPS0)) != o)
	t0_overflows= ECI_edge= 0;
    }
  else if (cell == cell_ccon)
    {
      bit_CR= *val & bmCR;
    }
  else if (cell == cell_cr)
    {
      bit_CR= *val;
    }
  else
    {
      int i;
      for (i= 0; i < 5; i++)
	{
	  if (cell == cell_ccapm[i])
	    {
	      t_mem o= ccapm[i];
	      ccapm[i]= *val & 0xff;
	      if (o != ccapm[i])
		cex_neg[i]= cex_pos[i]= false;
	    }
	  else
	    {
	      if (ccapm[i] & (bmMAT|bmTOG))
	      {
		if (cell == cell_ccapl[i])
		  {
		    cell_ccapm[i]->set_bit0(bmECOM);
		    ccapm[i]= cell_ccapm[i]->get();
		  }
		else if (cell == cell_ccaph[i])
		  {
		    cell_ccapm[i]->set_bit1(bmECOM);
		    ccapm[i]= cell_ccapm[i]->get();
		  }
	      }
	    }
	}
    }
}

/*void
cl_pca::mem_cell_changed(class cl_m *mem, t_addr addr)
{
  class cl_m *sfr= uc->mem(MEM_SFR);

  if (mem && sfr && mem == sfr)
    {
      if (addr == addr_ccapXl)
	ccapXl= sfr->get_cell(addr_ccapXl);
      else if (addr == addr_ccapXh)
	ccapXh= sfr->get_cell(addr_ccapXh);
      else if (addr == addr_ccapmX)
	ccapmX= sfr->get_cell(addr_ccapmX);
    }
}*/

int
cl_pca::tick(int cycles)
{
  int ret= resGO;

  if (!bit_CR)
    return(resGO);
  if (uc->state == stIDLE &&
      bit_CIDL)
    return(resGO);

  switch (clk_source)
    {
    case 0:
      do_pca_counter(cycles);
      break;
    case bmCPS0:
      do_pca_counter(cycles*3);
      break;
    case bmCPS1:
      do_pca_counter(t0_overflows);
      t0_overflows= 0;
      break;
    case (bmCPS0|bmCPS1):
      do_pca_counter(ECI_edge);
      ECI_edge= 0;
      break;
    }
  return(ret);
}

void
cl_pca::do_pca_counter(int cycles)
{
  //class cl_m *sfr= uc->mem(MEM_SFR);

  while (cycles--)
    {
      if (cell_cl->add(1) == 0)
	{
	  int i;
	  for (i= 0; i < 5; i++)
	    if (ccapm[i] & bmPWM)
	      cell_ccapl[i]->set(cell_ccaph[i]->get());
	  if (cell_ch->add(1) == 0)
	    {
	      // CH,CL overflow
	      cell_ccon->set_bit1(bmCF);
	      do_pca_module(0);
	      do_pca_module(1);
	      do_pca_module(2);
	      do_pca_module(3);
	      do_pca_module(4);
	    }
	}
    }
}

void
cl_pca::do_pca_module(int nr)
{
  uchar bmCEX[5]= {bmCEX0, bmCEX1, bmCEX2, bmCEX3, bmCEX4};
  uchar bmCCF[5]= {bmCCF0, bmCCF1, bmCCF2, bmCCF3, bmCCF4};
  //uint p1= sfr->get(P1);

  bool capture= false;
  if ((ccapm[nr] & bmCAPP) &&
      cex_pos[nr])
    {
      capture= true;
      cex_pos[nr]= false;
    }
  if ((ccapm[nr] & bmCAPN) &&
      cex_neg[nr])
    {
      capture= true;
      cex_pos[nr]= false;
    }
  if (capture)
    {
      // Capture
      cell_ccapl[nr]->set(cell_cl->get());
      cell_ccaph[nr]->set(cell_ch->get());
      cell_ccon->set_bit1(bmCCF[nr]);
    }

  if (ccapm[nr] & bmECOM)
    {
      // Comparator enabled
      if (cell_cl->get() == cell_ccapl[nr]->get() &&
	  cell_ch->get() == cell_ccaph[nr]->get())
	{
	  // Match
	  if (nr == 4 &&
	      (bit_WDTE))
	    {
	      reset();
	      return;
	    }
	  cell_ccon->set_bit1(bmCCF[nr]);
	  if (ccapm[nr] & bmTOG)
	    {
	      // Toggle
	      sfr->set(P1, sfr->get(P1) ^ bmCEX[nr]);
	    }
	}
      if (ccapm[nr] & bmPWM)
	{
	  // PWM
	  /*if (cell_cl->get() == 0)
	    cell_ccapl[nr]->set(cell_ccaph[nr]->get());*/
	  if (cell_cl->get() < cell_ccapl[nr]->get())
	    //sfr->set(P1, sfr->get(P1) & ~(bmCEX[nr]));
	    sfr->set_bit1(P1, bmCEX[nr]);
	  else
	    sfr->set_bit1(P1, bmCEX[nr]);
	}
    }
}

void
cl_pca::reset(void)
{
  t0_overflows= ECI_edge= 0;
  int i;
  for (i= 0; i < 5; cex_pos[i]= cex_neg[i]= false, i++) ;
}

void
cl_pca::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;
  uchar bmCEX[5]= {bmCEX0, bmCEX1, bmCEX2, bmCEX3, bmCEX4};

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 1)
    {
      t_mem p1n= ep->new_pins & ep->new_value;
      t_mem p1o= ep->pins & ep->prev_value;
      if (!(p1n & bmECI) &&
	  (p1o & bmECI))
	ECI_edge++;
      int i;
      for (i= 0; i < 5; i++)
	{
	  if (!(p1n & bmCEX[i]) &&
	      (p1o & bmCEX[i]))
	    cex_neg[i]= true;
	  else if ((p1n & bmCEX[i]) &&
		   !(p1o & bmCEX[i]))
	    cex_pos[i]= true;
	}
    }
  else if (where->cathegory == HW_TIMER &&
	   he == EV_OVERFLOW &&
	   where->id == 0)
    {
      t0_overflows++;
    }
}


void
cl_pca::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d] FIXME\n", id_string, id);
}


/* End of s51.src/pca.cc */
