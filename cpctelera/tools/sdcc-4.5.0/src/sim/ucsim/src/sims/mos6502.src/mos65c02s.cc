/*
 * Simulator of microcontrollers (mos65c02s.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#include <stdlib.h>
#include <ctype.h>

#include "appcl.h"

#include "glob.h"

#include "mos65c02scl.h"


cl_mos65c02s::cl_mos65c02s(class cl_sim *asim):
  cl_mos65c02(asim)
{
  *my_id= "MOS65C02S";
}

int
cl_mos65c02s::init(void)
{
  cl_mos65c02::init();
  itab[0xcb]= instruction_wrapper_cb; // WAI
  itab[0xdb]= instruction_wrapper_db; // STOP
  return 0;
}


struct dis_entry *
cl_mos65c02s::get_dis_entry(t_addr addr)
{
  struct dis_entry *de;

  t_mem code= rom->get(addr);
  for (de = disass_mos65c02s; de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
        return de;
    }
  return cl_mos65c02::get_dis_entry(addr);
}

int
cl_mos65c02s::inst_length(t_addr addr)
{
  struct dis_entry *de= get_dis_entry(addr);
  if (!de)
    return 1;
  return (de->mnemonic)?(de->length):1;
}


int
cl_mos65c02s::WAI(t_mem code)
{
  // TODO: wai
  tick(2);
  return resGO;
}

int
cl_mos65c02s::STP(t_mem code)
{
  // TODO: stp
  tick(2);
  return resGO;
}


int
cl_mos65c02s::rmb(t_mem code, class cl_cell8 &op)
{
  u8_t v= op.R();
  u8_t mask= 1<<((code>>4)&0x7);
  v&= ~mask;
  op.W(v);
  return resGO;
}

int
cl_mos65c02s::smb(t_mem code, class cl_cell8 &op)
{
  u8_t v= op.R();
  u8_t mask= 1<<((code>>4)&0x7);
  v|= mask;
  op.W(v);
  return resGO;
}

int
cl_mos65c02s::bbr(t_mem code, class cl_cell8 &op)
{
  u8_t v= op.R();
  u8_t mask= 1<<((code>>4)&0x7);
  i8_t offset= fetch();
  if (!(v&mask))
    {
      PC+= offset;
      PC&= 0xffff;
    }
  tick(2);
  return resGO;
}

int
cl_mos65c02s::bbs(t_mem code, class cl_cell8 &op)
{
  u8_t v= op.R();
  u8_t mask= 1<<((code>>4)&0x7);
  i8_t offset= fetch();
  if (v&mask)
    {
      PC+= offset;
      PC&= 0xffff;
    }
  tick(2);
  return resGO;
}


/* End of mos6502.src/mos65c02s.cc */
