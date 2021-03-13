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

//#include "ddconfig.h"

// local
#include "uc51cl.h"
//#include "regs51.h"
//#include "types51.h"


/*
 * 0x72 2 24 ORL C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_72/*inst_orl_c_bit*/(t_mem/*uchar*/ code)
{
  uchar bitaddr;

  //t_addr a;
  //t_mem m;
  //class cl_address_space *mem;
  bitaddr= fetch();
  //mem= bit2mem(bitaddr, &a, &m);
  /*SFR_SET_C(*/bits->set(0xd7,/*SFR_GET_C*/bits->get(0xd7) ||
	    /*(mem->read(a) & m)*/bits->read(bitaddr));
  tick(1);
  vc.rd++;
  return(resGO);
}


/*
 * 0x82 2 24 ANL C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_82/*inst_anl_c_bit*/(t_mem/*uchar*/ code)
{
  //t_mem m;
  //t_addr a;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch(), c= bits->get(0xd7);
  
  //mem= bit2mem(bitaddr, &a, &m);
  /*SFR_SET_C*/bits->set(0xd7,/*SFR_GET_C*/c &&
	    /*(mem->read(a) & m)*/bits->read(bitaddr));
  tick(1);
  vc.rd++;
  return(resGO);
}


/*
 * 0x92 2 24 MOV bit,C
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_92/*inst_mov_bit_c*/(t_mem/*uchar*/ code)
{
  //t_addr a;
  //t_mem m, d;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch();
  
  //mem= bit2mem(bitaddr, &a, &m);
  /*d= mem->read(a, HW_PORT);
  if (SFR_GET_C)
    mem->write(a, d|m);
  else
  mem->write(a, d&~m);*/
  bits->write(bitaddr, /*SFR_GET_C*/bits->get(0xd7));
  tick(1);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xa2 2 12 MOV C,bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_a2/*inst_mov_c_bit*/(t_mem/*uchar*/ code)
{
  //t_addr a;
  //t_mem m;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch(), x;
  
  //mem= bit2mem(bitaddr, &a, &m);
  //SFR_SET_C(/*mem->read(a) & m*/bits->read(bitaddr));
  x= bits->read(bitaddr);
  bits->set(0xd7, x);
  vc.rd++;
  return(resGO);
}


/*
 * 0xa0 2 24 ORL C,/bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_a0/*inst_orl_c_Sbit*/(t_mem/*uchar*/ code)
{
  //t_mem m;
  //t_addr a;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch();
  
  //mem= bit2mem(fetch(), &a, &m);
  /*SFR_SET_C(*/bits->set(0xd7, /*SFR_GET_C*/bits->get(0xd7) ||
			  !(/*mem->read(a) & m*/bits->read(bitaddr)));
  tick(1);
  vc.rd++;
  return(resGO);
}


/*
 * 0xb0 2 24 ANL C,/bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_b0/*inst_anl_c_Sbit*/(t_mem/*uchar*/ code)
{
  //t_mem m;
  //t_addr a;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch();
    
  //mem= bit2mem(fetch(), &a, &m);
  /*SFR_SET_C(*/bits->set(0xd7, /*SFR_GET_C*/bits->get(0xd7) &&
			  !(/*mem->read(a) & m*/bits->read(bitaddr)));
  tick(1);
  vc.rd++;
  return(resGO);
}


/*
 * 0xb2 2 12 CPL bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_b2/*inst_cpl_bit*/(t_mem/*uchar*/ code)
{
  //t_addr a;
  //t_mem m, d;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch(), b;
  
  //mem= bit2mem(fetch(), &a, &m);
  //d= mem->read(a, HW_PORT);
  //mem->write(a, d^m);
  b= bits->/*read*/get(bitaddr);
  bits->write(bitaddr, !b);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xb3 1 12 CPL C
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_b3/*inst_cpl_c*/(t_mem/*uchar*/ code)
{
  //psw->write(psw->read() ^ bmCY);
  bits->write(0xd7, !bits->read(0xd7));
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xc2 2 12 CLR bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c2/*inst_clr_bit*/(t_mem/*uchar*/ code)
{
  //t_addr a;
  //t_mem m;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch();//, b;
  
  //b= bits->get(bitaddr);
  //mem= bit2mem(bitaddr, &a, &m);
  //t_mem d= mem->read(a, HW_PORT);
  //mem->write(a, d&~m);
  bits->write(bitaddr, 0);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xc3 1 12 CLR C
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_c3/*inst_clr_c*/(t_mem/*uchar*/ code)
{
  //psw->write(psw->read() & ~bmCY);
  bits->write(0xd7, 0);
  return(resGO);
}


/*
 * 0xd2 2 12 SETB bit
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_d2/*inst_setb_bit*/(t_mem/*uchar*/ code)
{
  //t_addr a;
  //t_mem m, d;
  //class cl_address_space *mem;
  u8_t bitaddr= fetch();
  
  //mem= bit2mem(bitaddr, &a, &m);
  //d= mem->read(a, HW_PORT);
  //mem->write(a, d|m);
  bits->write(bitaddr, 1);
  vc.rd++;
  vc.wr++;
  return(resGO);
}


/*
 * 0xd3 1 12 SETB C
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_d3/*inst_setb_c*/(t_mem/*uchar*/ code)
{
  //psw->write(psw->read() | bmCY);
  bits->write(0xd7, 1);
  return(resGO);
}


/* End of s51.src/bit.cc */
