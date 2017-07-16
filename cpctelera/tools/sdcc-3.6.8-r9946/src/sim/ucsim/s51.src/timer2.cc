/*
 * Simulator of microcontrollers (timer2.cc)
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

#include "timer2cl.h"
#include "regs51.h"
#include "types51.h"


cl_timer2::cl_timer2(class cl_uc *auc, int aid, const char *aid_string,
		     int afeatures):
  cl_timer0(auc, /*2*/aid, /*"timer2"*/aid_string)
{
  features= afeatures;
  exf2it= 0;
  mask_RCLK= bmRCLK;
  mask_TCLK= bmTCLK;
  mask_CP_RL2= bmCP_RL2;
  make_partner(HW_UART, 0);
  sfr= uc->address_space(MEM_SFR_ID);
  if (features & (t2_down|t2_clock_out))
    {
      cell_t2mod= register_cell(sfr, T2MOD);
    }
}

int
cl_timer2::init(void)
{
  cl_timer0::init();
  cell_rcap2l= sfr->get_cell(RCAP2L);//use_cell(sfr, RCAP2L);
  cell_rcap2h= sfr->get_cell(RCAP2H);//use_cell(sfr, RCAP2H);
  if (sfr)
    bit_t2ex= sfr->read(P1) & bmT2EX;
  return(0);
}

void
cl_timer2::added_to_uc(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  class cl_it_src *is;
  
  uc->it_sources->add(is= new cl_it_src(uc, bmET2,
					sfr->get_cell(IE), bmET2,
					sfr->get_cell(T2CON), bmTF2,
					0x002b, false, false,
					"timer #2 TF2", 7));
  is->init();
  exf2it= new cl_it_src(uc, bmET2,
			sfr->get_cell(IE), bmET2,
			sfr->get_cell(T2CON), bmEXF2,
			0x002b, false, false,
			"timer #2 EXF2", 7);
  exf2it->init();
  uc->it_sources->add(exf2it);
}

void
cl_timer2::write(class cl_memory_cell *cell, t_mem *val)
{
  int oldmode= mode;
  bool oldtr= TR;
  t_addr ba;
  bool b= bas->is_owned(cell, &ba);
  t_mem n= *val;
  
  if (b)
    {
      u8_t m= 1 << (ba - addr_tcon);
      n= cell_tcon->get();
      if (*val)
	n|= m;
      else
	n&= ~m;
    }
  if (exf2it)
    exf2it->activate();
  if ((cell == cell_tcon) ||
      b)
    {
      C_T = n & mask_C_T;
      TR  = n & mask_TR;
      RCLK= n & mask_RCLK;
      TCLK= n & mask_TCLK;
      CP_RL2= n & mask_CP_RL2;
      EXEN2 = n & bmEXEN2;
      if (!(RCLK || TCLK) &&
	  !CP_RL2)
	mode= T2MODE_RELOAD;
      else if (!(RCLK || TCLK) &&
	       CP_RL2)
	mode= T2MODE_CAPTURE;
      else if (RCLK || TCLK)
	mode= T2MODE_BAUDRATE;
      else
	mode= T2MODE_OFF;
      if (mode != oldmode)
	inform_partners(EV_T2_MODE_CHANGED, &n);
    }
  else if (cell == cell_t2mod)
    {
      bit_dcen= (*val & bmDCEN) != 0;
      bit_t2oe= (*val & bmT2OE) != 0;
      if ((features & t2_down) &&
	  bit_dcen &&
	  mode == T2MODE_RELOAD)
	{
	  mode= T2MODE_DOWN;
	  if (exf2it)
	    exf2it->deactivate();
	}
      if ((features & t2_clock_out) &&
          bit_t2oe)
        mode= T2MODE_CLKOUT;
    }
  if ((mode != oldmode) ||
      (TR && !oldtr) ||
      (!TR && oldtr))
    T_edge= t2ex_edge= 0;
}

int
cl_timer2::tick(int cycles)
{ 
  switch (mode)
    {
    case T2MODE_BAUDRATE:
      do_t2_baud(cycles);
      break;
    case T2MODE_CAPTURE:
      do_t2_capture(cycles);
      break;
    case T2MODE_RELOAD:
      do_t2_reload(cycles);
      break;
    case T2MODE_DOWN:
      do_t2_down(cycles);
      break;
    case T2MODE_CLKOUT:
      do_t2_clock_out(cycles);
      break;
    default: break;
    }
  
  return(resGO);
}

/*
 * Baud rate generator mode of Timer #2
 */

int
cl_timer2::do_t2_baud(int cycles)
{
  if (EXEN2 && t2ex_edge)
    {
      cell_tcon->set_bit1(bmEXF2);
      t2ex_edge= 0;
    }

  if (!TR)
    return(0);

  if (C_T)
    (cycles= T_edge), T_edge= 0;
  else
    cycles*= 6;

  while (cycles--)
    {
      if (!cell_tl->add(1))
	if (!cell_th->add(1))
	  {
	    cell_th->set(cell_rcap2h->get());
	    cell_tl->set(cell_rcap2l->get());
	    inform_partners(EV_OVERFLOW, 0);
	  }
    }
  return(resGO);
}


/*
 * Capture function of Timer #2
 */

