/*
 * Simulator of microcontrollers (bit.cc)
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

// local
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"


/*
 * 0x72 2 24 ORL C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_c_bit(uchar code)
{
  uchar bitaddr;

  t_addr a;
  t_mem m;
  class cl_address_space *mem;
  mem= bit2mem(bitaddr= fetch(), &a, &m);
  SFR_SET_C(SFR_GET_C ||
	    (mem->read(a) & m));
  tick(1);
  return(resGO);
}


/*
 * 0x82 2 24 ANL C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_c_bit(uchar code)
{
  t_mem m;
  t_addr a;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  SFR_SET_C(SFR_GET_C &&
	    (mem->read(a) & m));
  tick(1);
  return(resGO);
}


/*
 * 0x92 2 24 MOV bit,C
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_mov_bit_c(uchar code)
{
  t_addr a;
  t_mem m, d;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  d= mem->read(a, HW_PORT);
  if (SFR_GET_C)
    mem->write(a, d|m);
  else
    mem->write(a, d&~m);
  tick(1);
  return(resGO);
}


/*
 * 0xa2 2 12 MOV C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_mov_c_bit(uchar code)
{
  t_addr a;
  t_mem m;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  SFR_SET_C(mem->read(a) & m);
  return(resGO);
}


/*
 * 0xa0 2 24 ORL C,/bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_orl_c_Sbit(uchar code)
{
  t_mem m;
  t_addr a;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  SFR_SET_C(SFR_GET_C ||
	    !(mem->read(a) & m));
  tick(1);
  return(resGO);
}


/*
 * 0xb0 2 24 ANL C,/bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_anl_c_Sbit(uchar code)
{
  t_mem m;
  t_addr a;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  SFR_SET_C(SFR_GET_C &&
	    !(mem->read(a) & m));
  tick(1);
  return(resGO);
}


/*
 * 0xb2 2 12 CPL bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cpl_bit(uchar code)
{
  t_addr a;
  t_mem m, d;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  d= mem->read(a, HW_PORT);
  mem->write(a, d^m);
  return(resGO);
}


/*
 * 0xb3 1 12 CPL C
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_cpl_c(uchar code)
{
  psw->write(psw->read() ^ bmCY);
  return(resGO);
}


/*
 * 0xc2 2 12 CLR bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_clr_bit(uchar code)
{
  t_addr a;
  t_mem m;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  t_mem d= mem->read(a, HW_PORT);
  mem->write(a, d&~m);
  return(resGO);
}


/*
 * 0xc3 1 12 CLR C
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_clr_c(uchar code)
{
  psw->write(psw->read() & ~bmCY);
  return(resGO);
}


/*
 * 0xd2 2 12 SETB bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_setb_bit(uchar code)
{
  t_addr a;
  t_mem m, d;
  class cl_address_space *mem;

  mem= bit2mem(fetch(), &a, &m);
  d= mem->read(a, HW_PORT);
  mem->write(a, d|m);
  return(resGO);
}


/*
 * 0xd3 1 12 SETB C
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_setb_c(uchar code)
{
  psw->write(psw->read() | bmCY);
  return(resGO);
}


/* End of s51.src/bit.cc */
