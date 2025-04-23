/*
 * Simulator of microcontrollers (t16.cc)
 *
 * Copyright (C) 2024 Drotos Daniel
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

#include "pdkcl.h"
#include "osccl.h"

#include "t16cl.h"


cl_t16::cl_t16(class cl_uc *auc, const char *aname):
  cl_hw(auc, HW_TIMER, 0, aname)
{
  puc= (class cl_pdk *)auc;
  src= NULL;
}

int
cl_t16::init(void)
{
  mod= register_cell(puc->sfr, 6);
  edg= register_cell(puc->sfr, 0xc);
  irq= register_cell(puc->sfr, 5);
  cl_hw::init();
  uc->mk_mvar(cfg, t16_cnt, "T16", cfg_help(t16_cnt));
  reset();
  return 0;
}

const char *
cl_t16::cfg_help(t_addr addr)
{
  switch ((enum t16_cfg)addr)
    {
    case t16_on: return "Turn ticking of T16 on/off (bool, RW)";
    case t16_cnt: return "T16 counter value (RW)";
    case t16_nuof: return "";
    }
  return "Not used";
}

void
cl_t16::reset(void)
{
  cnt= 0;
  last= 0;
  mod->set(0);
  recalc();
}

void
cl_t16::recalc(void)
{
  u8_t v= mod->get();
  switch ((v>>5)&7)
    {
    case 0: clk_source="None"; src= NULL; break;
    case 1: clk_source="SysClk"; src= &(puc->osc->sys); break;
    case 2: clk_source="None"; src= NULL; break;
    case 3: /* TODO PA4 */ clk_source="PA4"; src= NULL; break;
    case 4: clk_source="ihrc"; src= &(puc->osc->ihrc); break;
    case 5: clk_source="eosc"; src= &(puc->osc->eosc); break;
    case 6: clk_source="ilrc"; src= &(puc->osc->ilrc); break;
    case 7: /* TODO PA0 */ clk_source="PA0"; src= NULL; break;
    }
  set_div();
  mask= 1<<((v&7)+8);
  pre= 0;
  if (src)
    last= *src;
}

void
cl_t16::set_div(void)
{
  u8_t v= mod->get();
  switch ((v>>3)&3)
    {
    case 0: div= 1; break;
    case 1: div= 4; break;
    case 2: div= 16; break;
    case 3: div= 64; break;
    }
}

void
cl_t16::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == mod)
    {
      if ((*val & 0xff) != cell->get())
	{
	  cell->set(*val);
	  recalc();
	}
    }
  /*else if (cell == egs)
    {
    }*/
  /*else if (cell == irq)
    {
    }*/
  cell->set(*val);
}

t_mem
cl_t16::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case t16_on: // turn this HW on/off
      if (val)
	on= *val;
      else
	cell->set(on?1:0);
      break;
    case t16_cnt:
      if (val)
	cnt= *val & 0xffff;
      else
	cell->set(cnt);
      break;
    }
    return cell->get();
}

int
cl_t16::tick(int cycles)
{
  if (src)
    {
      t_mem act= *src;
      if (act != last)
	{
	  int d= act - last, i;
	  if (d<0) d= -d;
	  if (d)
	    {
	      pre+= d;
	      if ((i= pre/div))
		{
		  u16_t prev= cnt;
		  cnt+= i;
		  pre%= div;
		  if ((prev & mask) != (cnt & mask))
		    {
		      // 0= rising, 1= falling
		      u8_t edge= edg->get() & 0x10;
		      if (( edge && !(cnt & mask)) ||
			  (!edge &&  (cnt & mask)))
			{
			  irq->write(irq->get() | 4);
			}
		    }
		}
	    }
	  last= act;
	}
    }
  return 0;
}

void
cl_t16::print_info(class cl_console_base *con)
{
  con->dd_printf("T16 Src=%s/%d pre=%u mask=%04x edge=%s irq=%d\n",
		 clk_source.c_str(), div, pre, mask,
		 (edg->get()&0x10)?"fall":"rise",
		 (irq->get()&4)?1:0);
  con->dd_printf("cnt= %5u 0x%04x\n", cnt, cnt);
}


/* End of pdk.src/t16.cc */
