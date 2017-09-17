/*
 * Simulator of microcontrollers (timer0.cc)
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

#include "timer0cl.h"
#include "regs51.h"
#include "types51.h"
#include "uc51cl.h"


cl_timer0::cl_timer0(class cl_uc *auc, int aid, const char *aid_string):
  cl_hw(auc, HW_TIMER, aid, aid_string)
{
  cell_tmod= cell_tcon= 0;
  if (aid == 0)
    {
      mask_M0  = bmM00;
      mask_M1  = bmM10;
      mask_C_T = bmC_T0;
      mask_GATE= bmGATE0;
      mask_TR  = bmTR0;
      mask_INT = bm_INT0;
      mask_TF  = bmTF0;
      mask_T   = bmT0;
      addr_tl  = TL0;
      addr_th  = TH0;
      addr_tcon= TCON;
    }
  else if (aid == 1)
    {
      mask_M0  = bmM01;
      mask_M1  = bmM11;
      mask_C_T = bmC_T1;
      mask_GATE= bmGATE1;
      mask_TR  = bmTR1;
      mask_INT = bm_INT1;
      mask_TF  = bmTF1;
      mask_T   = bmT1;
      addr_tl  = TL1;
      addr_th  = TH1;
      addr_tcon= TCON;
    }
  else if (aid == 2)
    {
      addr_tl  = TL2;
      addr_th  = TH2;
      mask_T   = bmT2;
      mask_C_T = bmC_T2;
      mask_TR  = bmTR2;
      mask_TF  = bmTF2;
      mask_M0= mask_M1= mask_GATE= mask_INT= 0;
      addr_tcon= T2CON;
    }
  else {}
  make_partner(HW_PCA, 0);
  make_partner(HW_PCA, 1);
  make_partner(HW_PCA, 2);
  make_partner(HW_PCA, 3);
  make_partner(HW_PCA, 4);
}

int
cl_timer0::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  int i;
  
  cl_hw::init();
  bas= uc->address_space("bits");
  if (sfr)
    {
      //t_mem d;
      if (id == 0 || id == 1)
	{
	  cell_tmod= register_cell(sfr, TMOD);
	  cell_tcon= register_cell(sfr, TCON);
	  INT= sfr->read(P3) & mask_INT;
	}
      else if (id == 2)
	{
	  cell_tmod= 0;
	  cell_tcon= register_cell(sfr, T2CON);
	}
      cell_tl= sfr->get_cell(addr_tl);//use_cell(sfr, addr_tl);
      cell_th= sfr->get_cell(addr_th);//use_cell(sfr, addr_th);
    }
  for (i= 0; i < 8; i++)
    {
      tcon_bits[i]= register_cell(bas, addr_tcon + i);
    }
  return(0);
}

void
cl_timer0::added_to_uc(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  class cl_it_src *is;
  
  if (id == 0)
    {
      uc->it_sources->add(is= new cl_it_src(uc, bmET0,
					    sfr->get_cell(IE), bmET0,
					    sfr->get_cell(TCON), bmTF0,
					    0x000b, true, false,
					    "timer #0", 2));
      is->init();
    }
  else if (id == 1)
    {
      uc->it_sources->add(is= new cl_it_src(uc, bmET1,
					    sfr->get_cell(IE), bmET1,
					    sfr->get_cell(TCON), bmTF1,
					    0x001b, true, false,
					    "timer #1", 4));
      is->init();
    }
}

/*t_mem
cl_timer0::read(class cl_cell *cell)
{
  return(cell->get());
}*/

void
cl_timer0::write(class cl_memory_cell *cell, t_mem *val)
{
  t_addr ba;
  bool b= bas->is_owned(cell, &ba);
  u8_t n= *val;
  
  if (b)
    {
      u8_t m= 1 << (ba - addr_tcon);
      n= cell_tcon->get();
      if (*val)
	n|= m;
      else
	n&= ~m;
    }
  if (cell == cell_tmod)
    {
      t_mem md= *val & (mask_M0|mask_M1);
      if (md == mask_M0)
	mode= 1;
      else if (md == mask_M1)
	mode= 2;
      else if (md == (mask_M0|mask_M1))
	mode= 3;
      else
	mode= 0;
      GATE= *val & mask_GATE;
      C_T = *val & mask_C_T;
      T_edge= 0;
    }
  else if ((cell == cell_tcon) ||
	   b)
    {
      TR= n & mask_TR;
      T_edge= 0;
    }
}

/*void
cl_timer0::mem_cell_changed(class cl_m *mem, t_addr addr)
{
  //class cl_m *sfr= uc->mem(MEM_SFR);
  //t_mem d;

  cl_hw::mem_cell_changed(mem, addr);

  //d= cell_tmod->get();
  //write(cell_tmod, &d);
  //d= cell_tcon->get();
  //write(cell_tcon, &d);
  //if (addr == addr_tl) cell_tl= sfr->get_cell(addr_tl);
  //if (addr == addr_th) cell_th= sfr->get_cell(addr_th);
}*/

int
cl_timer0::tick(int cycles)
{
  switch (mode)
    {
    case 0: do_mode0(cycles); break;
    case 1: do_mode1(cycles); break;
    case 2: do_mode2(cycles); break;
    case 3: do_mode3(cycles); break;
    }
  return(resGO);
}