void
cl_timer2::do_t2_capture(int cycles)
{
  if (EXEN2 && t2ex_edge)
    {
      cell_tcon->set_bit1(bmEXF2);
      cell_rcap2h->set(cell_th->get());
      cell_rcap2l->set(cell_tl->get());
      t2ex_edge= 0;
    }

  if (!TR)
    return;

  if (C_T)
    (cycles= T_edge), T_edge= 0;

  if (!cell_tl->add(1))
    {
      if (!cell_th->add(1))
	cell_tcon->set_bit1(bmTF2);
    }
}


/*
 * Auto Reload mode of Timer #2, counting UP
 */

void
cl_timer2::do_t2_reload(int cycles)
{
  if (EXEN2 && t2ex_edge)
    {
      cell_tcon->set_bit1(bmEXF2);
      cell_th->set(cell_rcap2h->get());
      cell_tl->set(cell_rcap2l->get());
      t2ex_edge= 0;
    }

  if (!TR)
    return;

  if (C_T)
    (cycles= T_edge), T_edge= 0;

  if (!cell_tl->add(1))
    {
      if (!cell_th->add(1))
	{
	  cell_tcon->set_bit1(mask_TF);
	  cell_th->set(cell_rcap2h->get());
	  cell_tl->set(cell_rcap2l->get());
	}
    }
}

void
cl_timer2::do_t2_down(int cycles)
{
  bool toggle= false;

  if (!TR)
    return;

  if (C_T)
    (cycles= T_edge), T_edge= 0;

  if (bit_t2ex)
    {
      // UP
      while (cycles--)
	if (!cell_tl->add(1))
	  {
	    if (!cell_th->add(1))
	      {
		cell_tcon->set_bit1(mask_TF);
		cell_th->set(cell_rcap2h->get());
		cell_tl->set(cell_rcap2l->get());
		toggle= true;
	      }
	  }
    }
  else
    {
       // DOWN
      while (cycles--)
	{
	  t_mem l, h;
	  if ((l= cell_tl->add(-1)) == 0xff)
	    h= cell_th->add(-1);
	  else
	    h= cell_th->get();
	  if ((u16_t)(h*256+l) <
	      (u16_t)(cell_rcap2h->get()*256+cell_rcap2l->get()))
	    {
	      cell_tcon->set_bit1(mask_TF);
	      cell_th->set(0xff);
	      cell_tl->set(0xff);
	      toggle= true;
	    }
	}
    }
  if (toggle &&
      sfr)
    {
      class cl_memory_cell *p1= sfr->get_cell(P1);
      if (p1)
	p1->set(p1->get() ^ bmEXF2);
    }
}

void
cl_timer2::do_t2_clock_out(int cycles)
{
  if (EXEN2 && t2ex_edge)
    {
      cell_tcon->set_bit1(bmEXF2);
      t2ex_edge= 0;
    }

  if (!TR)
    return;

  if (C_T)
    (cycles= T_edge), T_edge= 0;
  else
    cycles*= 6;

  while (cycles--)
    {
      if (!cell_tl->add(1))
	if (!cell_th->add(1))
	  {
	    cell_th->set(cell_rcap2h->get());
	    cell_tl->set(cell_rcap2l->get());
	    inform_partners(EV_OVERFLOW, 0);
	    if (!C_T &&
		sfr)
	      {
		// toggle T2 on P1
		class cl_memory_cell *p1= sfr->get_cell(P1);
		if (p1)
		  p1->set(p1->get() ^ bmT2);
	      }
	  }
    }
}

void
cl_timer2::happen(class cl_hw *where, enum hw_event he, void *params)
{
  struct ev_port_changed *ep= (struct ev_port_changed *)params;

  if (where->cathegory == HW_PORT &&
      he == EV_PORT_CHANGED &&
      ep->id == 1)
    {
      t_mem p1n= ep->new_pins & ep->new_value;
      t_mem p1o= ep->pins & ep->prev_value;
      if (!(p1n & mask_T) &&
	  (p1o & mask_T))
	T_edge++;
      if (!(p1n & bmT2EX) &&
	  (p1o & bmT2EX))
	t2ex_edge++;
      bit_t2ex= p1n & bmT2EX;
    }
}

void
cl_timer2::print_info(class cl_console_base *con)
{
  int t2con= cell_tcon->get();

  con->dd_printf("%s[%d] 0x%04x", id_string, id,
		 256*cell_th->get()+cell_tl->get());
  if (RCLK || TCLK)
    {
      con->dd_printf(" baud");
      if (RCLK)
	con->dd_printf(" RCLK");
      if (TCLK)
	con->dd_printf(" TCLK");
    }
  else
    con->dd_printf(" %s", (CP_RL2)?"capture":"reload");
  con->dd_printf(" 0x%04x",
		 256*cell_rcap2h->get()+cell_rcap2l->get());
  con->dd_printf(" %s", (C_T)?"counter":"timer");
  con->dd_printf(" %s", (TR)?"ON":"OFF");
  con->dd_printf(" irq=%c", (t2con&bmTF2)?'1':'0');
  con->dd_printf(" %s", sfr?"?":((sfr->get(IE)&bmET2)?"en":"dis"));
  con->dd_printf(" prio=%d", uc->priority_of(bmPT2));
  con->dd_printf("\n");
}


/* End of s51.src/timer2.cc */
