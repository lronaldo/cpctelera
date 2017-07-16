/*
 * Simulator of microcontrollers (stm8.src/clk.cc)
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

#include "stm8cl.h"

#include "clkcl.h"


cl_clk::cl_clk(class cl_uc *auc):
       cl_hw(auc, HW_CLOCK, 0, "clk")
{
  base= 0x50C0;
  ckdivr= NULL;
  pckenr1= NULL;
  pckenr2= NULL;
  pckenr3= NULL;
}

int
cl_clk::init(void)
{
  cl_hw::init();

  make_partner(HW_TIMER, 1);
  make_partner(HW_TIMER, 2);
  make_partner(HW_TIMER, 3);
  make_partner(HW_TIMER, 4);
  make_partner(HW_TIMER, 5);
  make_partner(HW_TIMER, 6);

  make_partner(HW_UART, 1);
  make_partner(HW_UART, 2);
  make_partner(HW_UART, 3);
  make_partner(HW_UART, 4);
  
  return 0;
}

void
cl_clk::write(class cl_memory_cell *cell, t_mem *val)
{
  cl_clk_event e;
  hw_event ev;
  
  if ((cell == pckenr1) ||
      (cell == pckenr2) ||
      (cell == pckenr3))
    {
      cell->set(*val);

      e.set(HW_TIMER, 1);
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 2;
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 3;
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 4;
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 5;
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 6;
      ev= tim(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.set(HW_UART, 1);
      ev= usart(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 2;
      ev= usart(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 3;
      ev= usart(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);

      e.id= 4;
      ev= usart(e.id)?EV_CLK_ON:EV_CLK_OFF;
      inform_partners(ev, &e);
    }
}


/* SAF */

cl_clk_saf::cl_clk_saf(class cl_uc *auc):
  cl_clk(auc)
{
}

int
cl_clk_saf::init(void)
{
  cl_clk::init();
  ckdivr= register_cell(uc->rom, base+6);
  pckenr1= register_cell(uc->rom, base+7);
  pckenr2= register_cell(uc->rom, base+10);
  return 0;
}

void
cl_clk_saf::reset(void)
{
  //ickr->write(0x01);
  //eckr->write(0);
  //cmsr->write(0xe1);
  //swr->write(0xe1);
  //swcr->write(0);
  ckdivr->write(0x18);
  pckenr1->write(0xff);
  //cssr->write(0);
  //ccor->write(0);
  pckenr2->write(0xff);
  //hsitrimr->write(0);
  //swimccr->write(0);
}

bool
cl_clk_saf::tim(int id)
{
  switch (id)
    {
    case 1:
      return pckenr1 && (pckenr1->get() & 0x80);
    case 2: case 5:
      return pckenr1 && (pckenr1->get() & 0x20);
    case 3:
      return pckenr1 && (pckenr1->get() & 0x40);
    case 4: case 6:
      return pckenr1 && (pckenr1->get() & 0x10);
    }
  return false;
}

bool
cl_clk_saf::usart(int id)
{
  cl_stm8 *u= (cl_stm8 *)uc;
  if (id == 1)
    switch (u->type->subtype)
      {
      case DEV_STM8S003: case DEV_STM8S103: case DEV_STM8S903:
	return pckenr1 && (pckenr1->get() & 0x08);
      case DEV_STM8S007: case DEV_STM8S207: case DEV_STM8S208:
      case DEV_STM8AF52:
	return pckenr1 && (pckenr1->get() & 0x04);
      }
  else if (id == 2)
    switch (u->type->subtype)
      {
      case DEV_STM8S005: case DEV_STM8S105: case DEV_STM8AF62_46:
	return pckenr1 && (pckenr1->get() & 0x08);
      }
  else if (id == 3)
    switch (u->type->subtype)
      {
      case DEV_STM8S007: case DEV_STM8S207: case DEV_STM8S208:
      case DEV_STM8AF52:
	return pckenr1 && (pckenr1->get() & 0x08);
      }
  else if (id == 4)
    switch (u->type->subtype)
      {
      case DEV_STM8AF62_12:
	return pckenr1 && (pckenr1->get() & 0x08);
      }
  return false;
}

/* ALL */

cl_clk_all::cl_clk_all(class cl_uc *auc):
  cl_clk(auc)
{
}

int
cl_clk_all::init(void)
{
  cl_clk::init();
  ckdivr= register_cell(uc->rom, base+0);
  pckenr1= register_cell(uc->rom, base+3);
  pckenr2= register_cell(uc->rom, base+4);
  pckenr3= register_cell(uc->rom, base+16);
  return 0;
}

void
cl_clk_all::reset(void)
{
  ckdivr->write(3);
  pckenr1->write(0);
  pckenr2->write(0x80);
  pckenr3->write(0);
}

bool
cl_clk_all::tim(int id)
{
  switch (id)
    {
    case 1:
      return pckenr2 && (pckenr2->get() & 0x02);
    case 2:
      return pckenr1 && (pckenr1->get() & 0x01);
    case 3:
      return pckenr1 && (pckenr1->get() & 0x02);
    case 4:
      return pckenr1 && (pckenr1->get() & 0x04);
    case 5:
      return pckenr3 && (pckenr3->get() & 0x02);
    }
  return false;
}

bool
cl_clk_all::usart(int id)
{
  switch (id)
    {
    case 1:
      return pckenr1 && (pckenr1->get() & 0x20);
    case 2:
      return pckenr3 && (pckenr3->get() & 0x08);
    case 3:
      return pckenr3 && (pckenr3->get() & 0x10);
    }
  return false;
}


/* L101 */

cl_clk_l101::cl_clk_l101(class cl_uc *auc):
  cl_clk(auc)
{
}

int
cl_clk_l101::init(void)
{
  cl_clk::init();
  ckdivr= register_cell(uc->rom, base+0);
  pckenr1= register_cell(uc->rom, base+3);
  return 0;
}

void
cl_clk_l101::reset(void)
{
  ckdivr->write(3);
  pckenr1->write(0);
}

bool
cl_clk_l101::tim(int id)
{
  switch (id)
    {
    case 2:
      return pckenr1 && (pckenr1->get() & 0x01);
    case 3:
      return pckenr1 && (pckenr1->get() & 0x02);
    case 4:
      return pckenr1 && (pckenr1->get() & 0x04);
    }
  return false;
}

bool
cl_clk_l101::usart(int id)
{
  switch (id)
    {
    case 1:
      return pckenr1 && (pckenr1->get() & 0x20);
    }
  return false;
}


/* End of stm8.src/clk.cc */
