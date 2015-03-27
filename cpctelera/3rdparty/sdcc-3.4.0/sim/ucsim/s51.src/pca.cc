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
  for (i= 0; i < 5; cex_pos[i]= cex_neg[i]= DD_FALSE, i++) ;
}

int
cl_pca::init(void)
{
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
  register_cell(sfr, CMOD, &cell_cmod, wtd_restore_write);
  register_cell(sfr, CCON, &cell_ccon, wtd_restore_write);
  for (i= 0; i < 5; i++)
    {
      use_cell(sfr, CCAPL[i], &cell_ccapl[i], wtd_restore);
      use_cell(sfr, CCAPH[i], &cell_ccaph[i], wtd_restore);
      register_cell(sfr, CCAPM[i], &cell_ccapm[i], wtd_restore_write);
    }
  use_cell(sfr, CL, &cell_cl, wtd_restore);
  use_cell(sfr, CH, &cell_ch, wtd_restore);
  return(0);
}

void
cl_pca::added_to_uc(void)
{
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF4, 0x0033, false,
                                          "PCA module #4", 5));
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF3, 0x0033, false,
                                          "PCA module #3", 5));
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF2, 0x0033, false,
                                          "PCA module #2", 5));
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF1, 0x0033, false,
                                          "PCA module #1", 5));
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCCF0, 0x0033, false,
                                          "PCA module #0", 5));
  uc->it_sources->add_at(4, new cl_it_src(bmEC, CCON, bmCF, 0x0033, false,
                                          "PCA counter", 5));
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
                cex_neg[i]= cex_pos[i]= DD_FALSE;
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

  bool capture= DD_FALSE;
  if ((ccapm[nr] & bmCAPP) &&
      cex_pos[nr])
    {
      capture= DD_TRUE;
      cex_pos[nr]= DD_FALSE;
    }
  if ((ccapm[nr] & bmCAPN) &&
      cex_neg[nr])
    {
      capture= DD_TRUE;
      cex_pos[nr]= DD_FALSE;
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
  for (i= 0; i < 5; cex_pos[i]= cex_neg[i]= DD_FALSE, i++) ;
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
            cex_neg[i]= DD_TRUE;
          else if ((p1n & bmCEX[i]) &&
                   !(p1o & bmCEX[i]))
            cex_pos[i]= DD_TRUE;
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