int
cl_timer0::do_mode0(int cycles)
{
  if (!TR)
    return(0);

  //t_mem p3= uc->mem(MEM_SFR)->get(P3);
  if (GATE)
    {
      if ((/*p3 & mask_*/INT) == 0)
	return(0);
    }

  if (C_T)
    {
      /*cycles= 0;
      if ((uc51->prev_p3 & mask_T) &&
	  !(p3 & uc51->port_pins[3] & mask_T))
	  cycles= 1;*/
      cycles= T_edge;
      T_edge= 0;
    }
  while (cycles--)
    {
      // mod 0, TH= 8 bit t/c, TL= 5 bit precounter
      t_mem tl= cell_tl->add(1);
      if ((tl & 0x1f) == 0)
	{
	  cell_tl->set(0);
	  if (!cell_th->add(1))
	    {
	      cell_tcon->set_bit1(mask_TF);
	      overflow();
	    }
	}
    }

  return(0);
}

int
cl_timer0::do_mode1(int cycles)
{
  if (!TR)
    return(0);

  //t_mem p3= uc->mem(MEM_SFR)->get(P3);
  if (GATE)
    {
      if ((/*p3 & mask_*/INT) == 0)
	return(0);
    }

  if (C_T)
    {
      /*cycles= 0;
      if ((uc51->prev_p3 & mask_T) &&
	  !(p3 & uc51->port_pins[3] & mask_T))
	  cycles= 1;*/
      cycles= T_edge;
      T_edge= 0;
    }

  while (cycles--)
    {
      // mod 1 TH+TL= 16 bit t/c
      if (!cell_tl->add(1))
	{
	  if (!cell_th->add(1))
	    {
	      cell_tcon->set_bit1(mask_TF);
	      overflow();
	    }
	}
    }

  return(0);
}

int
cl_timer0::do_mode2(int cycles)
{
  if (!TR)
    return(0);

  //t_mem p3= uc->mem(MEM_SFR)->get(P3);
  if (GATE)
    {
      if ((/*p3 & mask_*/INT) == 0)
	return(0);
    }

  if (C_T)
    {
      /*cycles= 0;
      if ((uc51->prev_p3 & mask_T) &&
	  !(p3 & uc51->port_pins[3] & mask_T))
	  cycles= 1;*/
      cycles= T_edge;
      T_edge= 0;
    }

  //unsigned long startt= uc->ticks->ticks-(cycles*12);int i=0;
  while (cycles--)
    {
      // mod 2 TL= 8 bit t/c auto reload from TH
      if (!cell_tl->add(1))
	{
	  cell_tl->set(cell_th->get());
	  cell_tcon->set_bit1(mask_TF);
	  //printf("timer%d overflow %d (%d) %d\n",id,uc->ticks->ticks,i,startt+(i*12));
	  overflow();
	}
      //i++;
    }
  return(0);
}

int
cl_timer0::do_mode3(int cycles)
{
  int cyc= cycles;
  //t_mem p3= uc->mem(MEM_SFR)->get(P3);

  if (!TR)
    goto do_th;

  if (GATE)
    {
      if ((/*p3 & mask_*/INT) == 0)
	goto do_th;
    }

  if (C_T)
    {
      /*cycles= 0;
      if ((uc51->prev_p3 & mask_T) &&
	  !(p3 & uc51->port_pins[3] & mask_T))
	  cycles= 1;*/
      cycles= T_edge;
      T_edge= 0;
    }

  while (cycles--)
    {
      if (!cell_tl->add(1))
	{
	  cell_tcon->set_bit1(mask_TF);
	  overflow();
	}
    }

 do_th:
  if ((cell_tcon->get() & bmTR1) != 0)
    while (cyc--)
      {
	if (!cell_th->add(1))
	  cell_tcon->set_bit1(bmTF1);
      }
  return(0);
}

void
cl_timer0::overflow(void)
{
  inform_partners(EV_OVERFLOW, 0);
}

void
cl_timer0::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 3)
    {
      t_mem p3n= ep->new_pins & ep->new_value;
      t_mem p3o= ep->pins & ep->prev_value;
      if ((p3n & mask_T) &&
	  !(p3o & mask_T))
	T_edge++;
      INT= p3n & mask_INT;
      //printf("timer%d p%dchanged (%02x,%02x->%02x,%02x) INT=%d(%02x) edge=%d(%02x)\n",id,where->id,ep->prev_value,ep->pins,ep->new_value,ep->new_pins,INT,mask_INT,T_edge,mask_T);
    }
}

void
cl_timer0::print_info(class cl_console_base *con)
{
  const char *modes[]= { "13 bit", "16 bit", "8 bit autoreload", "2x8 bit" };
  //t_mem tmod= cell_tmod->get();
  int on;
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  //int mode= tmod & (bmM00|bmM10);
  con->dd_printf(" %s", modes[mode]);
  con->dd_printf(" %s", (/*tmod&bm*/C_T/*0*/)?"counter":"timer");
  if (/*tmod&bm*/GATE/*0*/)
    {
      con->dd_printf(" gated");
      on= INT;
    }
  else
    on= TR;
  con->dd_printf(" %s", on?"ON":"OFF");
  con->dd_printf(" irq=%c", (cell_tcon->get()&mask_TF)?'1':'0');
  con->dd_printf(" %s", sfr?"?":((sfr->get(IE)&bmET0)?"en":"dis"));
  con->dd_printf(" prio=%d", uc->priority_of(bmPT0));
  con->dd_printf("\n");
}


/* End of s51.src/timer0.cc */
