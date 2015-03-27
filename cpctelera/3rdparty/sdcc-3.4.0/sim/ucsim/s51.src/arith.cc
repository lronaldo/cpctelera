/*
 * Simulator of microcontrollers (arith.cc)
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

#include <stdio.h>

// local
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"


/*
 * 0x03 1 12 RR A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_rr(uchar code)
{
  uchar ac;

  ac= acc->read(); 
  if (ac & 0x01)
    acc->write((ac >> 1) | 0x80);
  else
    acc->write(ac >> 1);
  return(resGO);
}


/*
 * 0x13 1 12 RRC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_rrc(uchar code)
{
  bool cy;
  uchar ac;

  cy= SFR_GET_C;
  SFR_SET_C((ac= acc->read()) & 0x01);
  ac>>= 1;
  if (cy)
    ac|= 0x80;
  sfr->write(ACC, ac);
  return(resGO);
}


/*
 * 0x23 1 12 RL A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_rl(uchar code)
{
  uchar ac;

  ac= acc->read();
  if (ac & 0x80)
    acc->write((ac << 1 ) | 0x01);
  else
    acc->write(ac << 1);
  return(resGO);
}


/*
 * 0x24 2 12 ADD A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_add_a_Sdata(uchar code)
{
  uchar data, ac;
  bool newC, newA, c6;

  data= fetch();
  ac  = acc->read();
  newC= (((uint)ac+(uint)(data)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x25 2 12 ADD A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_add_a_addr(uchar code)
{
  uchar data, ac;
  bool newC, newA, c6;
  class cl_memory_cell *cell;
  t_addr a;

  cell= get_direct(a= fetch());
  data= cell->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)(data)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x26-0x27 1 12 ADD A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_add_a_Sri(uchar code)
{
  uchar data, ac;
  bool newC, newA, c6;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  ac  = acc->get();
  data= cell->read();
  newC= (((uint)ac+(uint)data) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x28-0x2f 1 12 ADD A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_add_a_rn(uchar code)
{
  uchar data, ac;
  bool newC, newA, c6;

  data= get_reg(code & 0x07)->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x33 1 12 RLC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_rlc(uchar code)
{
  bool cy;
  uchar ac;

  cy= SFR_GET_C;
  SFR_SET_C((ac= acc->get()) & 0x80);
  ac<<= 1;
  if (cy)
    ac|= 0x01;
  acc->write(ac);
  return(resGO);
}


/*
 * 0x34 2 12 ADDC A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_addc_a_Sdata(uchar code)
{
  uchar data, ac;
  bool orgC, newC, newA, c6;

  data= fetch();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= SFR_GET_C)?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x35 2 12 ADDC A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_addc_a_addr(uchar code)
{
  uchar data, ac;
  bool orgC, newC, newA, c6;
  class cl_memory_cell *cell;
  t_addr a;

  cell= get_direct(a= fetch());
  data= cell->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= SFR_GET_C)?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x36-0x37 1 12 ADDC A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_addc_a_Sri(uchar code)
{
  uchar data, ac;
  bool orgC, newC, newA, c6;
  class cl_memory_cell *cell;
  
  cell= iram->get_cell(get_reg(code & 0x01)->read());
  ac  = acc->get();
  data= cell->read();
  newC= (((uint)ac+(uint)data+((orgC= SFR_GET_C)?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x38-0x3f 1 12 ADDC A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_addc_a_rn(uchar code)
{
  uchar data, ac;
  bool orgC, newC, newA, c6;

  data= get_reg(code & 0x07)->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= SFR_GET_C)?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  SFR_SET_C(newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  return(resGO);
}


/*
 * 0x84 1 48 DIV AB
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_div_ab(uchar code)
{
  uchar temp, pw, b, ac;

  pw= psw->get();
  pw&= ~bmCY;
  if (!(b= sfr->get(B)))
    pw|= bmOV;
  else
    {
      pw&= ~bmOV;
      temp= (ac= acc->get()) / b;
      sfr->write(B, ac % b);
      acc->write(temp);
    }
  psw->write(pw);
  tick(3);
  return(resGO);
}


/*
 * 0x94 2 12 SUBB A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_subb_a_Sdata(uchar code)
{
  uchar data, ac, result, pw, c;

  data= fetch();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  acc->write(result);
  psw->write((pw & ~(bmCY|bmOV|bmAC)) |
	     (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	     (((ac<0x80 && data>0x7f && result>0x7f) ||
	       (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	     (((ac&0x0f) < ((data+c)&0x0f) ||
	       (c && ((data&0x0f)==0x0f)))?bmAC:0));
  return(resGO);
}


/*
 * 0x95 2 12 SUBB A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_subb_a_addr(uchar code)
{
  uchar data, ac, result, pw, c;
  class cl_memory_cell *cell;
  
  cell= get_direct(fetch());
  ac  = acc->get();
  data= cell->read();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  acc->write(result);
  psw->set((pw & ~(bmCY|bmOV|bmAC)) |
	   (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	   (((ac<0x80 && data>0x7f && result>0x7f) ||
	     (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	   (((ac&0x0f) < ((data+c)&0x0f) ||
	     (c && ((data&0x0f)==0x0f)))?bmAC:0));
  return(resGO);
}


/*
 * 0x96-0x97 1 12 SUBB A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_subb_a_Sri(uchar code)
{
  uchar data, ac, result, pw, c;
  class cl_memory_cell *cell;

  cell= iram->get_cell(get_reg(code & 0x01)->read());
  data= cell->read();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  acc->write(result);
  psw->write((pw & ~(bmCY|bmOV|bmAC)) |
	     (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	     (((ac<0x80 && data>0x7f && result>0x7f) ||
	       (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	     (((ac&0x0f) < ((data+c)&0x0f) ||
	       (c && ((data&0x0f)==0x0f)))?bmAC:0));
  return(resGO);
}


/*
 * 0x98-0x9f 1 12 SUBB A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_subb_a_rn(uchar code)
{
  uchar data, ac, result, pw, c;

  data= get_reg(code & 0x07)->read();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  acc->write(result);
  psw->write((pw & ~(bmCY|bmOV|bmAC)) |
	     (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	     (((ac<0x80 && data>0x7f && result>0x7f) ||
	       (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	     (((ac&0x0f) < ((data+c)&0x0f) ||
	       (c && ((data&0x0f)==0x0f)))?bmAC:0));
  return(resGO);
}


/*
 * 0xa4 1 48 MUL AB
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_mul_ab(uchar code)
{
  uint temp, pw, ac, b, x;

  pw= psw->get();
  pw&= ~bmCY;
  temp= (ac= acc->read()) * (b= sfr->get(B));
  acc->write(temp & 0xff);
  x= sfr->write(B, (temp >> 8) & 0xff);
  SFR_SET_BIT(x/*sfr->get(B)*/, PSW, bmOV);
  SFR_SET_BIT(0, PSW, bmCY);
  tick(3);
  return(resGO);
}


/*
 * 0xd4 1 12 DA A
 *____________________________________________________________________________
 *
 */

int
cl_51core::inst_da_a(uchar code)
{
  uchar ac, pw;

  ac= acc->get();
  pw= psw->get();
  if ((ac & 0x0f) > 9 ||
      (pw & bmAC))
    {
      if (((uint)ac+(uint)0x06) > 255)
	pw|= bmCY;
      ac+= 0x06;
    }
  if ((ac & 0xf0) > 0x90 ||
      (pw & bmCY))
    {
      if (((uint)ac+(uint)0x60) > 255)
	pw|= bmCY;
      ac+= 0x60;
    }
  acc->write(ac);
  psw->write(pw);
  return(resGO);
}


/* End of s51.src/arith.cc */
